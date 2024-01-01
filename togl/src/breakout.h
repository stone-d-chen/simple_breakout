#pragma once
unsigned int PlatformCreateTexture(const char* filename, int pixelFormat);
void* PlatformLoadWAV(const char* filename);
void* PlatformPlayMusic(const char* filename);
void* PlatformLoadFont(const char* filename, unsigned int pts);
void PlatformClear();


// forward declare and platform defines the rest later


glm::vec4 Colors[] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.3f, 0.4f, 0.5f, 1.0f },
	{ 0.0f, 0.4f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 0.5f, 1.0f },
	{ 0.3f, 0.0f, 0.5f, 1.0f },
};

int gameLevel1[] =
{
  1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1,
  2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2,
  3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1,
  1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1,
  2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2,
  3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1, 3, 1, 4, 2, 1, 1,
};

int gameLevel2[] =
{
	1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3,
};

struct gameLevel
{
	int rows;
	int cols;
	int brickCount;
	int* levelData;
};

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
	bool reset;
	bool pause;
	bool space;

	bool upProcessed;
	bool downProcessed;
	bool leftProcessed;
	bool rightProcessed;
	bool resetProcessed;
	bool pauseProcessed;
	bool spaceProcessed;
};

struct QuadRenderData {
	glm::vec2 pixelDimensions;
	glm::vec2 pixelPosition;
	float rotation;
	glm::vec4 Color;
	unsigned int textureId;
};

struct TextRenderData {
	void* font;
	std::string text;
	glm::vec2 pixelPosition;
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct objectData {
	glm::vec2 velocity;
	glm::vec2 dimension;
	glm::vec4 color;
	glm::vec2 position;
	unsigned int textureId;
};

struct Ball
{
	bool onPaddle = true;
	bool followsPaddle = true;
	float passThrough = 0; // in ms
	bool sticky = false;
	glm::vec2 velocity;
	glm::vec2 dimension;
	float rotation = 0;
	float rotVel = 0;
	glm::vec4 color;
	glm::vec2 position;
	unsigned int textureId;
};
enum class PowerUpType
{
	PASSTHROUGH,
	SPEED,
	INCREASE,
	STICKY,
	POWERUP_COUNT
};

struct PowerUp
{
	PowerUpType type;
	glm::vec2 velocity;
	glm::vec2 dimension;
	glm::vec4 color;
	glm::vec2 position;
	unsigned int textureId;
};


enum class GameMode { ACTIVE = 0, MENU, WIN, LOSE };

// struct GameAssets
// {
// 	unsigned int speedTextureId;
// 	unsigned int passthroughTextureId;
// 	unsigned int stickyTextureId;
// 	unsigned int increaseTextureId;
// 	void* bleep;
// 	void* powerSound;
// 	void* music;
// };

struct GameState
{
	bool initializedResources = false;
	bool running = true;
	GameMode mode = GameMode::ACTIVE;

	std::vector<Ball> balls;
	objectData player;
	objectData bricks;
	std::vector<PowerUp> powerUps;
	int currentLevel = 0;

	// GameAssets assets;

	unsigned int speedTextureId;
	unsigned int passthroughTextureId;
	unsigned int stickyTextureId;
	unsigned int increaseTextureId;

	void* bleep;
	void* powerSound;
	void* music;

	void* font;

	gameLevel levels[2];

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
