#pragma once
namespace dae
{
	class Texture final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Texture(const std::string& path, ID3D11Device* pDevice );
		~Texture();
		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Texture(const Texture& other) = delete;
		Texture(Texture&& other)noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other)noexcept = delete;
		//----------------------------------------
		// General Methods
		//----------------------------------------

	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		ID3D11Texture2D* m_pResource{nullptr};
		ID3D11ShaderResourceView* m_pSRV{ nullptr };
	};
}