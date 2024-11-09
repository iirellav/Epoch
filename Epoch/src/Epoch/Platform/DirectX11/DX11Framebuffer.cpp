#include "epch.h"
#include "DX11Framebuffer.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Core/Window.h"
#include "Epoch/Rendering/RHI.h"

namespace Epoch
{
	DX11Framebuffer::DX11Framebuffer(const FramebufferSpecification& aSpec)
	{
		EPOCH_PROFILE_FUNC();

		mySpecification = aSpec;

		if (aSpec.width == 0)
		{
			myWidth = Application::Get().GetWindowWidth();
			myHeight = Application::Get().GetWindowHeight();
		}
		else
		{
			myWidth = (uint32_t)(aSpec.width * aSpec.scale);
			myHeight = (uint32_t)(aSpec.height * aSpec.scale);
		}

		if (aSpec.existingFramebuffer)
		{
			for (size_t i = 0; i < mySpecification.existingFramebuffer->GetColorAttachmentCount(); i++)
			{
				myAttachmentTextures.emplace_back(mySpecification.existingFramebuffer->GetTarget((uint32_t)i));
			}

			if (mySpecification.existingFramebuffer->HasDepthAttachment())
			{
				myDepthAttachmentTexture = mySpecification.existingFramebuffer->GetDepthAttachment();
			}
		}
		else if (aSpec.existingAttachments.size() > 0)
		{
			for (size_t i = 0; i < aSpec.existingAttachments.size(); i++)
			{
				myAttachmentTextures.emplace_back(aSpec.existingAttachments[i]);
			}

			//if (aSpec.existingDepthAttachment)
			//{
			//	myDepthAttachmentTexture = aSpec.existingDepthAttachment;
			//}
		}
		else
		{
			int colorAttachmentIndex = 0;
			bool depthAttachmentCreated = false;
			for (auto& attachmentSpec : mySpecification.attachments.attachments)
			{
				if (IsDepthFormat(attachmentSpec.format))
				{
					if (depthAttachmentCreated)
					{
						LOG_ERROR("A framebuffer can't have more than one depth attachment! '{}'", mySpecification.debugName);
						continue;
					}

					TextureSpecification spec;
					spec.format = attachmentSpec.format;
					spec.usage = TextureUsage::Attachment;
					spec.width = myWidth;
					spec.height = myHeight;

					if (attachmentSpec.debugName.empty())
					{
						spec.debugName = std::format("{} - Depth Attachment", mySpecification.debugName);
					}
					else
					{
						spec.debugName = std::format("{} - {}", mySpecification.debugName, attachmentSpec.debugName);
					}

					myDepthAttachmentTexture = Texture2D::Create(spec);

					depthAttachmentCreated = true;
				}
				else
				{
					TextureSpecification spec;
					spec.format = attachmentSpec.format;
					spec.usage = TextureUsage::Attachment;
					spec.width = myWidth;
					spec.height = myHeight;

					if (attachmentSpec.debugName.empty())
					{
						spec.debugName = std::format("{} - Color Attachment {}", mySpecification.debugName.c_str(), colorAttachmentIndex);
					}
					else
					{
						spec.debugName = std::format("{} - {}", mySpecification.debugName.c_str(), attachmentSpec.debugName);
					}

					myAttachmentTextures.emplace_back(Texture2D::Create(spec));

					colorAttachmentIndex++;
				}
			}
		}

		//TODO: Rework existing textures
		if (aSpec.existingDepthAttachment)
		{
			myDepthAttachmentTexture = aSpec.existingDepthAttachment;
		}
	}

	void DX11Framebuffer::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		EPOCH_PROFILE_FUNC();

		if (myWidth == aWidth && myHeight == aHeight)
		{
			return;
		}

		myWidth = (uint32_t)(aWidth * mySpecification.scale);
		myHeight = (uint32_t)(aHeight * mySpecification.scale);

		if (mySpecification.existingFramebuffer)
		{
			return;
		}

		if (!mySpecification.existingDepthAttachment && myDepthAttachmentTexture)
		{
			myDepthAttachmentTexture->Resize(myWidth, myHeight);
		}

		if (mySpecification.existingAttachments.size() > 0)
		{
			return;
		}

		for (auto texture : myAttachmentTextures)
		{
			texture->Resize(myWidth, myHeight);
		}
	}
}
