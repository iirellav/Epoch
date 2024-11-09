#pragma once
#include <CommonUtilities/Color.h>
#include "Texture.h"

namespace Epoch
{
	enum class AttachmentLoadOp { Inherit, Clear };

	struct FramebufferTextureSpecification
	{
		TextureFormat format = TextureFormat::None;
		std::string debugName = "";

		FramebufferTextureSpecification(TextureFormat aFormat, const std::string& aName = "") : format(aFormat), debugName(aName) {}
	};

	struct FramebufferAttachmentSpecification
	{
		std::vector<FramebufferTextureSpecification> attachments;
		
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& aAttachments) : attachments(aAttachments) {}
	};

	class Framebuffer;

	struct FramebufferSpecification
	{
		float scale = 1.0f;
		uint32_t width = 0;
		uint32_t height = 0;

		bool swapChainTarget = false;

		FramebufferAttachmentSpecification attachments;
		std::shared_ptr<Framebuffer> existingFramebuffer;
		std::vector<std::shared_ptr<Texture2D>> existingAttachments;
		std::shared_ptr<Texture2D> existingDepthAttachment;

		CU::Color clearColor = CU::Color::Black;
		float depthClearValue = 1.0f;
		
		bool clearColorOnLoad = true;
		bool clearDepthOnLoad = true;

		std::string debugName = "Unnamed";

		FramebufferSpecification() = default;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& aSpec);
		
		std::shared_ptr<Texture2D> GetTarget(uint32_t aAttachmentIndex = 0) { return myAttachmentTextures[aAttachmentIndex]; }
		size_t GetColorAttachmentCount() const { return myAttachmentTextures.size(); }
		bool HasDepthAttachment() const { return (bool)myDepthAttachmentTexture; }
		std::shared_ptr<Texture2D> GetDepthAttachment() const { return myDepthAttachmentTexture; }

		FramebufferSpecification& GetSpecification() { return mySpecification; }
		const FramebufferSpecification& GetSpecification() const { return mySpecification; }
		uint32_t GetWidth() const { return myWidth; }
		uint32_t GetHeight() const { return myHeight; }

		virtual void Resize(uint32_t aWidth, uint32_t aHeight) = 0;

	protected:
		FramebufferSpecification mySpecification;

		uint32_t myWidth = 0;
		uint32_t myHeight = 0;

		std::vector<std::shared_ptr<Texture2D>> myAttachmentTextures;
		std::shared_ptr<Texture2D> myDepthAttachmentTexture;
	};
}
