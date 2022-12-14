#pragma once
namespace dae
{
	class Camera final
	{
	public:
		//----------------------------------------
		// Constructor(s) and Destructor
		//----------------------------------------
		Camera(const Vector3& origin, float fovAngle, float width, float height);
		~Camera();
		//----------------------------------------
		//  Disabling copy/move constructor and assignment operator
		//----------------------------------------
		Camera(const Camera& other) = delete;
		Camera(Camera&& other)noexcept = delete;
		Camera& operator=(const Camera& other) = delete;
		Camera& operator=(Camera&& other)noexcept = delete;
		//----------------------------------------
		// General Methods
		//----------------------------------------
		[[nodiscard]] Matrix GetViewMatrix() const;
		[[nodiscard]] Matrix GetProjectionMatrix(float zn, float zf) const;
		
	private:
		//----------------------------------------
		// Datamembers
		//----------------------------------------
		Vector3 m_Origin{};
		float m_FOV{};
		float m_AspectRatio{};

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };
	};
}