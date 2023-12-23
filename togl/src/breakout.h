#pragma once
unsigned int PlatformCreateTexture(const char* filename, int pixelFormat);
void* PlatformLoadWAV(const char* filename);
void* PlatformPlayMusic(const char* filename);

const int worldWidth = 640; 
const int worldHeight = 480;

glm::vec4 Colors[] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.3f, 0.4f, 0.5f, 1.0f },
	{ 0.0f, 0.4f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 0.5f, 1.0f },
	{ 0.3f, 0.0f, 0.5f, 1.0f },
};

const int BlockRows = 3;
const int BlockCols = 6;

int gameLevel[] =
{
  0, 1, 2, 3, 4, 1,
  2, 1, 0, 0, 1, 2,
  3, 0, 4, 2, 1, 0,
};


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
	objectData bricks;

	void* bleep;
	void* music;

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

void RenderGame(std::vector<QuadRenderData>& RenderQueue,
	objectData& ball, objectData& player,
	unsigned int windowWidth, unsigned int windowHeight);

void ProcessInput(InputState& inputState, GameState& gameState);

void SimulateGame(InputState& inputState,
	objectData& ball, objectData& player,
	double deltaTime,
	unsigned int windowWidth, unsigned int windowHeight, std::vector<void*> AudioQueue);
