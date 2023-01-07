#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Camera;
	class Renderer final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Renderer(SDL_Window* pWindow);
		~Renderer();

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
		void Update(const Timer* pTimer, const LPCSTR& technique);
		void Render() const;

	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh* m_pMesh{ nullptr };

		Camera* m_pCamera{ nullptr };
		LPCSTR m_CurrentTechnique{};

		//Pointers for Initialization
		ID3D11Device* m_pDevice						{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext		{ nullptr };
		IDXGISwapChain* m_pSwapChain				{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer		{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView	{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer		{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView	{ nullptr };

		//----------------------------------------
		// General Methods
		//----------------------------------------
		HRESULT InitializeDirectX();
		void SetIsInitialized(const HRESULT& result);

	};
}
