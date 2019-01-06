#pragma once

#include "Logger.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
	Forward, 
	Backward,
	Left,
	Right
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera final : Logger
{
	// Camera Attributes
	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _up{};
	glm::vec3 _right{};
	glm::vec3 _worldUp;
	// Euler Angles
	float _yaw;
	float _pitch;
	// Camera Options
	float _movementSpeed;
	float _mouseSensitivty;
	float _zoom;
public:

	Camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
	       const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
	       const float yaw = YAW,
	       const float pitch = PITCH) 
			: Logger("camera")
	        , _position(position)
	        , _front(glm::vec3(0.0f, 0.0f, -1.0f))
			, _worldUp(up)
			, _yaw(yaw)
			, _pitch(pitch)
		    , _movementSpeed(SPEED)
			, _mouseSensitivty(SENSITIVITY)
			, _zoom(ZOOM)
	{
		updateCameraVectors();
	}

	Camera(const float posX, const float posY, const float posZ,
	       const float upX, const float upY, const float upZ, 
		   const float yaw, const float pitch) 
			: Logger("camera")
		    , _position(glm::vec3(posX, posY, posZ))
		    , _front(glm::vec3(0.0f, 0.0f, -1.0f))
	        , _worldUp(glm::vec3(upX, upY, upZ))
	        , _yaw(yaw)
	        , _pitch(pitch)
	        , _movementSpeed(SPEED)
		    , _mouseSensitivty(SENSITIVITY)
	        , _zoom(ZOOM)
	{
		updateCameraVectors();
	}

	glm::mat4 getViewMatrix() const;
	void processKeyboard(CameraMovement direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void processMouseScroll(float yoffset);

	float zoom() const
	{
		return _zoom;
	}

	const glm::vec3& position() const
	{
		return _position;
	}

	const glm::vec3& front() const
	{
		return _front;
	}

private:
	void updateCameraVectors();
};