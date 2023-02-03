#pragma once
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh_Hardware;
	class Camera;
	class Renderer_Hardware final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Renderer_Hardware(SDL_Window* pWindow, Camera* pCamera);
		~Renderer_Hardware();

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Renderer_Hardware(const Renderer_Hardware&) = delete;
		Renderer_Hardware(Renderer_Hardware&&) noexcept = delete;
		Renderer_Hardware& operator=(const Renderer_Hardware&) = delete;
		Renderer_Hardware& operator=(Renderer_Hardware&&) noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
		void Update(const Timer* pTimer, const LPCSTR& technique);
		void Render() const;

		void ToggleMeshRotation();

	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh_Hardware* m_pMesh{ nullptr };

		Camera* m_pCamera{ nullptr };
		LPCSTR m_CurrentTechnique{"DefaultTechniquePoint"};

		const float m_NearPlane{};
		const float m_FarPlane{};

		bool m_IsMeshRotating{ false };

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
