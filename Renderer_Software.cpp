#include "pch.h"
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer_Software.h"

#include <cassert>

#include "Math.h"
#include "Vector2.h"
#include "Matrix.h"
#include "Texture_Software.h"
#include "Utils.h"
#include <future>
#include <ppl.h>

#include "Timer.h"

using namespace dae;

Renderer_Software::Renderer_Software(SDL_Window* pWindow, Camera* pCamera)
	:m_pWindow(pWindow)
	,m_pTexture{ Texture_Software::LoadFromFile("Resources/vehicle_diffuse.png") }
	,m_pNormalTexture(Texture_Software::LoadFromFile("Resources/vehicle_normal.png"))
	,m_pGlossinessTexture(Texture_Software::LoadFromFile("Resources/vehicle_gloss.png"))
	,m_pSpecularTexture(Texture_Software::LoadFromFile("Resources/vehicle_specular.png"))
{
	//Initialize
	m_pCamera = pCamera;
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	InitializeBuffer(pWindow);
	InitializeMesh("Resources/vehicle.obj");

}

Renderer_Software::~Renderer_Software()
{
	delete m_pTexture;
	delete m_pNormalTexture;
	delete m_pGlossinessTexture;
	delete m_pSpecularTexture;

	SDL_FreeSurface(m_pFrontBuffer);
	SDL_FreeSurface(m_pBackBuffer);
	//delete[] m_pBackBufferPixels; // Where is it freed?
	delete[] m_pDepthBufferPixels;
	//delete m_pCamera;
}

void Renderer_Software::Update(Timer* pTimer)
{
	if (m_IsMeshRotating)
	{
		constexpr float meshRotationPerSecond{ 0.25f };
		m_Mesh.Rotate(meshRotationPerSecond * pTimer->GetElapsed());
	}
}

void Renderer_Software::Render()
{
	ResetState();

	const Matrix worldViewProjectionMatrix{ m_Mesh.worldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix(1.f,100.f) };
	WorldToNDC(worldViewProjectionMatrix);

	std::vector<Vector2> rasterVertices{};
	NDCToRaster(rasterVertices);

	//Render on TopologyType
	switch (m_Mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		for (int VertexIndex{}; VertexIndex < m_Mesh.indices.size(); VertexIndex += 3)
		{
			RenderTriangle(rasterVertices, VertexIndex, false);
		}
		break;
	case PrimitiveTopology::TriangleStrip:
		for (int VertexIndex{}; VertexIndex < m_Mesh.indices.size() - 2; ++VertexIndex)
		{
			RenderTriangle(rasterVertices, VertexIndex, VertexIndex % 2);
		}
		break;
	}

	UpdateSDL();
}

