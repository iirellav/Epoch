#pragma once
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <CommonUtilities/Math/Transform.h>
#include "Epoch/Rendering/RenderConstants.h"
#include "Epoch/Scene/Scene.h"
#include "Epoch/Scene/SceneInfo.h"

namespace Epoch
{
	class Font;
	class Mesh;
	class Material;
	class MaterialTable;
	class Texture2D;
	class Environment;
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class Framebuffer;
	class RenderPipeline;
	class ComputePipeline;

	class DebugRenderer;

	struct SceneRendererCamera
	{
		Camera camera;
		CU::Vector3f position;
		CU::Matrix4x4f transform;
		CU::Matrix4x4f viewMatrix;
		float nearPlane = 0.0f;
		float farPlane = 0.0f;
		float fov = 0.0f;
		float aspect = 0.0f;
	};

	struct CameraBuffer
	{
		CU::Matrix4x4f viewProjection;
		CU::Vector3f pos;
		float nearPlane = 0.0f;
		float farPlane = 0.0f;
		float fov = 0.0f;
		CU::Vector2f viewportSize = 0.0f;
	};

	struct ObjectBuffer
	{
		CU::Matrix4x4f transform;
	};

	struct BoneBuffer
	{
		std::array<CU::Matrix4x4f, 128> boneTransforms;
	};

	struct LightBuffer
	{
		CU::Vector3f direction;
		float intensity = 0;
		CU::Vector3f color;
		float environmentIntensity = 0;
	};

	enum class DrawMode : uint32_t
	{
		Shaded,
		Albedo,
		Normals,
		AmbientOcclusion,
		Roughness,
		Metalness,
		Emission
	};

	class SceneRenderer
	{
	public:
		SceneRenderer();
		~SceneRenderer();

		void SetDebugRenderer(std::shared_ptr<DebugRenderer> aDebugRenderer) { myDebugRenderer = aDebugRenderer; }
		std::shared_ptr<DebugRenderer> GetDebugRenderer() { return myDebugRenderer; }

		std::pair<uint32_t, uint32_t> GetViewportSize() { return { myViewportWidth, myViewportHeight }; }
		void SetViewportSize(uint32_t aWidth, uint32_t aHeight);

		void SetScene(std::shared_ptr<Scene> aScene);
		void BeginScene(const SceneRendererCamera& aCamera);
		void EndScene();

		void SubmitMesh(std::shared_ptr<Mesh> aMesh, std::shared_ptr<MaterialTable> aMaterialTable, const CU::Matrix4x4f& aTransform, uint32_t aEntityID = 0);
		void SubmitAnimatedMesh(std::shared_ptr<Mesh> aMesh, const CU::Matrix4x4f& aTransform, const std::vector<CU::Matrix4x4f>& aBoneTransforms);

		void SubmitQuad(const CU::Matrix4x4f aTransform, const CU::Color& aColor, uint32_t aEntityID = 0);
		void SubmitQuad(const CU::Matrix4x4f aTransform, std::shared_ptr<Texture2D> aTexture, const CU::Color& aTint = CU::Color::White, bool aFlipX = false, bool aFlipY = false, uint32_t aEntityID = 0);

		struct TextSettings
		{
			bool centered = false;
			float maxWidth = 10.0f;
			CU::Color color = CU::Color::White;
			float lineHeightOffset = 0.0f;
			float letterSpacing = 0.0f;
			bool billboard = false;
		};
		void SubmitText(const std::string& aString, const std::shared_ptr<Font>& aFont, const CU::Matrix4x4f& aTransform, const TextSettings& aSettings, uint32_t aEntityID = 0);

		std::shared_ptr<Texture2D> GetFinalPassTexture();
		std::shared_ptr<Texture2D> GetEntityIDTexture();
		std::shared_ptr<Framebuffer> GetExternalCompositingFramebuffer();

		DrawMode GetDrawMode() { return myDrawMode; }
		void SetDrawMode(DrawMode aDrawMode) { myDrawMode = aDrawMode; }

		bool& ColorGrading() { return myColorGradingEnabled; } //TEMP

		struct Stats
		{
			uint32_t drawCalls = 0;
			uint32_t savedDraws = 0;
			uint32_t instances = 0;
			uint32_t batched = 0;
			uint32_t vertices = 0;
			uint32_t indices = 0;
			uint32_t triangles = 0;
			uint32_t meshes = 0;
			uint32_t submeshes = 0;
		};
		const Stats GetStats() const { return myStats; }

	private:
		void Init();
		void Shutdown();

		void PreRender();

		void UpdateStatistics();

		//TEMP
		void SetMaterial(std::shared_ptr<Material> aMaterial);

