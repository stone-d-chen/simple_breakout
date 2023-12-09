#pragma once

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
};
struct objectData
{
	glm::vec2 position;
	glm::vec2 dimension;
	glm::vec4 color;
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

