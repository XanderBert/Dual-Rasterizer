#include "pch.h"
#include "Camera.h"

dae::Camera::Camera(const Vector3& origin, const float fovAngle, float width, float height):
m_Origin(origin),
m_FOV(fovAngle * TO_RADIANS / 2.f),
m_AspectRatio(width / height)
{
}

dae::Matrix dae::Camera::GetViewMatrix() const
{
	return Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
}

dae::Matrix dae::Camera::GetProjectionMatrix(float zn, float zf) const
{
	return Matrix::CreatePerspectiveFovLH(m_FOV, m_AspectRatio, zn, zf);
}

void dae::Camera::Update(const Timer* pTimer)
{
	const float deltaTime{ pTimer->GetElapsed() };

	// Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	// Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	// Speed and limit constants
	constexpr float keyboardMovementSpeed{ 10.0f };
	constexpr float mouseMovementSpeed{ 2.0f };
	constexpr float angularSpeed{ 50.0f * TO_RADIANS };

	// The total movement of this frame
	Vector3 direction{};

	// Calculate new position with keyboard inputs
	direction += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_Z]) * m_Forward * keyboardMovementSpeed * deltaTime;
	direction -= pKeyboardState[SDL_SCANCODE_S] * m_Forward * keyboardMovementSpeed * deltaTime;
	direction -= (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A]) * m_Right * keyboardMovementSpeed * deltaTime;
	direction += pKeyboardState[SDL_SCANCODE_D] * m_Right * keyboardMovementSpeed * deltaTime;

	// Calculate new position and rotation with mouse inputs
	switch (mouseState)
	{
	case SDL_BUTTON_LMASK: // LEFT CLICK
		direction -= m_Forward * (mouseY * mouseMovementSpeed * deltaTime);
		m_TotalYaw += mouseX * angularSpeed * deltaTime;
		break;
	case SDL_BUTTON_RMASK: // RIGHT CLICK
		m_TotalYaw += mouseX * angularSpeed * deltaTime;
		m_TotalPitch -= mouseY * angularSpeed * deltaTime;
		break;
	case SDL_BUTTON_X2: // BOTH CLICK
		direction.y -= mouseY * mouseMovementSpeed * deltaTime;
		break;
	}
	m_TotalPitch = std::clamp(m_TotalPitch, -89.f * TO_RADIANS, 89.f * TO_RADIANS);


	//Movement calculations
	constexpr float shiftSpeed{ 4.0f };
	direction *= 1.0f + pKeyboardState[SDL_SCANCODE_LSHIFT] * (shiftSpeed - 1.0f);

	m_Origin += direction;

	// Calculate the rotation matrix with the new pitch and yaw
	const Matrix rotationMatrix = Matrix::CreateRotationX(m_TotalPitch) * Matrix::CreateRotationY(m_TotalYaw);

	// Calculate the new forward vector with the new pitch and yaw
	m_Forward = rotationMatrix.TransformVector(Vector3::UnitZ);

}