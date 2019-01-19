#include "Camera.hpp"

using namespace glm;

mat4 Camera::getViewMatrix() const
{
    return lookAt(_position, _position + _front, _up);
}

void Camera::processKeyboard(const CameraMovement direction, const float deltaTime)
{
    const auto velocity = _movementSpeed * deltaTime;
    if (direction == Forward)
        _position += _front * velocity;
    if (direction == Backward)
        _position -= _front * velocity;
    if (direction == Left)
        _position -= _right * velocity;
    if (direction == Right)
        _position += _right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset, const GLboolean constrainPitch)
{
    xoffset *= _mouseSensitivty;
    yoffset *= _mouseSensitivty;

    _yaw += xoffset;
    _pitch += yoffset;

    if (constrainPitch) {
        _pitch = clamp(_pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(const float yoffset)
{
    console->debug("scroll event yoffset={}", yoffset);
    if (_zoom >= 1.0f && _zoom <= 45.0f)
        _zoom -= yoffset;
    if (_zoom <= 1.0f)
        _zoom = 1.0f;
    if (_zoom >= 45.0f)
        _zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    vec3 front;
    front.x = cos(radians(_yaw)) * cos(radians(_pitch));
    front.y = sin(radians(_pitch));
    front.z = sin(radians(_yaw)) * cos(radians(_pitch));
    _front = normalize(front);

    _right = normalize(cross(front, _worldUp));
    _up = normalize(cross(_right, front));
}


