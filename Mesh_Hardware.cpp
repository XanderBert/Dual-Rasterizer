#include "pch.h"
#include "Mesh_Hardware.h"
#include <cassert>
#include "EffectShading.h"
#include "Texture_Hardware.h"
#include "Utils.h"
#include "EffectFire.h"


dae::Mesh_Hardware::Mesh_Hardware(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices):
	m_NumIndices{static_cast<uint32_t>(indices.size())},
	m_pEffectModel{ new EffectShading{pDevice, L"Resources/PosCol3D.fx"} }
{
	SetUpMesh(pDevice, vertices, indices);
}

dae::Mesh_Hardware::Mesh_Hardware(ID3D11Device* pDevice, const std::string& objFile, const std::string& opacityObjFile, const std::string& diffuseFile, const std::string& normalFile, const std::string& specularFile, const std::string& glossinessFile, const std::string& fireAlbedoFile)
:m_pEffectModel{ new EffectShading{pDevice, L"Resources/PosCol3D.fx"} },
m_pEffectFire(new EffectFire(pDevice, L"Resources/PosCol3DOpacity.fx"))
{
	m_WorldMatrix = { };

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	const bool mainMeshObj{ Utils::ParseOBJ(objFile,vertices, indices) };
	if (!mainMeshObj)
	{
		std::cout << "Failed to load OBJ \n";
		assert(false, "Failed to load OBJ");
		return;
	}

	m_pDiffuseMap = new Texture_Hardware{ diffuseFile, pDevice };
	if(m_pDiffuseMap)
	{
		m_pEffectModel->SetDiffuseMap(m_pDiffuseMap);
	}else
	{
		std::wcout << "Texture_Hardware Not loaded!";
	}

	m_pNormalMap = new Texture_Hardware{ normalFile, pDevice };
	if (m_pNormalMap)
	{
		m_pEffectModel->SetNormalMap(m_pNormalMap);
	}
	else
	{
		std::wcout << "Texture_Hardware Not loaded!";
	}

	m_pSpecularMap = new Texture_Hardware{ specularFile, pDevice };
	if (m_pSpecularMap)
	{
		m_pEffectModel->SetSpecularMap(m_pSpecularMap);
	}
	else
	{
		std::wcout << "Texture_Hardware Not loaded!";
	}

	m_pGlossinessMap = new Texture_Hardware{ glossinessFile, pDevice };
	if (m_pGlossinessMap)
	{
		m_pEffectModel->SetGlossinessMap(m_pGlossinessMap);
	}
	else
	{
		std::wcout << "Texture_Hardware Not loaded!";
	}


	m_NumIndices = static_cast<uint32_t>(indices.size());
	SetUpMesh(pDevice, vertices, indices);

	vertices.clear();
	indices.clear();

	m_pFireAlbedo = new Texture_Hardware{ fireAlbedoFile, pDevice };
	if (m_pFireAlbedo)
	{
		m_pEffectFire->SetDiffuseMap(m_pFireAlbedo);
	}
	else
	{
		std::wcout << "Texture_Hardware Not loaded!";
	}


	const bool objResult{ Utils::ParseOBJ(opacityObjFile,vertices, indices) };
	if (!objResult)
	{
		std::cout << "Failed to load OBJ \n";
		assert(false, "Failed to load OBJ");
		return;
	}

	m_NumIndicesOpacity = static_cast<uint32_t>(indices.size());
	SetUpMeshOpacity(pDevice, vertices, indices);

}

dae::Mesh_Hardware::~Mesh_Hardware()
{
	if(m_pInputLayout)m_pInputLayout->Release();
	if(m_pVertexBuffer)m_pVertexBuffer->Release();
	if(m_pIndexBuffer)m_pIndexBuffer->Release();

	if (m_pInputLayoutOpacity)m_pInputLayoutOpacity->Release();
	if (m_pVertexBufferOpacity)m_pVertexBufferOpacity->Release();
	if (m_pIndexBufferOpacity)m_pIndexBufferOpacity->Release();

	delete m_pEffectModel;
	delete m_pEffectFire;
	delete m_pDiffuseMap;
	delete m_pNormalMap;
	delete m_pSpecularMap;
	delete m_pGlossinessMap;
	delete m_pFireAlbedo;
}

