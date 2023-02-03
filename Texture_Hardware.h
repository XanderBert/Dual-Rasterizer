#pragma once
namespace dae
{
	class Texture_Hardware final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Texture_Hardware(const std::string& path, ID3D11Device* pDevice );
		~Texture_Hardware();
		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Texture_Hardware(const Texture_Hardware& other) = delete;
		Texture_Hardware(Texture_Hardware&& other)noexcept = delete;
		Texture_Hardware& operator=(const Texture_Hardware& other) = delete;
		Texture_Hardware& operator=(Texture_Hardware&& other)noexcept = delete;
		//----------------------------------------
		// General Methods
		//----------------------------------------
		ID3D11Texture2D* GetTexture2D();
		ID3D11ShaderResourceView* GetSRV();
	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		ID3D11Texture2D* m_pResource{nullptr};
		ID3D11ShaderResourceView* m_pSRV{ nullptr };
	};
}