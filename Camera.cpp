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