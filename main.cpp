#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

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
		"DirectX - Berten Xander 2DAE GD 07",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

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
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					//Change Technique
					switch (currentTechnique)
					{
					case linear: currentTechnique = Techniques::point; currentTechniqueString = "DefaultTechniquePoint"; break;
					case point: currentTechnique = Techniques::anisotropic; currentTechniqueString = "DefaultTechniqueAnisotropic"; break;
					case anisotropic: currentTechnique = Techniques::linear; currentTechniqueString = "DefaultTechniqueLinear"; break;
					}
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer, currentTechniqueString);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}