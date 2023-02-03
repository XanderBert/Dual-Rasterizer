#include "pch.h"
#include "Renderer_Hardware.h"
#include <cassert>
#include "Mesh_Hardware.h"
#include "Camera.h"
#include "Effect.h"

namespace dae {

	Renderer_Hardware::Renderer_Hardware(SDL_Window* pWindow, Camera* pCamera) :
		m_pWindow(pWindow)
	,	m_NearPlane(1.f)
	,	m_FarPlane(100.f)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		SetIsInitialized(InitializeDirectX());

		//Initialize Camera
		m_pCamera = pCamera;

		//Create Data for our mesh
		m_pMesh = new Mesh_Hardware{ m_pDevice,
			"Resources/vehicle.obj",
			"Resources/fireFX.obj",
			"Resources/vehicle_diffuse.png",
			"Resources/vehicle_normal.png",
			"Resources/vehicle_specular.png",
			"Resources/vehicle_gloss.png",
			"Resources/fireFX_diffuse.png"
		};
		
	}

	Renderer_Hardware::~Renderer_Hardware()
	{
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();

		delete m_pMesh;
		//delete m_pCamera;		
	}

	void Renderer_Hardware::Update(const Timer* pTimer, const LPCSTR& technique)
	{
		const Matrix worldViewMatrix{ m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix(m_NearPlane,m_FarPlane) };
		const Matrix inverseMatrix{m_pCamera->GetInverseViewmatrix()};
		m_pMesh->Update(reinterpret_cast<const float*>(&worldViewMatrix), reinterpret_cast<const float*>(&inverseMatrix));

		//change technique if it changed
		if (m_CurrentTechnique != technique)
		{
			m_pMesh->SetTechnique(technique);
			m_CurrentTechnique = technique;
		}

		if (m_IsMeshRotating)
		{
			constexpr float meshRotationPerSecond{ 0.25f };
			m_pMesh->Rotate(meshRotationPerSecond * pTimer->GetElapsed());
		}
	}

	void Renderer_Hardware::Render() const
	{
		if (!m_IsInitialized) return;
		//1. Clear RTV and DSV
		//
		constexpr ColorRGB clearColor = ColorRGB{ 0.39f, 0.59f, 0.93f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set pipeline + Invoke Drawcalls (= RENDER)
		//
		m_pMesh->Render(m_pDeviceContext);

		//3.Present BackBuffer (SWAP)
		//
		m_pSwapChain->Present(0, 0);
	}

	void Renderer_Hardware::ToggleMeshRotation()
	{
		m_IsMeshRotating = !m_IsMeshRotating;
	}

	HRESULT Renderer_Hardware::InitializeDirectX()
	{
		//Select DirectX11
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

		//Enalbe DirectX11 Debugging if debugging is enabled
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		//Create result to check if our initializations have failed or not
		HRESULT result{};

		//Create the device
		//
		result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
		{
			std::cout << "\nDevice Created Failed!\n";
			assert(false, "\nDevice Created Failed!\n");
			return result;
		} 

		//Create DXGI Factory for the Swapchain
		//
		IDXGIFactory1* pDxgiFactory{ nullptr };
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
		{
			pDxgiFactory->Release();

			std::cout << "\nCreate DXGI Factory for the Swapchain Failed!\n";
			assert(false, "\nCreate DXGI Factory for the Swapchain Failed!\n");
			return result;
		}

		//Initialize the Swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height	= m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handel (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create the Swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		pDxgiFactory->Release();
		if (FAILED(result))
		{
			std::cout << "\nCreate the Swapchain Failed!\n";
			assert(false, "\nCreate the Swapchain Failed!\n");
			return result;
		}

		//Create DepthStencil (DS) & DepthStencilView (DSV)
		//It will mask pixels of an image that does not need to be rendered
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		//Create the resource and the resourceView
		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
		{
			std::cout << "\nCreate the resource Failed!\n";
			assert(false, "\nCreate the resource Failed!\n");
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
		{
			std::cout << "\nCreate the resourceView Failed!\n";
			assert(false, "\nCreate the resourceView Failed!\n");
			return result;
		}


		//Create RenderTarget (RT) and RenderTargetView (RTV)
		//
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
		{
			std::cout << "\nCreate the RenderTarget Failed!\n";
			assert(false, "\nCreate the RenderTarget Failed!\n");
			return result;
		}

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
		{
			std::cout << "\nCreate the RenderTargetView Failed!\n";
			assert(false, "\nCreate the RenderTargetView Failed!\n");
			return result;
		}

		//Bind RTV & DSV to Output Merger Stage
		//
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//Set Viewport
		//
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}

	void Renderer_Hardware::SetIsInitialized(const HRESULT& result)
	{
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
			assert(false, "DirectX initialization failed!\n");
		}
	}
}
