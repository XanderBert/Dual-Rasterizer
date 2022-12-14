#pragma once

class Effect;

namespace dae
{
	class Effect;
	struct Vertex
	{
		Vector3 Position;
		ColorRGB Color;
	};

	class Mesh final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		~Mesh();

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other)noexcept = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other)noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
		void Render(ID3D11DeviceContext* pDeviceContext) const;
	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		uint32_t m_NumIndices{};

		Effect* m_pEffect{ nullptr };
		ID3D11InputLayout* m_pInputLayout{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };
	};
}