void dae::Mesh_Hardware::Render(ID3D11DeviceContext* pDeviceContext) const
{
	//------------------------
	// Set primitive topology
	//------------------------
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//------------------------
	// Set input layout
	//------------------------
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//------------------------
	// Set vertex buffer
	//------------------------
	constexpr UINT stride{ sizeof(Vertex) };
	constexpr UINT offset{0};
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//------------------------
	// Set index buffer
	//------------------------
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//------------------------
	// Draw
	//------------------------
	D3DX11_TECHNIQUE_DESC techniqueDesc{};
	m_pEffectModel->GetTechnique()->GetDesc(&techniqueDesc);
	for (UINT p{}; p < techniqueDesc.Passes; ++p)
	{
		m_pEffectModel->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}



	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetInputLayout(m_pInputLayoutOpacity);
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBufferOpacity, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(m_pIndexBufferOpacity, DXGI_FORMAT_R32_UINT, 0);

	//------------------------
	// Draw
	//------------------------
	D3DX11_TECHNIQUE_DESC techniqueDescfire{};
	m_pEffectFire->GetTechnique()->GetDesc(&techniqueDescfire);
	for (UINT p{}; p < techniqueDescfire.Passes; ++p)
	{
		m_pEffectFire->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndicesOpacity, 0, 0);
	}


}

void dae::Mesh_Hardware::Update(const float* pWorldViewMatrixData, const float* pInverseMatrixData)
{
	m_pEffectModel->SetWorldViewProjectionMatrix(pWorldViewMatrixData);
	m_pEffectModel->SetWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));
	m_pEffectModel->SetInverseViewMatrix(pInverseMatrixData);

	m_pEffectFire->SetWorldViewProjectionMatrix(pWorldViewMatrixData);
	m_pEffectFire->SetWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));
	m_pEffectFire->SetInverseViewMatrix(pInverseMatrixData);
}

void dae::Mesh_Hardware::SetTechnique(const LPCSTR& techniqueName)
{
	m_pEffectModel->SetTechnique(techniqueName);
}

void dae::Mesh_Hardware::Rotate(float angle)
{
	m_WorldMatrix = Matrix::CreateRotationY(angle)* m_WorldMatrix;
	m_pEffectModel->SetWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));
}

HRESULT dae::Mesh_Hardware::SetUpMesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	//------------------------
	// Create Vertex layout
	//------------------------
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 12;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TANGENT";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 24;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXCOORD";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//------------------------
	// Create Input Layout
	//------------------------
	D3DX11_PASS_DESC passDesc{};
	m_pEffectModel->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout
	);

	if (FAILED(result)) { assert(false, "Failed Creating InputLayout");}

	//------------------------
	// Create Vertex Buffer
	//------------------------
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result)) assert(false, "Failed Creating VertexBuffer");

	//------------------------
	// Create Index Buffer
	//------------------------
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) assert(false, "Failed Creating IndexBuffer");

	return result;
}

HRESULT dae::Mesh_Hardware::SetUpMeshOpacity(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{

//------------------------
// Create Vertex layout
//------------------------
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 12;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TANGENT";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 24;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXCOORD";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//------------------------
	// Create Input Layout
	//------------------------
	D3DX11_PASS_DESC passDesc{};
	m_pEffectFire->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayoutOpacity
	);

	if (FAILED(result)) { assert(false, "Failed Creating InputLayout"); }

	//------------------------
	// Create Vertex Buffer
	//------------------------
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBufferOpacity);
	if (FAILED(result)) assert(false, "Failed Creating VertexBuffer");

	//------------------------
	// Create Index Buffer
	//------------------------
	m_NumIndicesOpacity = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndicesOpacity;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBufferOpacity);
	if (FAILED(result)) assert(false, "Failed Creating IndexBuffer");

	return result;
}