void dae::Renderer_Software::RenderTriangle(const std::vector<Vector2>& rasterVertices, int curVertexIdx, bool swapVertices) const
{
	const uint32_t vertIndex0{ m_Mesh.indices[curVertexIdx] };
	const uint32_t vertIndex1{ m_Mesh.indices[curVertexIdx + 1 * !swapVertices + 2 * swapVertices] };
	const uint32_t vertIndex2{ m_Mesh.indices[curVertexIdx + 2 * !swapVertices + 1 * swapVertices] };

	if (IsVertexSame(vertIndex0, vertIndex1, vertIndex2) || IsOutsideFrustum(vertIndex0, vertIndex1, vertIndex2)) return;

	const Vector2 v0{ rasterVertices[vertIndex0] };
	const Vector2 v1{ rasterVertices[vertIndex1] };
	const Vector2 v2{ rasterVertices[vertIndex2] };
	const Vector2 edge01{ v1 - v0 };
	const Vector2 edge12{ v2 - v1 };
	const Vector2 edge20{ v0 - v2 };

	//Area
	const float triangleArea{ Vector2::Cross(edge01, edge12) };
	if (triangleArea < FLT_EPSILON) return;

	//BoundingBox
	int startingX, StartingY, endingX, endingY;
	CalculateBoundingBox(v0, v1, v2, startingX, StartingY, endingX, endingY);


	for (int py{ StartingY }; py < endingY; ++py)
	{
		for (int px{ startingX }; px < endingX; ++px)
		{
			// Calculate the pixel index and create a Vector2 of the current pixel
			const int pixelIdx{ px + py * m_Width };
			const Vector2 curPixel{ static_cast<float>(px), static_cast<float>(py) };

			if (m_RenderMode == RenderMode::BoundingBox)
			{
				RenderBoundingBox(pixelIdx);
				continue;
			}

			//VertexToPoint
			const Vector2 v0ToPoint{ curPixel - v0 };
			const Vector2 v1ToPoint{ curPixel - v1 };
			const Vector2 v2ToPoint{ curPixel - v2 };
			const float edge01Point{ Vector2::Cross(edge01, v0ToPoint) };
			const float edge12Point{ Vector2::Cross(edge12, v1ToPoint) };
			const float edge20Point{ Vector2::Cross(edge20, v2ToPoint) };
			if (!IsInsideTriangle(edge01Point, edge12Point, edge20Point)) continue;

			// Barycentric weights
			const float weightV0{ edge12Point / triangleArea };
			const float weightV1{ edge20Point / triangleArea };
			const float weightV2{ edge01Point / triangleArea };

			// Calculate the Z depth at this pixel
			const float interpolatedZDepth
			{
				1.0f / (weightV0 / m_Mesh.vertices_out[vertIndex0].position.z +
						weightV1 / m_Mesh.vertices_out[vertIndex1].position.z +
						weightV2 / m_Mesh.vertices_out[vertIndex2].position.z)
			};
			if (IsCurrentDepthBufferLessThenDepth(pixelIdx, interpolatedZDepth)) continue;
			m_pDepthBufferPixels[pixelIdx] = interpolatedZDepth;


			Vertex_Out pixelInfo{};

			// Switch between all the render states
			switch (m_RenderMode)
			{
			case RenderMode::Normal:
			{
				// Calculate the W depth
				const float interpolatedWDepth
				{
					1.0f /
						(weightV0 / m_Mesh.vertices_out[vertIndex0].position.w +
						weightV1 / m_Mesh.vertices_out[vertIndex1].position.w +
						weightV2 / m_Mesh.vertices_out[vertIndex2].position.w)
				};
				CalculatePixelInfo(pixelInfo, weightV0, weightV1, weightV2, vertIndex0, vertIndex1, vertIndex2, interpolatedWDepth);
				break;
			}
			case RenderMode::DepthBuffer:
			{
				float depthColor;
				RemapZDepth(interpolatedZDepth, depthColor);
				pixelInfo.color = { depthColor, depthColor, depthColor };
				break;
			}
			}

			Shade(pixelIdx, pixelInfo);
		}
	}
}

void dae::Renderer_Software::ClearBackground() const
{
	constexpr float color{ 0.39f * 255 };
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, color, color, color));
}

