#include "pch.h"
#include "Texture_Software.h"
#include <algorithm>
#include <cassert>

#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture_Software::Texture_Software(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture_Software::~Texture_Software()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture_Software* Texture_Software::LoadFromFile(const std::string& path)
	{
		//create an SDL_Surface from the file
		SDL_Surface* file = IMG_Load(path.c_str());

		//return nothing when there is no file
		if (!file)
		{
			assert(false && "File is not found");
			return nullptr;
		}

		//create and return a texture from the file
		return new Texture_Software(file);
	}

	ColorRGB Texture_Software::Sample(const Vector2& uv) const
	{
		Uint8 r{};
		Uint8 g{};
		Uint8 b{};

		const float xCord = std::clamp(uv.x, 0.0f, 1.0f) * m_pSurface->w;
		const float yCord = std::clamp(uv.y, 0.0f, 1.0f) * m_pSurface->h;

		const Uint32 pixelIndex{ m_pSurfacePixels[static_cast<int>(xCord) + static_cast<int>(yCord) * m_pSurface->w] };

		SDL_GetRGB(pixelIndex, m_pSurface->format, &r, &g, &b);

		return ColorRGB{ r / 255.f, g / 255.f, b / 255.f };
	}
}