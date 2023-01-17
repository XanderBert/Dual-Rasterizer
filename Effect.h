#pragma once
namespace dae
{
	class Texture;
	class Effect final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~Effect();

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Effect(const Effect& other) = delete;
		Effect(Effect&& other)noexcept = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect& operator=(Effect&& other)noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;

		void SetWorldViewProjectionMatrix(const float* pData);
		void SetWorldMatrix(const float* pData);
		void SetInverseViewMatrix(const float* pData);

		//Todo Make this one function
		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);
		//todo end

		void SetTechnique(const LPCSTR& techniqueName);

	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		ID3DX11Effect* m_pEffect{ nullptr };
		ID3DX11EffectTechnique* m_pTechnique{ nullptr };

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatInverseViewVariable{ nullptr };


		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVarialbe{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{ nullptr };

		//----------------------------------------
		// General Methods
		//----------------------------------------
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		bool GetShaderResources();
	};
}