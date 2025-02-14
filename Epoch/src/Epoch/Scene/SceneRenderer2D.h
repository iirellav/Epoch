#pragma once
#include <memory>
#include <unordered_map>
#include <CommonUtilities/Math/Transform.h>
#include "Epoch/Rendering/RenderConstants.h"
#include "Epoch/Core/UUID.h"

namespace Epoch
{
	class Framebuffer;
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class RenderPipeline;
	class Texture2D;

	class SceneRenderer2D
	{
	public:
		struct QuadSetting
		{
			CU::Color tint = CU::Color::White;
			bool flipX = false;
			bool flipY = false;
			bool billboard = false;
		};
		
		struct ScreenSpaceQuadSetting
		{
			CU::Color tint = CU::Color::White;
			bool flipX = false;
			bool flipY = false;
			CU::Vector2f pivot = CU::Vector2f(0.5f, 0.5f);
			CU::Vector2f anchor = CU::Vector2f(0.5f, 0.5f);
		};

		static inline const QuadSetting DefaultQuadSetting;
		static inline const ScreenSpaceQuadSetting DefaultScreenSpaceQuadSetting;

	public:
		SceneRenderer2D();
		~SceneRenderer2D();

		void Init(std::shared_ptr<Framebuffer> aTargetBuffer);
		
		std::pair<uint32_t, uint32_t> GetViewportSize() { return { myViewportWidth, myViewportHeight }; }
		void SetViewportSize(uint32_t aWidth, uint32_t aHeight);

		void BeginScene(const CU::Matrix4x4f& aProj, const CU::Matrix4x4f& aView);
		void EndScene();

		void SubmitQuad(const CU::Matrix4x4f aTransform, std::shared_ptr<Texture2D> aTexture = nullptr, const QuadSetting& aSettings = DefaultQuadSetting, uint32_t aEntityID = 0);

		void SubmitScreenSpaceQuad(const CU::Vector3f aPosition, const CU::Vector3f aRotation, const CU::Vector2ui aSize, std::shared_ptr<Texture2D> aTexture = nullptr, const ScreenSpaceQuadSetting& aSettings = DefaultScreenSpaceQuadSetting, uint32_t aEntityID = 0);

	private:
		void Shutdown();

		void QuadPass();

	private:
		bool myActive = false;

		uint32_t myViewportWidth = 0;
		uint32_t myViewportHeight = 0;
		bool myNeedsResize = false;

		CU::Matrix4x4f myViewProj;
		CU::Matrix4x4f myView;
		CU::Matrix4x4f myProj;
		
		std::shared_ptr<ConstantBuffer> myCameraBuffer;
		
		std::shared_ptr<RenderPipeline> myQuadPipeline;

		// Quads
		struct QuadVertex
		{
			CU::Vector3f position;
			uint32_t entityID = 0;
			CU::Vector4f tint;
			CU::Vector2f uv;
		};

		CU::Vector2f myScreenSpaceQuadVertexPositions[4];
		CU::Vector4f myQuadVertexPositions[4];
		CU::Vector2f myQuadUVCoords[4];
		std::unordered_map<UUID, std::vector<QuadVertex>> myQuadVertices;
		std::unordered_map<UUID, std::shared_ptr<Texture2D>> myTextures;
		std::shared_ptr<VertexBuffer> myQuadVertexBuffer;
		std::shared_ptr<IndexBuffer> myQuadIndexBuffer;
	};
}
