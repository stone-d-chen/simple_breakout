#pragma once
unsigned int PlatformCreateTexture(const char* filename, int pixelFormat);

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
	bool reset;
	bool pause;

	bool upProcessed;
	bool downProcessed;
	bool leftProcessed;
	bool rightProcessed;
	bool resetProcessed;
	bool pauseProcessed;
};

struct QuadRenderData {
	glm::vec2 pixelDimensions;
	glm::vec2 pixelPosition;
	glm::vec4 Color;
	unsigned int textureId;
};

struct TextRenderData { std::string text; glm::vec2 pixelPosition;/* top left  */ };

struct objectData {
	glm::vec2 dimension;
	glm::vec4 color;
	glm::vec2 position;
	glm::vec2 velocity;
	unsigned int textureId;
};

enum class GameMode { ACTIVE = 0, MENU, WIN };
struct GameState
{
	bool initializedResources = false;
	bool running = true;
	GameMode mode = GameMode::ACTIVE;

	objectData ball;
	objectData player;
	int* gameLevel;
	int playerScore = 0;
	int playerLives = 3;

	InputState inputState;
};

enum Direction { UP, RIGHT, DOWN, LEFT };
struct Collision
{
	bool hasCollided;
	Direction aabbCollisionSide;
	glm::vec2 difference;
};

void RenderGame(std::vector<QuadRenderData>& RenderQueue, objectData& ball, objectData& player, unsigned int windowWidth, unsigned int windowHeight);

void ProcessInput(InputState& inputState, GameState& gameState);

void SimulateGame(InputState& inputState, objectData& ball, objectData& player, double deltaTime, unsigned int windowWidth, unsigned int windowHeight, uint32_t* AudioQueue);
