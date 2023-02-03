#pragma once
#include "Effect.h"

namespace dae
{
	class Texture_Hardware;
	class EffectFire final : public dae::Effect
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile);
		~EffectFire() override; 
		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		EffectFire(const EffectFire& other) = delete;
		EffectFire(EffectFire&& other)noexcept = delete;
		EffectFire& operator=(const EffectFire& other) = delete;
		EffectFire& operator=(EffectFire&& other)noexcept = delete;

		//----------------------------------------
		// General Methods
		//----------------------------------------
	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------

	};
}