#pragma once
#include <memory>
#include <vector>
#include <array>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>
#include "Epoch/Assets/Asset.h"
#include "Epoch/Rendering/RenderConstants.h"
#include "Epoch/Math/AABB.h"
#include "Epoch/Math/Frustum.h"

namespace Epoch
{
	class Framebuffer;
	class RenderPipeline;
	class ConstantBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Mesh;
	class Texture2D;

	class DebugRenderer
	{
	public:
		DebugRenderer() = default;
		~DebugRenderer();

		void Init(std::shared_ptr<Framebuffer> aTargetBuffer);
		void SetViewportSize(unsigned aWidth, unsigned aHeight);

		void Render(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProjection, bool aOnTop = true);

		void DrawLine(const CU::Vector3f& aP0, const CU::Vector3f& aP1, CU::Color aColor = CU::Color::White);
		void DrawCircle(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor = CU::Color::White);
		void DrawWireBox(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector3f& aExtent, const CU::Color aColor = CU::Color::White);
		void DrawWireSphere(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor = CU::Color::White);
		void DrawWireCapsule(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, float aHeight, const CU::Color aColor = CU::Color::White);
		void DrawWireCone(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aAngle, float aRange, const CU::Color aColor = CU::Color::White);

		void DrawFrustum(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProj, const CU::Color aColor = CU::Color::White);

		void DrawWireAABB(const AABB& aAABB, const CU::Matrix4x4f& aTransform, const CU::Color aColor = CU::Color::White);

		void DrawGrid(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector2i& aSize, float aAlpha = 0.5f);

		void DrawQuad(std::shared_ptr<Texture2D> aTexture, const CU::Matrix4x4f& aTransform, const CU::Color& aTint = CU::Color::White, uint32_t aEntityID = 0);

		struct Stats
		{
			uint32_t vertices = 0;
			uint32_t indices = 0;
			uint32_t drawCalls = 0;
		};
		const Stats& GetStats() const { return myStats; }

	private:
		void Shutdown();

		void Flush();

	private:
		Stats myStats;
		bool myNeedsResize = false;
		uint32_t myViewportWidth = 0;
		uint32_t myViewportHeight = 0;
		
		std::shared_ptr<RenderPipeline> myLinePipelineState;
		std::shared_ptr<RenderPipeline> myOccludedLinePipelineState;
		std::shared_ptr<RenderPipeline> myGridPipelineState;
		std::shared_ptr<RenderPipeline> myQuadPipelineState;

		struct CameraBuffer
		{
			CU::Matrix4x4f invViewProjection;
		};
		std::shared_ptr<ConstantBuffer> myCameraBuffer;

		// Lines
		struct LineVertex
		{
			CU::Vector3f position;
			CU::Color color;
		};

		struct Frame
		{
			std::vector<LineVertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;

			//uint32_t baseVertex = 0;
			//uint32_t baseIndex = 0;
			//uint32_t vertexCount = 0;
			//uint32_t indexCount = 0;
		};

		std::vector<Frame> myFrames;

		std::vector<LineVertex> myVertices;
		std::vector<uint32_t> myIndices;

		std::shared_ptr<VertexBuffer> myVertexBuffer;
		std::shared_ptr<IndexBuffer> myIndexBuffer;

		std::shared_ptr<ConstantBuffer> myDebugLineBuffer;

		// Grid
		struct GridBuffer
		{
			CU::Matrix4x4f transform;
			CU::Vector2f size;
			float alpha;
			float padding;
		};
		std::shared_ptr<ConstantBuffer> myGridBuffer;

		// Quads
		struct QuadVertex
		{
			CU::Vector3f position;
			uint32_t entityID = 0;
			CU::Vector4f tint;
			CU::Vector2f uv;
		};

		CU::Vector4f myQuadVertexPositions[4];
		CU::Vector2f myQuadUVCoords[4];
		std::unordered_map<AssetHandle, std::vector<QuadVertex>> myQuadVertices;
		std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> myTextures;
		std::shared_ptr<VertexBuffer> myQuadVertexBuffer;
		std::shared_ptr<IndexBuffer> myQuadIndexBuffer;
		uint32_t myQuadCount = 0;
	};
}
