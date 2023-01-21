#include "pch.h"
#include "Mesh.h"
#include <cassert>
#include "Effect.h"
#include "Texture.h"
#include "Utils.h"

dae::Mesh::Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices):
	m_NumIndices{static_cast<uint32_t>(indices.size())},
	m_pEffect{ new Effect{pDevice, L"Resources/PosCol3D.fx"} }
{
	SetUpMesh(pDevice, vertices, indices);
}

dae::Mesh::Mesh(ID3D11Device* pDevice, const std::string& objFile, const std::string& diffuseFile, const std::string& normalFile, const std::string& specularFile, const std::string& glossinessFile)

:m_pEffect{ new Effect{pDevice, L"Resources/PosCol3D.fx"} }
{
	m_WorldMatrix = { {1.f, 0.f, 0.f, 0.f},
						{0.f, 1.f, 0.f, 0.f},
						{0.f, 0.f, 1.f, 0.f},
							{0.f, 0.f, 0.f, 1.f} };


	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	const bool objResult{ Utils::ParseOBJ(objFile,vertices, indices) };
	if (!objResult)
	{
		std::cout << "Failed to load OBJ \n";
		assert(false, "Failed to load OBJ");
		return;
	}

	//TODO Make function to load all maps.
	m_pDiffuseMap = new Texture{ diffuseFile, pDevice };
	if(m_pDiffuseMap)
	{
		m_pEffect->SetDiffuseMap(m_pDiffuseMap);
	}else
	{
		std::wcout << "Texture Not loaded!";
	}

	m_pNormalMap = new Texture{ normalFile, pDevice };
	if (m_pNormalMap)
	{
		m_pEffect->SetNormalMap(m_pNormalMap);
	}
	else
	{
		std::wcout << "Texture Not loaded!";
	}

	m_pSpecularMap = new Texture{ specularFile, pDevice };
	if (m_pSpecularMap)
	{
		m_pEffect->SetSpecularMap(m_pSpecularMap);
	}
	else
	{
		std::wcout << "Texture Not loaded!";
	}

	m_pGlossinessMap = new Texture{ glossinessFile, pDevice };
	if (m_pGlossinessMap)
	{
		m_pEffect->SetGlossinessMap(m_pGlossinessMap);
	}
	else
	{
		std::wcout << "Texture Not loaded!";
	}

	m_NumIndices = static_cast<uint32_t>(indices.size());
	SetUpMesh(pDevice, vertices, indices);
}

dae::Mesh::~Mesh()
{
	if(m_pInputLayout)m_pInputLayout->Release();
	if(m_pVertexBuffer)m_pVertexBuffer->Release();
	if(m_pIndexBuffer)m_pIndexBuffer->Release();

	delete m_pEffect;
	delete m_pDiffuseMap;
	delete m_pNormalMap;
	delete m_pSpecularMap;
	delete m_pGlossinessMap;
}

void dae::Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
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
	m_pEffect->GetTechnique()->GetDesc(&techniqueDesc);
	for (UINT p{}; p < techniqueDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void dae::Mesh::Update(const float* pWorldViewMatrixData, const float* pInverseMatrixData)
{
	m_pEffect->SetWorldViewProjectionMatrix(pWorldViewMatrixData);
	m_pEffect->SetWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));
	m_pEffect->SetInverseViewMatrix(pInverseMatrixData);
}

void dae::Mesh::SetTechnique(const LPCSTR& techniqueName)
{
	m_pEffect->SetTechnique(techniqueName);
}

HRESULT dae::Mesh::SetUpMesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	//for (auto element : vertices)
	//{
	//	std::cout << std::to_string(element.tangent.x) << ', ';
	//	std::cout << std::to_string(element.tangent.y) << ', ';
	//	std::cout << std::to_string(element.tangent.z) << '\n';
	//}

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
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

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
