#include "epch.h"
#include "RendererAPI.h"

namespace Epoch
{
	std::string RendererAPI::CurrentName()
	{
		switch (staticCurrentRendererAPI)
		{
		case Epoch::RendererAPIType::DirectX11:
			return "DirectX11";
			break;
		case Epoch::RendererAPIType::DirectX12:
			return "DirectX12";
			break;
		default:
			return "Unkown";
			break;
		}
	}

	void Epoch::RendererAPI::SetAPI(RendererAPIType aApi)
	{
		EPOCH_ASSERT(aApi == RendererAPIType::DirectX11, "DirectX 11 is currently the only supported Renderer API");
		staticCurrentRendererAPI = aApi;
	}
}
