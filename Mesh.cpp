#include "pch.h"
#include "Mesh.h"

#include <cassert>

#include "Effect.h"

dae::Mesh::Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices):
	m_NumIndices{static_cast<uint32_t>(indices.size())},
	m_pEffect{ new Effect{pDevice, L"Resources/PosCol3D.fx"} }
{
	//------------------------
	// Create Vertex layout
	//------------------------
	static constexpr uint32_t numElements{ 2 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

	if(FAILED(result)) assert(false, "Failed Creating InputLayout");

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
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) assert(false, "Failed Creating IndexBuffer");
}

dae::Mesh::~Mesh()
{
	if(m_pInputLayout)m_pInputLayout->Release();
	if(m_pVertexBuffer)m_pVertexBuffer->Release();
	if(m_pIndexBuffer)m_pIndexBuffer->Release();

	delete m_pEffect;
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
	constexpr UINT offset{};
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

void dae::Mesh::Update(const float* pWorldViewMatrixData)
{
	m_pEffect->SetWorldViewProjectionMatrix(pWorldViewMatrixData);
}
