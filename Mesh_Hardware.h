#pragma once

namespace dae
{
	class Effect;
	class Texture_Hardware;
	class EffectShading;
	class EffectFire;
	struct Vertex final
	{
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector2 uv;
	};

	class Mesh_Hardware final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Mesh_Hardware(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		Mesh_Hardware(ID3D11Device* pDevice, const std::string& objFile, const std::string& opacityObjFile, const std::string& diffuseFile, const std::string& normalFile, const std::string& specularFile, const std::string& glossinessFile, const std::string& fireAlbedoFile);
		~Mesh_Hardware();

		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Mesh_Hardware(const Mesh_Hardware& other) = delete;
		Mesh_Hardware(Mesh_Hardware&& other)noexcept = delete;
		Mesh_Hardware& operator=(const Mesh_Hardware& other) = delete;
		Mesh_Hardware& operator=(Mesh_Hardware&& other)noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void Update(const float* pWorldViewMatrixData, const float* pInverseMatrixData);
		void SetTechnique(const LPCSTR& techniqueName);
		void Rotate(float angle);
	private:
		HRESULT SetUpMesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		HRESULT SetUpMeshOpacity(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

		//----------------------------------------
		// Datamembers
		//----------------------------------------
		uint32_t m_NumIndices{};
		uint32_t m_NumIndicesOpacity{};

		EffectShading* m_pEffectModel{ nullptr };
		EffectFire* m_pEffectFire{ nullptr };

		ID3D11InputLayout* m_pInputLayout{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };


		ID3D11InputLayout* m_pInputLayoutOpacity{ nullptr };
		ID3D11Buffer* m_pVertexBufferOpacity{ nullptr };
		ID3D11Buffer* m_pIndexBufferOpacity{ nullptr };

		Texture_Hardware* m_pDiffuseMap{ nullptr };
		Texture_Hardware* m_pNormalMap{ nullptr };
		Texture_Hardware* m_pSpecularMap{ nullptr };
		Texture_Hardware* m_pGlossinessMap{ nullptr };
		Texture_Hardware* m_pFireAlbedo{ nullptr };

		Matrix m_WorldMatrix{};
	};
}