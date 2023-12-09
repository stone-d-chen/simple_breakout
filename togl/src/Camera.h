#pragma once
#include <glm.hpp>

struct Camera
{
	glm::vec3 position;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 lookAt;
};


glm::vec3 CalculatePositionChange(Camera camera, InputState state, float DeltaTime, float velocity)
{
	//float velocity = 0.0025;
	glm::vec3 positionDelta = {};
	if (state.up)
	{
		 positionDelta += camera.lookAt * DeltaTime * velocity;
	}
	if (state.down)
	{
		positionDelta -= camera.lookAt * DeltaTime * velocity;
	}
	if (state.right)
	{
		positionDelta += camera.right * DeltaTime * velocity;
	}
	if (state.left)
	{
		positionDelta -= camera.right * DeltaTime * velocity;
	}
	return positionDelta;
}

void UpdatePosition(Camera& camera, glm::vec3 position)
{
	camera.position += position;
}