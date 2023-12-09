#pragma once

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
};

// Players Levels and Tiles
glm::vec2 playerPosition = { 640 / 2.0f , 480 * 1.0/ 10.0f };
glm::vec2 playerDimensions = { 100.0f, 20.0f };
glm::vec4 playerColor = { 1.0, 0.0, 0.0, 1.0 };