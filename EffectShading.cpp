#include "pch.h"
#include "EffectShading.h"

#include <cassert>

#include "Texture_Hardware.h"

dae::EffectShading::EffectShading(ID3D11Device* pDevice, const std::wstring& assetFile):Effect(pDevice, assetFile)
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"\nm_pDiffuseMapVariable not valid\n";
		assert(false, "\nm_pDiffuseMapVariable not valid\n");
	}


	m_pNormalMapVarialbe = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVarialbe->IsValid())
	{
		std::wcout << L"\nm_pNormalMapVarialbe not valid\n";
		assert(false, "\nm_pNormalMapVarialbe not valid\n");
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"\nm_pSpecularMapVariable not valid\n";
		assert(false, "\nm_pSpecularMapVariable not valid\n");
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"\nm_pGlossinessMapVariable not valid\n";
		assert(false, "\nm_pGlossinessMapVariable not valid\n");
	}

}

dae::EffectShading::~EffectShading()
{
	if(m_pDiffuseMapVariable)m_pDiffuseMapVariable->Release();
	if(m_pNormalMapVarialbe)m_pNormalMapVarialbe->Release();
	if(m_pSpecularMapVariable)m_pSpecularMapVariable->Release();
	if(m_pGlossinessMapVariable)m_pGlossinessMapVariable->Release();
}

void dae::EffectShading::SetNormalMap(Texture_Hardware* pNormalTexture)
{
	if (m_pNormalMapVarialbe)
	{
		m_pNormalMapVarialbe->SetResource(pNormalTexture->GetSRV());
	}
}

void dae::EffectShading::SetSpecularMap(Texture_Hardware* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
	}
}

void dae::EffectShading::SetGlossinessMap(Texture_Hardware* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
	}
}

void dae::EffectShading::SetTechnique(const LPCSTR& techniqueName)
{
	m_pTechnique = m_pEffect->GetTechniqueByName(techniqueName);
	std::cout << '\n' << techniqueName << '\n';
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"\nTechnique not valid\n";
		assert(false, "\nTechnique not valid\n");
	}
}