	private:
		std::shared_ptr<Scene> myScene;
		std::shared_ptr<DebugRenderer> myDebugRenderer;

		bool myActive = false;

		//TODO: Move to setting or something
		DrawMode myDrawMode = DrawMode::Shaded;
		bool myColorGradingEnabled = true;

		Stats myStats;

		uint32_t myViewportWidth = 0;
		uint32_t myViewportHeight = 0;
		float myInvViewportWidth = 0.0f;
		float myInvViewportHeight = 0.0f;
		bool myNeedsResize = false;

		struct SceneInfo
		{
			SceneRendererCamera sceneCamera;
			LightEnvironment lightEnvironment;
			PostProcessingData postProcessingData;
		} mySceneData;

		std::shared_ptr<Framebuffer> myExternalCompositingFramebuffer;

		std::shared_ptr<ConstantBuffer> myCameraBuffer;
		std::shared_ptr<ConstantBuffer> myObjectBuffer;
		std::shared_ptr<ConstantBuffer> myMaterialBuffer;
		std::shared_ptr<ConstantBuffer> myBoneBuffer;
		std::shared_ptr<ConstantBuffer> myLightBuffer;
		std::shared_ptr<ConstantBuffer> myPointLightBuffer;
		std::shared_ptr<ConstantBuffer> mySpotlightBuffer;
		std::shared_ptr<ConstantBuffer> myDebugDrawModeBuffer;
		std::shared_ptr<ConstantBuffer> myPostProcessingBuffer;

		// Meshes
		struct MeshKey
		{
			AssetHandle meshHandle = 0;
			AssetHandle materialHandle = 0;
			unsigned submeshIndex = 0;
			bool castsShadows = false;
			//bool isSelected = false;

			MeshKey(AssetHandle aMeshHandle, AssetHandle aMaterialHandle, unsigned aSubmeshIndex, bool aCastsShadows/*, bool aIsSelected*/)
				: meshHandle(aMeshHandle), materialHandle(aMaterialHandle), submeshIndex(aSubmeshIndex), castsShadows(aCastsShadows)/*, isSelected(aIsSelected)*/
			{
			}

			bool operator<(const MeshKey& other) const
			{
				if (meshHandle < other.meshHandle)
					return true;

				if (meshHandle > other.meshHandle)
					return false;

				if (submeshIndex < other.submeshIndex)
					return true;

				if (submeshIndex > other.submeshIndex)
					return false;

				if (materialHandle < other.materialHandle)
					return true;

				if (materialHandle > other.materialHandle)
					return false;

				//if (castsShadows < other.castsShadows)
				//	return true;

				//if (castsShadows > other.castsShadows)
				//	return false;

				return castsShadows > other.castsShadows;

				//return isSelected < other.isSelected;

			}
		};

		struct DrawCommand
		{
			std::shared_ptr<Mesh> mesh;
			uint32_t submeshIndex = 0;
			std::shared_ptr<Material> material;

			uint32_t instanceCount = 0;
		};

		struct AnimatedDrawCommand
		{
			std::shared_ptr<Mesh> mesh;
			CU::Matrix4x4f transform;
			std::vector<CU::Matrix4x4f> boneTransforms;
		};

		struct MeshInstanceData
		{
			CU::Vector4f row[3];
			uint32_t id;
		};
		
		std::map<MeshKey, std::vector<MeshInstanceData>> myMeshTransformMap;
		std::map<MeshKey, DrawCommand> myDrawList;
		std::vector<AnimatedDrawCommand> myAnimatedDrawList;

		std::shared_ptr<VertexBuffer> myInstanceTransformBuffer;

		std::shared_ptr<RenderPipeline> myGBufferPipeline;

		std::shared_ptr<RenderPipeline> myEnvironmentalLightPipeline;
		std::shared_ptr<RenderPipeline> myPointLightPipeline;
		std::shared_ptr<RenderPipeline> mySpotlightPipeline;

		std::shared_ptr<RenderPipeline> myUberPipeline;

		std::shared_ptr<RenderPipeline> mySpritePipeline;
		std::shared_ptr<RenderPipeline> myTextPipeline;

		std::shared_ptr<RenderPipeline> myDebugRenderPipeline;

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
		
		// Text
		struct TextVertex
		{
			CU::Vector3f position;
			uint32_t entityID = 0;
			CU::Vector4f tint;
			CU::Vector2f uv;
		};
		std::unordered_map<AssetHandle, std::vector<TextVertex>> myTextVertices;
		std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> myFontAtlases;
		std::shared_ptr<VertexBuffer> myTextVertexBuffer;
		std::shared_ptr<IndexBuffer> myTextIndexBuffer;
		uint32_t myTextQuadCount = 0;
	};
}
