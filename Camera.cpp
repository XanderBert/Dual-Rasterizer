#include "pch.h"
#include "Camera.h"

dae::Camera::Camera(const Vector3& origin, const float fovAngle, float width, float height) :
	m_Origin(origin),
	m_FOV(fovAngle* TO_RADIANS / 2.f),
	m_AspectRatio(width / height),
	m_InverseViewMatrix()
{
}

dae::Matrix dae::Camera::GetViewMatrix()
{
	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
	m_Up = Vector3::Cross(m_Forward, m_Right);

	const Matrix InvViewMatrix = Matrix
	{
		m_Right,
		m_Up,
		m_Forward,
		m_Origin
	};

	//SET THE INVERSE MATRIX
	m_InverseViewMatrix = InvViewMatrix;

	return Matrix::Inverse(InvViewMatrix);
	//return Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
}

dae::Matrix dae::Camera::GetProjectionMatrix(float zn, float zf) const
{
	return Matrix::CreatePerspectiveFovLH(m_FOV, m_AspectRatio, zn, zf);
}

dae::Matrix dae::Camera::GetInverseViewmatrix()
{
	return m_InverseViewMatrix;
}

bool dae::Camera::IsOutsideFrustum(const Vector4& vector) const
{
	return vector.x < -1.f || vector.x > 1.f
		|| vector.y < -1.f || vector.y > 1.f
		|| vector.z < -1.f || vector.z > 1.f;
}

void dae::Camera::Update(const Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	// Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	// Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	float keyboardSpeed{ 8.f };
	constexpr float mouseSpeed{ 0.7f };
	constexpr float angularSpeed{ 5.0f * TO_RADIANS };

	if (pKeyboardState[SDL_SCANCODE_LSHIFT]) keyboardSpeed *= 2;

	Vector3 direction{};
	direction += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_Z]) * m_Forward * keyboardSpeed * deltaTime;
	direction -= pKeyboardState[SDL_SCANCODE_S] * m_Forward * keyboardSpeed * deltaTime;
	direction -= (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A]) * m_Right * keyboardSpeed * deltaTime;
	direction += pKeyboardState[SDL_SCANCODE_D] * m_Right * keyboardSpeed * deltaTime;



	switch (mouseState)
	{
	case SDL_BUTTON_LMASK:
		direction -= m_Forward * (mouseY * mouseSpeed * deltaTime);
		m_TotalYaw += mouseX * angularSpeed * deltaTime;
		break;
	case SDL_BUTTON_RMASK:
		m_TotalYaw += mouseX * angularSpeed * deltaTime;
		m_TotalPitch -= mouseY * angularSpeed * deltaTime;
		break;
	case SDL_BUTTON_X2:
		direction.y -= mouseY * mouseSpeed * deltaTime;
		break;
	}
	m_TotalPitch = std::clamp(m_TotalPitch, -90.f * TO_RADIANS, 90.f * TO_RADIANS);


	m_Origin += direction;

	const Matrix rotationMatrix = Matrix::CreateRotationX(m_TotalPitch) * Matrix::CreateRotationY(m_TotalYaw);
	m_Forward = rotationMatrix.TransformVector(Vector3::UnitZ);
}