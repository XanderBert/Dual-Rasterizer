#include "pch.h"
#include "Camera.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer_Hardware.h"
#include "Renderer_Software.h"

using namespace dae;

void Togglebool(bool& isSomething)
{
	isSomething = !isSomething;
}

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - Berten Xander 2DAE GD 07",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto* m_pCamera = new Camera{ {0.f, 0.f, -50.f},45.f,static_cast<float>(width), static_cast<float>(height) };

	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer_Hardware(pWindow, m_pCamera);
	const auto pRendererSoftware = new Renderer_Software(pWindow, m_pCamera);
	bool isHardwareRendering{ false };
	bool isPrintingFPS{ false };

	//Techniques
	enum Techniques
	{
		linear,
		point,
		anisotropic
	};
	Techniques currentTechnique{ Techniques::point };
	LPCSTR currentTechniqueString{"DefaultTechniquePoint"};
	

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					Togglebool(isHardwareRendering);
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRendererSoftware->ToggleMeshRotation();
					pRenderer->ToggleMeshRotation();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					switch (currentTechnique)
					{
					case linear: currentTechnique = Techniques::point; currentTechniqueString = "DefaultTechniquePoint"; break;
					case point: currentTechnique = Techniques::anisotropic; currentTechniqueString = "DefaultTechniqueAnisotropic"; break;
					case anisotropic: currentTechnique = Techniques::linear; currentTechniqueString = "DefaultTechniqueLinear"; break;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					
					pRendererSoftware->ToggleLightingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pRendererSoftware->ToggleNormalMap();
					
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					pRendererSoftware->ToggleDepth();

				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					pRendererSoftware->ToggleBounding();

				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					Togglebool(isPrintingFPS);

				}
				break;
			default: ;
			}
		}

		//--------- Update ---------

			m_pCamera->Update(pTimer);
			pRenderer->Update(pTimer, currentTechniqueString);
			pRendererSoftware->Update(pTimer);

		
		

		//--------- Render ---------
		if(isHardwareRendering)
		{
			pRenderer->Render();
		}else
		{
			pRendererSoftware->Render();
		}
		
		

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f && isPrintingFPS)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete m_pCamera;
	delete pRenderer;
	delete pRendererSoftware;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}