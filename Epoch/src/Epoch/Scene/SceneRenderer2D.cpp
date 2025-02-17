#include "epch.h"
#include "SceneRenderer2D.h"
#include "Epoch/Rendering/Renderer.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "Epoch/Rendering/ConstantBuffer.h"
#include "Epoch/Rendering/RenderPipeline.h"
#include "Epoch/Rendering/Texture.h"
#include "Epoch/Rendering/Font.h"
#include "Epoch/Rendering/MSDFData.h"

#include "Epoch/Rendering/RHI.h"
#include <codecvt>

namespace Epoch
{
	SceneRenderer2D::SceneRenderer2D()
	{
	}

	SceneRenderer2D::~SceneRenderer2D()
	{
		Shutdown();
	}

	void SceneRenderer2D::Init(std::shared_ptr<Framebuffer> aTargetBuffer)
	{
		EPOCH_PROFILE_FUNC();

		//Quad Data/Vertex Buffer
		{
			myQuadUVCoords[0] = { 0.0f, 1.0f };
			myQuadUVCoords[1] = { 0.0f, 0.0f };
			myQuadUVCoords[2] = { 1.0f, 0.0f };
			myQuadUVCoords[3] = { 1.0f, 1.0f };

			myQuadVertexPositions[0] = { 0.0f, 0.0f };
			myQuadVertexPositions[1] = { 0.0f, 1.0f };
			myQuadVertexPositions[2] = { 1.0f, 1.0f };
			myQuadVertexPositions[3] = { 1.0f, 0.0f };

			myQuadVertexBuffer = VertexBuffer::Create(MaxQuadVertices, sizeof(Vertex));
		}
		
		//Quad Vertex Buffer
		{
			myTextVertexBuffer = VertexBuffer::Create(MaxQuadVertices, sizeof(Vertex));
		}

		//Index Buffer
		{
			uint32_t* quadIndices = new uint32_t[MaxQuadIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxQuadIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}

			myIndexBuffer = IndexBuffer::Create(quadIndices, MaxQuadIndices);
			delete[] quadIndices;
		}

		myCameraBuffer = ConstantBuffer::Create(sizeof(CU::Matrix4x4f));

		VertexBufferLayout layout =
		{
			{ ShaderDataType::Float3,	"POSITION" },
			{ ShaderDataType::UInt,		"ID" },
			{ ShaderDataType::Float4,	"TINT" },
			{ ShaderDataType::Float2,	"UV" }

		};

		FramebufferSpecification specs;
		specs.existingFramebuffer = aTargetBuffer;
		specs.clearColorOnLoad = false;
		specs.clearDepthOnLoad = false;
		std::shared_ptr<Framebuffer> fb = Framebuffer::Create(specs);

		//Quads
		{
			PipelineSpecification pipelineSpecs("Quads");
			pipelineSpecs.targetFramebuffer = fb;
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Sprite");
			pipelineSpecs.vertexLayouts.push_back(layout);
			myQuadPipeline = RenderPipeline::Create(pipelineSpecs);
		}

		//Text
		{
			PipelineSpecification pipelineSpecs("Text");
			pipelineSpecs.targetFramebuffer = fb;
			pipelineSpecs.shader = Renderer::GetShaderLibrary()->Get("Text");
			pipelineSpecs.vertexLayouts.push_back(layout);
			myTextPipeline = RenderPipeline::Create(pipelineSpecs);
		}
	}

	void SceneRenderer2D::Shutdown()
	{
	}

	void SceneRenderer2D::SetViewportSize(uint32_t aWidth, uint32_t aHeight)
	{
		if ((myViewportWidth != aWidth || myViewportHeight != aHeight) && aWidth > 0 && aHeight > 0)
		{
			myViewportWidth = aWidth;
			myViewportHeight = aHeight;
			myNeedsResize = true;
		}
	}

	void SceneRenderer2D::BeginScene(const CU::Matrix4x4f& aProj, const CU::Matrix4x4f& aView)
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(!myActive, "Can't call begin scene while rendering!");
		myActive = true;

		myProj = aProj;
		myView = aView;
		myViewProj = aView * aProj;

		if (myNeedsResize)
		{
			myNeedsResize = false;

			myQuadPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
			myTextPipeline->GetSpecification().targetFramebuffer->Resize(myViewportWidth, myViewportHeight);
		}

