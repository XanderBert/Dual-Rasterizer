#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture_Software;
	struct Mesh_Software;
	struct Vertex_Software;
	class Timer;
	class Scene;

	class Renderer_Software final
	{
	public:
		Renderer_Software(SDL_Window* pWindow, Camera* pCamera);
		~Renderer_Software();

		Renderer_Software(const Renderer_Software&) = delete;
		Renderer_Software(Renderer_Software&&) noexcept = delete;
		Renderer_Software& operator=(const Renderer_Software&) = delete;
		Renderer_Software& operator=(Renderer_Software&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();
		bool SaveBufferToImage() const;

		void ToggleBounding();
		void ToggleDepth();
		void ToggleLightingMode();
		void ToggleNormalMap();
		void ToggleMeshRotation();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera* m_pCamera { nullptr };

		Texture_Software* m_pTexture{};
		Texture_Software* m_pNormalTexture{};
		Texture_Software* m_pGlossinessTexture{};
		Texture_Software* m_pSpecularTexture{};

		bool m_IsNormalActive{ false };
		bool m_IsMeshRotating{ false };
		bool m_IsDepthShown{ false };
		bool m_IsBoundingBoxShown{ false };

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};

		Mesh_Software m_Mesh{};

		enum class RenderMode
		{
			Normal,
			DepthBuffer,
			BoundingBox,
			Last
		};

		enum class LightingMode
		{
			Combined,
			Diffuse,
			ObservedArea,
			Specular,
			Last
		};

		RenderMode m_RenderMode{ RenderMode::Normal };
		LightingMode m_LightingMode{ LightingMode::Combined };
		
		void RenderTriangle(const std::vector<Vector2>& rasterVertices, int vertexIdx, bool swapVertices) const;
		void ClearBackground() const;
		void ResetDepthBuffer() const;
		void Shade(int pixelIndex,Vertex_Out pxlInfo) const;
		void InitializeBuffer(SDL_Window* pWindow);
		void InitializeMesh(const char* filename);
		void ResetState();
		void WorldToNDC(const Matrix& worldViewProjectionMatrix);
		void NDCToRaster(std::vector<Vector2>& rasterVertices) const;
		void UpdateSDL() const;
		[[nodiscard]] bool IsVertexSame(uint32_t vertex0, uint32_t vertex1, uint32_t vertex2) const;
		[[nodiscard]] bool IsOutsideFrustum(uint32_t vertex0, uint32_t vertex1, uint32_t vertex2) const;
		void CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, int& startingX, int& StartingY, int& endingX, int& endingY)const;
		void RenderBoundingBox(const int pixelIndex) const;
		[[nodiscard]] bool IsInsideTriangle(const float edgePoint01, const float edgePoint12, const float edgePoint20) const;
		[[nodiscard]] bool IsCurrentDepthBufferLessThenDepth(const int pixelIndex, const float ZDepth) const;
		void CalculatePixelInfo(Vertex_Out& pixelInfo, float weightV0, float weightV1, float weightV2, uint32_t vertIndex0, uint32_t vertIndex1, uint32_t vertIndex2, float wDepth) const;
		void RemapZDepth(float interpolatedZDepth, float& depthColor)const;
		ColorRGB CalculatePhong(const float exponent, const Vector3& lightDirection, const Vector3& viewDirection, const Vector3& normal) const;
	};
}