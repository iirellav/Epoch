#pragma once
#include <memory>
#include <vector>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>

namespace Epoch
{
	class Framebuffer;
	class RenderPipeline;
	class ConstantBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Mesh;

	constexpr uint32_t MaxIndices = 1024 * 3;
	constexpr uint32_t MaxVertices = 1024 * 2;

	class DebugRenderer
	{
	public:
		DebugRenderer() = default;
		~DebugRenderer();

		void Init(std::shared_ptr<Framebuffer> aTargetBuffer);
		void SetViewportSize(unsigned aWidth, unsigned aHeight);

		void Render(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProjection);

		void DrawLine(const CU::Vector3f& aP0, const CU::Vector3f& aP1, CU::Color aColor = CU::Color::White);
		void DrawCircle(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor = CU::Color::White);
		void DrawWireBox(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector3f& aExtent, const CU::Color aColor = CU::Color::White);
		void DrawWireSphere(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, const CU::Color aColor = CU::Color::White);
		void DrawWireCapsule(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aRadius, float aHeight, const CU::Color aColor = CU::Color::White);
		void DrawWireCone(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, float aAngle, float aRange, const CU::Color aColor = CU::Color::White);
		
		void DrawGrid(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation, const CU::Vector2i& aSize, float aAlpha = 0.5f);

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
		struct DebugVertex
		{
			CU::Vector3f position;
			CU::Color color;
		};

		struct Frame
		{
			std::vector<DebugVertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;

			//uint32_t baseVertex = 0;
			//uint32_t baseIndex = 0;
			//uint32_t vertexCount = 0;
			//uint32_t indexCount = 0;
		};

		std::vector<Frame> myFrames;

		std::vector<DebugVertex> myVertices;
		std::vector<uint32_t> myIndices;

		std::shared_ptr<VertexBuffer> myVertexBuffer;
		std::shared_ptr<IndexBuffer> myIndexBuffer;

		std::shared_ptr<RenderPipeline> myLinePipelineState;
		std::shared_ptr<RenderPipeline> myOccludedLinePipelineState;
		std::shared_ptr<RenderPipeline> myGridPipelineState;

		struct CameraBuffer
		{
			CU::Matrix4x4f invViewProjection;
		};
		std::shared_ptr<ConstantBuffer> myCameraBuffer;

		std::shared_ptr<ConstantBuffer> myDebugLineBuffer;

		struct GridBuffer
		{
			CU::Matrix4x4f transform;
			CU::Vector2f size;
			float alpha;
			float padding;
		};
		std::shared_ptr<ConstantBuffer> myGridBuffer;

		Stats myStats;
	};
}
