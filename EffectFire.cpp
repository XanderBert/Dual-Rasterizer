#include "pch.h"
#include "EffectFire.h"

#include <cassert>

dae::EffectFire::EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect(pDevice, assetFile)
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if(!m_pDiffuseMapVariable->IsValid())
	{
		std::cout << "Error Opacity DiffuseMap is not loaded!\n";
		assert(false);
	}
}

dae::EffectFire::~EffectFire()
{
	if(m_pDiffuseMapVariable)m_pDiffuseMapVariable->Release();

}

