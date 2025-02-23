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

		if (!aSpec.existingFramebuffer)
		{

			int colorAttachmentIndex = 0;
			bool depthAttachmentCreated = false;
			for (auto& attachmentSpec : aSpec.attachments.attachments)
			{
				std::string attachmentName = attachmentSpec.name;
				EPOCH_ASSERT(!attachmentName.empty(), "A frame buffers attachments name can't be empty!")

				if (IsDepthFormat(attachmentSpec.format))
				{
					if (depthAttachmentCreated)
					{
						LOG_ERROR("A framebuffer can't have more than one depth attachment! '{}'", aSpec.debugName);
						continue;
					}

					if (aSpec.existingDepthAttachment)
					{
						myDepthAttachmentTexture = aSpec.existingDepthAttachment;
					}
					else
					{
						TextureSpecification spec;
						spec.format = attachmentSpec.format;
						spec.usage = TextureUsage::Attachment;
						spec.width = myWidth;
						spec.height = myHeight;
						spec.debugName = std::format("{} - {}", mySpecification.debugName, attachmentName);

						myDepthAttachmentTexture = Texture2D::Create(spec);
					}

					depthAttachmentCreated = true;
				}
				else
				{
					if (aSpec.existingColorAttachments.find(colorAttachmentIndex) != aSpec.existingColorAttachments.end())
					{
						myColorAttachment.push_back(aSpec.existingColorAttachments.at(colorAttachmentIndex));
					}
					else
					{
						TextureSpecification spec;
						spec.format = attachmentSpec.format;
						spec.usage = TextureUsage::Attachment;
						spec.width = myWidth;
						spec.height = myHeight;
						spec.debugName = std::format("{} - {}", mySpecification.debugName, attachmentName);

						myColorAttachment.emplace_back(Texture2D::Create(spec));
					}

					myColorAttachmentNameMap[attachmentName] = colorAttachmentIndex;
					colorAttachmentIndex++;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < aSpec.existingFramebuffer->GetColorAttachmentCount(); i++)
			{
				myColorAttachment.emplace_back(aSpec.existingFramebuffer->GetTarget((uint32_t)i));
			}

			if (aSpec.existingFramebuffer->HasDepthAttachment())
			{
				myDepthAttachmentTexture = aSpec.existingFramebuffer->GetDepthAttachment();
			}

			myColorAttachmentNameMap = aSpec.existingFramebuffer->GetColorAttachmentNameMap();
		}
	}

	void DX11Framebuffer::Resize(uint32_t aWidth, uint32_t aHeight)
	{
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

		for (size_t i = 0; i < myColorAttachment.size(); i++)
		{
			if (mySpecification.existingColorAttachments.find((uint32_t)i) != mySpecification.existingColorAttachments.end()) continue;

			myColorAttachment[i]->Resize(myWidth, myHeight);
		}
	}
}
