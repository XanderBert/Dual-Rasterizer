#include "pch.h"
#include "Texture.h"

#include <cassert>

dae::Texture::Texture(const std::string& path, ID3D11Device* pDevice)
{
	SDL_Surface* pSurface = { IMG_Load(path.c_str()) };

	if (!pSurface) 
	{
		std::wcout << L"\SDL_Surface(Texture) not loaded\n";
		assert(false, "\nSDL_Surface(Texture) not loaded\n");
	}


	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);
	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if(result != S_OK)
	{
		std::wcout << "Failed initializing SubResource Texture Data.\n";
		assert(false , "Failed initializing SubResource Texture Data.\n");
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
	if (result != S_OK)
	{
		std::wcout << "Failed initializing D3D11_SHADER_RESOURCE_VIEW_DESC Texture.\n";
		assert(false, "Failed initializing D3D11_SHADER_RESOURCE_VIEW_DESC Texture.\n");
	}

	SDL_FreeSurface(pSurface);
}

dae::Texture::~Texture()
{
	if(m_pResource)m_pResource->Release();
	if(m_pSRV)m_pSRV->Release();
}

ID3D11Texture2D* dae::Texture::GetTexture2D()
{
	return m_pResource;
}

ID3D11ShaderResourceView* dae::Texture::GetSRV()
{
	return m_pSRV;
}