		myCameraBuffer->SetData(&myViewProj);
		myCameraBuffer->Bind(PIPELINE_STAGE_VERTEX_SHADER, 0);
	}

	void SceneRenderer2D::EndScene()
	{
		EPOCH_PROFILE_FUNC();

		EPOCH_ASSERT(myActive, "Can't call end scene if not rendering!");
		myActive = false;

		QuadPass();
		TextPass();

		myQuadVertices.clear();
		myTextVertices.clear();
		myTextures.clear();
		myFontAtlases.clear();
	}

	static CU::Vector2f FlipUVCoord(const CU::Vector2f& aCoord, bool aFlipX, bool aFlipY)
	{
		return { CU::Math::Abs(aCoord.x - (float)aFlipX), CU::Math::Abs(aCoord.y - (float)aFlipY) };
	}

	void SceneRenderer2D::SubmitQuad(const CU::Matrix4x4f aTransform, const CU::Vector2ui aSize, std::shared_ptr<Texture2D> aTexture, const QuadSetting& aSettings, uint32_t aEntityID)
	{
		if (!aTexture) aTexture = Renderer::GetWhiteTexture();
		auto& vertexList = myQuadVertices[aTexture->GetHandle()];
		myTextures[aTexture->GetHandle()] = aTexture;

		CU::Vector2f scaleMultiplier = CU::Vector2f::One;
		if (aTexture && aSettings.preserveAspectRatio)
		{
			if ((float)aTexture->GetWidth() < (float)aTexture->GetHeight())
			{
				scaleMultiplier.x = (float)aTexture->GetWidth() / (float)aTexture->GetHeight();
			}
			else
			{
				scaleMultiplier.y = (float)aTexture->GetHeight() / (float)aTexture->GetWidth();
			}
		}

		const CU::Vector2f size = CU::Vector2f((float)aSize.x, (float)aSize.y) * scaleMultiplier;

		for (size_t i = 0; i < 4; i++)
		{
			const CU::Vector2f vertPos = (myQuadVertexPositions[i] - aSettings.pivot) * size;

			Vertex& vertex = vertexList.emplace_back();
			vertex.position = aTransform * CU::Vector4f(vertPos.x, vertPos.y, 0.0f, 1.0f);
			vertex.uv = FlipUVCoord(myQuadUVCoords[i], aSettings.flipX, aSettings.flipY);
			vertex.tint = aSettings.tint.GetVector4();
			vertex.entityID = aEntityID;
		}
	}

	static bool NextLine(int aIndex, const std::set<int>& aLines)
	{
		if (aLines.contains(aIndex))
		{
			return true;
		}
		return false;
	}

	// warning C4996: 'std::codecvt_utf8<char32_t,1114111,(std::codecvt_mode)0>': warning STL4017: std::wbuffer_convert, std::wstring_convert, and the <codecvt> header
	// (containing std::codecvt_mode, std::codecvt_utf8, std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17. (The std::codecvt class template is NOT deprecated.)
	// The C++ Standard doesn't provide equivalent non-deprecated functionality; consider using MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead.
	// You can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#pragma warning(disable : 4996)
	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	static std::u32string To_UTF32(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(s);
	}
