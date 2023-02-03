#pragma once
#include "Effect.h"

namespace dae
{


	class EffectShading final : public Effect
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		EffectShading(ID3D11Device* pDevice, const std::wstring& assetFile);
		~EffectShading() override;

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		EffectShading(const EffectShading& other) = delete;
		EffectShading(EffectShading&& other)noexcept = delete;
		EffectShading& operator=(const EffectShading& other) = delete;
		EffectShading& operator=(EffectShading&& other)noexcept = delete;

		void SetNormalMap(Texture_Hardware* pNormalTexture);
		void SetSpecularMap(Texture_Hardware* pNormalTexture);
		void SetGlossinessMap(Texture_Hardware* pNormalTexture);

		void SetTechnique(const LPCSTR& techniqueName);
	private:

		ID3DX11EffectShaderResourceVariable* m_pNormalMapVarialbe{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{ nullptr };
	};
}
