#pragma once

namespace dae
{
	class Texture;
	class Effect;
	struct Vertex final
	{
		Vector3 Position;
		//ColorRGB Color;
		Vector3 tangent;
		Vector3 normal;
		Vector2 uv;
	};

	class Mesh final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		Mesh(ID3D11Device* pDevice, const std::string& objFile, const std::string& diffuseFile, const std::string& normalFile, const std::string& specularFile, const std::string& glossinessFile);
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
		void Update(const float* pWorldViewMatrixData, const float* pWorldMatrixData, const float* pInverseMatrixData);
		void SetTechnique(const LPCSTR& techniqueName);
	private:
		HRESULT SetUpMesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

		//----------------------------------------
		// Datamembers
		//----------------------------------------
		uint32_t m_NumIndices{};

		Effect* m_pEffect{ nullptr };
		ID3D11InputLayout* m_pInputLayout{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };

		Texture* m_pDiffuseMap{ nullptr };
		Texture* m_pNormalMap{ nullptr };
		Texture* m_pSpecularMap{ nullptr };
		Texture* m_pGlossinessMap{ nullptr };
	};
}