#pragma warning(default : 4996)

	void SceneRenderer2D::SubmitText(const std::string& aString, const std::shared_ptr<Font>& aFont, const CU::Matrix4x4f& aTransform, const TextSettings& aSettings, uint32_t aEntityID)
	{
		if (aString.empty()) return;

		EPOCH_PROFILE_FUNC();

		std::shared_ptr<Texture2D> fontAtlas = aFont->GetFontAtlas();
		EPOCH_ASSERT(fontAtlas, "Font didn't have a valid font atlas!");

		auto& vertexList = myTextVertices[aFont->GetHandle()];
		myFontAtlases[aFont->GetHandle()] = fontAtlas;

		auto& fontGeometry = aFont->GetMSDFData()->fontGeometry;
		const auto& metrics = fontGeometry.getMetrics();

		std::u32string utf32string = To_UTF32(aString);

		std::set<int> nextLines;
		{
			double x = 0.0;
			double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;
			const double startHeight = y;
			int lastSpace = -1;
			for (int i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					x = 0;
					y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					continue;
				}
				if (character == '\r')
				{
					continue;
				}

				const bool isTab = character == '\t';
				if (isTab) character = ' ';

				const auto glyph = fontGeometry.getGlyph(character);
				//if (!glyph)
				//{
				//	glyph = fontGeometry.getGlyph('?');}
				//}
				if (!glyph)
				{
					continue;
				}

				if (character != ' ')
				{
					// Calculate geo
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					CU::Vector2f quadMin((float)pl, (float)pb);
					CU::Vector2f quadMax((float)pr, (float)pt);

					quadMin *= (float)fsScale;
					quadMax *= (float)fsScale;
					quadMin += CU::Vector2f((float)x, (float)y);
					quadMax += CU::Vector2f((float)x, (float)y);

					if (quadMax.x > aSettings.maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						nextLines.insert(lastSpace);
						lastSpace = -1;
						x = 0;
						y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32string[i + 1]);
				x += (fsScale * advance + aSettings.letterSpacing) * (isTab ? 4 : 1);
			}
		}

		{
			double x = 0.0f;
			const double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0;
			for (int i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n' || NextLine(i, nextLines))
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight + aSettings.lineHeightOffset;
					continue;
				}
				if (character == '\r')
				{
					continue;
				}

				const bool isTab = character == '\t';
				if (isTab) character = ' ';

				auto glyph = fontGeometry.getGlyph(character);
				//if (!glyph)
				//{
				//	glyph = fontGeometry.getGlyph('?');
				//}
				if (!glyph)
				{
					continue;
				}

				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				pl += x, pb += y, pr += x, pt += y;

				double texelWidth = 1. / fontAtlas->GetWidth();
				double texelHeight = 1. / fontAtlas->GetHeight();
				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				{
					Vertex& vertex = vertexList.emplace_back();
					vertex.position = aTransform * CU::Vector4f((float)pl * 100.0f, (float)pb * 100.0f, 0.0f, 1.0f);
					vertex.tint = aSettings.color.GetVector4();
					vertex.uv = { (float)l, (float)b };
					vertex.entityID = aEntityID;
				}

				{
					Vertex& vertex = vertexList.emplace_back();
					vertex.position = aTransform * CU::Vector4f((float)pl * 100.0f, (float)pt * 100.0f, 0.0f, 1.0f);
					vertex.tint = aSettings.color.GetVector4();
					vertex.uv = { (float)l, (float)t };
					vertex.entityID = aEntityID;
				}

				{
					Vertex& vertex = vertexList.emplace_back();
					vertex.position = aTransform * CU::Vector4f((float)pr * 100.0f, (float)pt * 100.0f, 0.0f, 1.0f);
					vertex.tint = aSettings.color.GetVector4();
					vertex.uv = { (float)r, (float)t };
					vertex.entityID = aEntityID;
				}

				{
					Vertex& vertex = vertexList.emplace_back();
					vertex.position = aTransform * CU::Vector4f((float)pr * 100.0f, (float)pb * 100.0f, 0.0f, 1.0f);
					vertex.tint = aSettings.color.GetVector4();
					vertex.uv = { (float)r, (float)b };
					vertex.entityID = aEntityID;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32string[i + 1]);
				x += (fsScale * advance + aSettings.letterSpacing) * (isTab ? 4 : 1);
			}
		}
	}

	void SceneRenderer2D::QuadPass()
	{
		EPOCH_PROFILE_FUNC();

		Renderer::SetRenderPipeline(myQuadPipeline);

		for (const auto& [texture, vertexList] : myQuadVertices)
		{
			if (!myTextures[texture])
			{
				continue;
			}

			std::vector<ID3D11ShaderResourceView*> SRVs(1);
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(myTextures[texture]);
			SRVs[0] = dxTexture->GetSRV().Get();
			RHI::GetContext()->PSSetShaderResources(0, 1, SRVs.data());

			const uint32_t quadCount = (uint32_t)vertexList.size() / 4;
			for (uint32_t i = 0; i < quadCount; i += MaxQuads)
			{
				uint32_t count = CU::Math::Min(quadCount - i, MaxQuads);

				myQuadVertexBuffer->SetData((void*)vertexList.data(), count * 4, i * 4 * sizeof(Vertex));
				Renderer::RenderGeometry(myQuadVertexBuffer, myIndexBuffer, count * 6);
			}

			std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
			RHI::GetContext()->PSSetShaderResources(0, 1, emptySRVs.data());
		}

		Renderer::RemoveRenderPipeline(myQuadPipeline);
	}
	
	void SceneRenderer2D::TextPass()
	{
		EPOCH_PROFILE_FUNC();

		Renderer::SetRenderPipeline(myTextPipeline);

		for (const auto& [font, vertexList] : myTextVertices)
		{
			if (!myFontAtlases[font])
			{
				continue;
			}

			std::vector<ID3D11ShaderResourceView*> SRVs(1);
			auto dxTexture = std::dynamic_pointer_cast<DX11Texture2D>(myFontAtlases[font]);
			SRVs[0] = dxTexture->GetSRV().Get();
			RHI::GetContext()->PSSetShaderResources(0, 1, SRVs.data());

			const uint32_t quadCount = (uint32_t)vertexList.size() / 4;
			for (uint32_t i = 0; i < quadCount; i += MaxQuads)
			{
				uint32_t count = CU::Math::Min(quadCount - i, MaxQuads);

				myTextVertexBuffer->SetData((void*)vertexList.data(), count * 4, i * 4 * sizeof(Vertex));
				Renderer::RenderGeometry(myTextVertexBuffer, myIndexBuffer, count * 6);
			}

			std::vector<ID3D11ShaderResourceView*> emptySRVs(1);
			RHI::GetContext()->PSSetShaderResources(0, 1, emptySRVs.data());
		}

		Renderer::RemoveRenderPipeline(myTextPipeline);
	}
}