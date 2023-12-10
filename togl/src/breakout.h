#pragma once

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
	bool r;
};

struct QuadRenderData
{
	glm::vec2 pixelDimensions;
	glm::vec2 pixelPosition;
	glm::vec4 Color;
};

struct objectData
{
	glm::vec2 dimension;
	glm::vec4 color;
	glm::vec2 position;
	glm::vec2 velocity;
};

enum class GameState
{
	ACTIVE = 0, MENU, WIN
};

struct Game
{
	GameState gameState;
	InputState inputState;
	bool running = true;
};

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

struct GameData
{
	objectData ball;
	objectData player;
	int* gameLevel;
};