void dae::Renderer_Software::ResetDepthBuffer() const
{
	const int nrPixels{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
}

void dae::Renderer_Software::Shade(int pixelIndex,Vertex_Out pxlInfo) const
{
	Vector3 normal{ pxlInfo.normal };

	ColorRGB finalColor{};

	const Vector3 lightDirection{ Vector3(0.577f, -0.577f, 0.577f).Normalized() };
	constexpr float lightIntensity{ 7.0f };
	constexpr float specularShininess{ 25.0f };

	if (m_pNormalTexture != nullptr && m_IsNormalActive)
	{
		const Vector3 binormal = Vector3::Cross(pxlInfo.normal, pxlInfo.tangent);
		const Matrix tangentSpaceAxis = Matrix{ pxlInfo.tangent, binormal, pxlInfo.normal, Vector3::Zero };

		//Clamp uv between -1 and 1;
		const ColorRGB currentNormalMap{ 2.0f * m_pNormalTexture->Sample(pxlInfo.uv) - ColorRGB{ 1.0f, 1.0f, 1.0f } };
		
		const Vector3 normalMapSample{ currentNormalMap.r, currentNormalMap.g, currentNormalMap.b };
		normal = tangentSpaceAxis.TransformVector(normalMapSample);
	}

	switch (m_RenderMode)
	{
	case RenderMode::Normal:
	{
		const float observedArea{ std::max(Vector3::Dot(normal.Normalized(), -lightDirection.Normalized()), 0.0f) };
			
		switch (m_LightingMode)
		{
		case LightingMode::Combined:
		{

			if (!m_pGlossinessTexture || !m_pSpecularTexture)
			{
				assert(false);
			}

			// cd * (kd) / PI
			const ColorRGB lambert{ m_pTexture->Sample(pxlInfo.uv) / PI };

			const float phongExponent{ specularShininess * m_pGlossinessTexture->Sample(pxlInfo.uv).r };
			const ColorRGB specular{ m_pSpecularTexture->Sample(pxlInfo.uv) * CalculatePhong(phongExponent, -lightDirection, pxlInfo.viewDirection, normal) };

			finalColor += (lightIntensity * lambert + specular) * observedArea;
			break;
		}
		case LightingMode::ObservedArea:
		{
			finalColor += ColorRGB{ observedArea, observedArea, observedArea };
			break;
		}
		case LightingMode::Diffuse:
		{
			// cd * (kd) / PI
			const ColorRGB lambert{ m_pTexture->Sample(pxlInfo.uv) / PI };
			finalColor += ColorRGB(lightIntensity * observedArea * lambert);
			break;
		}
		case LightingMode::Specular:
		{
			const float phongExponent{ specularShininess * m_pGlossinessTexture->Sample(pxlInfo.uv).r };
			const ColorRGB specular{ m_pSpecularTexture->Sample(pxlInfo.uv) * CalculatePhong(phongExponent, -lightDirection, pxlInfo.viewDirection, normal) };
			finalColor += specular * observedArea;
			break;
		}
		}
		break;
	}
	case RenderMode::DepthBuffer:
	{
			//Depthbuffer is stored in pxlinfo.color
		finalColor += pxlInfo.color;
		break;
	}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void dae::Renderer_Software::InitializeBuffer(SDL_Window* pWindow)
{
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[m_Width * m_Height];
	ResetDepthBuffer();
}

void dae::Renderer_Software::InitializeMesh(const char* filename)
{
	bool isObjLoaded{ Utils::ParseOBJ(filename, m_Mesh.vertices, m_Mesh.indices) };
	assert(isObjLoaded);

	const Vector3 translation{ };
	const Vector3 rotation{ };
	const Vector3 scale{ Vector3{ 1.0f, 1.0f, 1.0f } };
	m_Mesh.worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(translation);
}

void dae::Renderer_Software::ResetState()
{
	m_Mesh.vertices_out.clear();
	ResetDepthBuffer();
	ClearBackground();
	SDL_LockSurface(m_pBackBuffer);
}

void dae::Renderer_Software::WorldToNDC(const Matrix& worldViewProjectionMatrix)
{
	m_Mesh.vertices_out.reserve(m_Mesh.vertices.size());
	for (const Vertex_Software& vertex : m_Mesh.vertices)
	{
		Vertex_Out vOut{ {}, vertex.color, vertex.uv, vertex.normal, vertex.tangent };

		//Transform
		vOut.position = worldViewProjectionMatrix.TransformPoint({ vertex.position, 1.0f });
		vOut.normal = m_Mesh.worldMatrix.TransformVector(vertex.normal);
		vOut.tangent = m_Mesh.worldMatrix.TransformVector(vertex.tangent);

		vOut.viewDirection = Vector3{ vOut.position.x, vOut.position.y, vOut.position.z };
		vOut.viewDirection.Normalize();

		// Divide positions by old z (stored in w)
		vOut.position.x /= vOut.position.w;
		vOut.position.y /= vOut.position.w;
		vOut.position.z /= vOut.position.w;

		m_Mesh.vertices_out.emplace_back(vOut);
	}
}

void dae::Renderer_Software::NDCToRaster(std::vector<Vector2>& rasterVertices) const
{
	rasterVertices.clear();
	rasterVertices.reserve(m_Mesh.vertices_out.size());

	for (const auto& ndcVertex : m_Mesh.vertices_out)
	{
		rasterVertices.emplace_back((ndcVertex.position.x + 1) / 2.0f * m_Width
			, (1.0f - ndcVertex.position.y) / 2.0f * m_Height);
	}
}

void dae::Renderer_Software::UpdateSDL() const
{
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

bool dae::Renderer_Software::IsVertexSame(uint32_t vertex0, uint32_t vertex1, uint32_t vertex2) const
{
	return vertex0 == vertex1 || vertex1 == vertex2 || vertex0 == vertex2;
}

bool dae::Renderer_Software::IsOutsideFrustum(uint32_t vertex0, uint32_t vertex1, uint32_t vertex2) const
{
	return m_pCamera->IsOutsideFrustum(m_Mesh.vertices_out[vertex0].position) ||
		m_pCamera->IsOutsideFrustum(m_Mesh.vertices_out[vertex1].position) ||
		m_pCamera->IsOutsideFrustum(m_Mesh.vertices_out[vertex2].position);

}

void dae::Renderer_Software::CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, int& startingX, int& StartingY, int& endingX, int& endingY) const
{
	// Calculate the bounding box of this triangle
	const float minx = std::min(v1.x, v2.x);
	const float minxx = std::min(v0.x, v2.x);
	const float minxtot = std::min(minx, minxx);

	const float miny = std::min(v1.y, v2.y);
	const float minyy = std::min(v0.y, v2.y);
	const float minytot = std::min(miny, minyy);


	const float maxx = std::max(v1.x, v2.x);
	const float maxxx= std::max(v0.x, v2.x);
	const float maxxtot = std::max(maxx, maxxx);

	const float maxy = std::max(v1.y, v2.y);
	const float maxyy = std::max(v0.y, v2.y);
	const float maxytot = std::max(maxy, maxyy);

	const Vector2 minBoundingBox{minxtot, minytot };
	const Vector2 maxBoundingBox{ maxxtot, maxytot };
	constexpr int margin{ 1 };

	startingX = { std::clamp(static_cast<int>(minBoundingBox.x - margin), 0, m_Width) };
	StartingY = { std::clamp(static_cast<int>(minBoundingBox.y - margin), 0, m_Height) };
	endingX = { std::clamp(static_cast<int>(maxBoundingBox.x + margin), 0, m_Width) };
	endingY = { std::clamp(static_cast<int>(maxBoundingBox.y + margin), 0, m_Height) };
}

void dae::Renderer_Software::RenderBoundingBox(const int pixelIndex) const
{
	m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(255),
		static_cast<uint8_t>(255),
		static_cast<uint8_t>(255));
}

bool dae::Renderer_Software::IsInsideTriangle(const float edgePoint01, const float edgePoint12, const float edgePoint20) const
{
	return edgePoint01 >= 0 && edgePoint12 >= 0 && edgePoint20 >= 0;
}

bool dae::Renderer_Software::IsCurrentDepthBufferLessThenDepth(const int pixelIndex, const float ZDepth) const
{
	return m_pDepthBufferPixels[pixelIndex] < ZDepth;
}

void dae::Renderer_Software::CalculatePixelInfo(Vertex_Out& pixelInfo, float weightV0, float weightV1,float weightV2, uint32_t vertIndex0, uint32_t vertIndex1, uint32_t vertIndex2, float wDepth) const
{
	pixelInfo.uv = Vector2{
		(weightV0 * m_Mesh.vertices_out[vertIndex0].uv / m_Mesh.vertices_out[vertIndex0].position.w +
		weightV1 * m_Mesh.vertices_out[vertIndex1].uv / m_Mesh.vertices_out[vertIndex1].position.w +
		weightV2 * m_Mesh.vertices_out[vertIndex2].uv / m_Mesh.vertices_out[vertIndex2].position.w)
		* wDepth};

	pixelInfo.normal = Vector3{
		(weightV0 * m_Mesh.vertices_out[vertIndex0].normal / m_Mesh.vertices_out[vertIndex0].position.w +
		weightV1 * m_Mesh.vertices_out[vertIndex1].normal / m_Mesh.vertices_out[vertIndex1].position.w +
		weightV2 * m_Mesh.vertices_out[vertIndex2].normal / m_Mesh.vertices_out[vertIndex2].position.w)
		* wDepth}.Normalized();

	pixelInfo.tangent = Vector3{
		(weightV0 * m_Mesh.vertices_out[vertIndex0].tangent / m_Mesh.vertices_out[vertIndex0].position.w +
		weightV1 * m_Mesh.vertices_out[vertIndex1].tangent / m_Mesh.vertices_out[vertIndex1].position.w +
		weightV2 * m_Mesh.vertices_out[vertIndex2].tangent / m_Mesh.vertices_out[vertIndex2].position.w)
		* wDepth}.Normalized();

	pixelInfo.viewDirection = Vector3{
		(weightV0 * m_Mesh.vertices_out[vertIndex0].viewDirection / m_Mesh.vertices_out[vertIndex0].position.w +
		weightV1 * m_Mesh.vertices_out[vertIndex1].viewDirection / m_Mesh.vertices_out[vertIndex1].position.w +
		weightV2 * m_Mesh.vertices_out[vertIndex2].viewDirection / m_Mesh.vertices_out[vertIndex2].position.w)
		* wDepth}.Normalized();
}

void dae::Renderer_Software::RemapZDepth(float interpolatedZDepth,float& depthColor) const
{
	constexpr float remapValue{ 0.985f };
	const float clamped{ std::clamp(interpolatedZDepth,remapValue , 1.0f) };
	depthColor = (clamped - remapValue) / (1.0f - remapValue);
}


ColorRGB dae::Renderer_Software::CalculatePhong(const float exponent, const Vector3& lightDirection, const Vector3& viewDirection, const Vector3& normal) const
{

	const Vector3 reflectedLightVector{ Vector3::Reflect(-lightDirection,normal) };
	const float reflectedViewDot{ std::max(Vector3::Dot(reflectedLightVector, viewDirection), 0.f) };
	const float phong{ 1 * powf(reflectedViewDot, exponent) };

	return ColorRGB{ phong, phong, phong };
}

bool Renderer_Software::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void dae::Renderer_Software::ToggleBounding()
{
	m_IsBoundingBoxShown = !m_IsBoundingBoxShown;

	if(m_IsBoundingBoxShown)
	{
		m_RenderMode = RenderMode::BoundingBox;
	}else
	{
		m_RenderMode = RenderMode::Normal;
	}
}

void dae::Renderer_Software::ToggleDepth()
{
	m_IsDepthShown = !m_IsDepthShown;

	if (m_IsDepthShown)
	{
		m_RenderMode = RenderMode::DepthBuffer;
	}
	else
	{
		m_RenderMode = RenderMode::Normal;
	}
}

void dae::Renderer_Software::ToggleLightingMode()
{
	//gets current Lighting mode as int
	int current = static_cast<int>(m_LightingMode);
	++current;

	//after increasing int convert it back to Lighting mode while limiting it to its boundaries
	current %= static_cast<int>(LightingMode::Last);

	//Set new Lighting mode as current Lighting mode
	m_LightingMode = static_cast<LightingMode>(current);
}

void dae::Renderer_Software::ToggleNormalMap()
{
	m_IsNormalActive = !m_IsNormalActive; 
}

void dae::Renderer_Software::ToggleMeshRotation()
{
	m_IsMeshRotating = !m_IsMeshRotating;
}
