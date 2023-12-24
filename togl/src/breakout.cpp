#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "breakout.h"

static const int worldWidth = 640;
static const int worldHeight = 480;

//////////////// COLLISION /////////////////////////

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}

bool AABBCollisionDetection(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo)
{
	bool collisionX = (positionOne.x + dimensionOne.x >= positionTwo.x) && (positionOne.x <= positionTwo.x + dimensionTwo.x);
	bool collisionY = (positionOne.y + dimensionOne.y >= positionTwo.y) && (positionOne.y <= positionTwo.y + dimensionTwo.y);
	return collisionX && collisionY;
}

Collision CheckCollision(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo) // AABB - Circle collision
{
	Collision result = {};

	glm::vec2 Center = positionOne + 0.5f * dimensionOne; // get center point circle first 
	
	glm::vec2 aabbHalfExtents = 0.5f * dimensionTwo;    // calculate AABB info (center, half-extents)
	glm::vec2 aabbCenter = positionTwo + aabbHalfExtents;
	
	glm::vec2 Difference = Center - aabbCenter;  // get difference vector between both centers
	glm::vec2 Clamped = glm::clamp(Difference, -aabbHalfExtents, aabbHalfExtents);

	glm::vec2 Closest = aabbCenter + Clamped; // add clamped value to AABB_center and we get the value of box closest to circle
	
	Difference = Center - Closest; // retrieve vector between center circle and closest point AABB and check if length <= radius
	
	result.difference = Difference;
	result.aabbCollisionSide = VectorDirection(Difference);

	if (glm::length(Difference) <= glm::length(dimensionOne.x * 0.5f))
		result.hasCollided = true;
	else
		result.hasCollided = false;

	return result;
}

void UpdateBallOnCollision(glm::vec2& ballVelocity, glm::vec2& ballPosition, const glm::vec2& ballDimensions, Collision collision)
{
	if (collision.hasCollided)
	{
		Direction dir = collision.aabbCollisionSide;
		glm::vec2 diffVector = collision.difference;
		if (dir == LEFT || dir == RIGHT)
		{
			ballVelocity.x = -ballVelocity.x;
			float penetration = std::ceilf(glm::length(ballDimensions * 0.5f) - std::abs(diffVector.x));
			if (dir == LEFT)
				ballPosition.x -= penetration;
			else
				ballPosition.x += penetration;
		}
		else
		{
			float penetration = glm::length(ballDimensions * 0.5f) - std::abs(diffVector.y);
			ballVelocity.y = -ballVelocity.y;
			if (dir == DOWN)
				ballPosition.y -= penetration;
			else
				ballPosition.y += penetration;
		}
	}
}

void UpdatePlayerPosition(glm::vec2* playerPosition, InputState inputState, float deltaTime)
{
	glm::vec2 playerPositionDelta = { 0.0f, 0.0f };
	if (inputState.right)
		playerPositionDelta = glm::vec2(1.0f, 0.0f) * 0.5f * deltaTime;
	if (inputState.left)
		playerPositionDelta = glm::vec2(-1.0f, 0.0f) * 0.5f * deltaTime;

	*playerPosition += playerPositionDelta;
}

// /// ///              Level init       /////////// 
std::vector<glm::vec2> CreateBrickPositions(unsigned int BlockRows, unsigned int BlockCols /*, worldWidth, worldHeight */)
{
	std::vector<glm::vec2> brickPositions;

	float blockWidth = (float)worldWidth / (float)BlockCols;
	float blockHeight = worldHeight / ((float)BlockRows * 3.0f);

	for (unsigned int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (unsigned int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			float blockPositionX = colIdx * blockWidth;
			float blockPositionY =  worldHeight - (rowIdx + 1) * blockHeight;

			brickPositions.push_back({ blockPositionX, blockPositionY });
		}
	}
	return brickPositions;
}

const std::vector<glm::vec2> levelBricks = CreateBrickPositions(BlockRows, BlockCols);

GameState initGameState() {
	GameState result;

	Ball ball =
	{
		true,
		{ 1.0f, 1.0f },
		{ 15.0f, 15.0f },
		{ 1.0, 1.0, 1.0, 1.0 },
		{ 640 / 2.0f, 480 / 2.0f },
	};
	objectData player =
	{
		{ 0.0f , 0.0f },
		{ 128.0f / 1.5, 32.0f / 2 },
		{ 3.0, 0.0, 0.0, 1.0 },
		{ 640 / 2.0f , 480 * 1.0 / 10.0f },
	};
	objectData bricks = {};

	bool running = true;
	result.balls.push_back(ball);
	result.player = player;
	result.inputState = {};

	result.levels[0].levelData = gameLevel1;
	result.levels[0].brickCount = BlockRows * BlockCols;
	result.levels[1].levelData = gameLevel2;
	result.levels[1].brickCount = BlockRows * BlockCols;

	return result;
}

glm::vec2 BallPositionToPaddleCenter(glm::vec2 ballPosition, glm::vec2 ballDimension, glm::vec2 paddlePosition, glm::vec2 paddleDimension)
{
	glm::vec2 result;
	result.x = paddlePosition.x + 0.5f * paddleDimension.x - 0.5f * ballDimension.x;
	result.y = paddlePosition.y + paddleDimension.y;
	return result;
}

void SimulateGame(InputState& inputState, objectData& player, GameState& gameState,
	double deltaTime, std::vector<void*>& AudioQueue)
{
	///////////// UPDATE POSITIONS ///////////////////
	// player position 
	UpdatePlayerPosition(&player.position, inputState, (float)deltaTime);

	for (auto& ball : gameState.balls)
	{
		if (inputState.reset)
		{
			ball.ballOnPaddle = true;
			ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);
			ball.velocity = { 0.0f , 0.0f };
		}
		if (gameState.levels[gameState.currentLevel].brickCount == 0)
		{
			ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);
			ball.velocity = { 0.0f , 0.0f };
			++gameState.currentLevel;
		}
		if (ball.ballOnPaddle && inputState.space && !(inputState.spaceProcessed))
		{
			ball.ballOnPaddle = false;
			inputState.spaceProcessed = true;
			float ballSpeedScale = 0.3f;
			ball.velocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
		}
		// ball position
		if (ball.ballOnPaddle == true)
		{
 			ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);
		}

		else if (!inputState.reset)
		{
			ball.position += ball.velocity * (float)deltaTime;

			//////////////// PHYSICS RESOLUTION /////////////////
			// Ball-Brick collision detection
			float blockWidth = (float)worldWidth / (float)BlockCols;
			float blockHeight = worldHeight / ((float)BlockRows * 3.0f);
			for (int i = 0; i < levelBricks.size(); ++i)
			{
				if (*((int*)(gameState.levels[gameState.currentLevel].levelData) + i) != 0)
				{
					Collision collision = CheckCollision(ball.position, ball.dimension, levelBricks[i], { blockWidth, blockHeight });
					if (collision.hasCollided) // on collision
					{
						UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
						gameState.playerScore += gameState.levels[gameState.currentLevel].levelData[i] * 10;
						gameState.levels[gameState.currentLevel].levelData[i] = 0;
						AudioQueue.push_back(gameState.bleep);
						--gameState.levels[gameState.currentLevel].brickCount;
						if (gameState.levels[gameState.currentLevel].brickCount == 15)
						{
							Ball ball = {};
							ball.dimension = { 15.0, 15.0f };
							ball.color = { 1.0f, 1.0f, 1.0f, 1.0f };
							ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);
							ball.textureId = gameState.balls[0].textureId;
							gameState.balls.push_back(ball);
						}
					}
				}
			}

			// Ball-wall collision detection
			if (ball.position.x + ball.dimension.x > worldWidth)
			{
				ball.velocity.x = -ball.velocity.x;
				ball.position.x = worldWidth - ball.dimension.x;
			}
			else if (ball.position.y + ball.dimension.y > worldHeight)
			{
				ball.velocity.y = -ball.velocity.y;
				ball.position.y = worldHeight - ball.dimension.y;
			}
			else if (ball.position.x < 0)
			{
				ball.velocity.x = -ball.velocity.x;
				ball.position.x = 0;
			}
			else if (ball.position.y < 0)
			{
				ball.velocity = { 0.0f, 0.0f };
				ball.position.y = 0;
				gameState.playerLives -= 1;
			}

			// ball-paddle collision detection
			Collision collision = CheckCollision(ball.position, ball.dimension, player.position, player.dimension);
			UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
			if (collision.hasCollided)
			{
				AudioQueue.push_back(gameState.bleep);
				// this is essentially the command pattern, the command pattern states that you're yield a command, in this case the render queue implicitly describes the function to be called
				// the parameters then are the members of the command function essentially
				// this contrasts to the original version where I'd immediately call the function
				// separates when do do something with how to do it
				glm::vec2 difference = collision.difference;
				ball.velocity += 0.001 * glm::length(difference.x);
			}
		}
	}
}

void RenderGame(
	std::vector<QuadRenderData>& RenderQueue, std::vector<TextRenderData>& TextRenderQueue,
	std::vector<Ball> balls, objectData player, objectData bricks, int score, int lives,
	int* gameLevel)
{
	// Draw Balls
	for (auto& ball : balls)
	{
		RenderQueue.push_back({ ball.dimension, ball.position, ball.color, ball.textureId });
	}

	// Draw Player
	RenderQueue.push_back({ player.dimension, player.position, player.color, player.textureId });
	// Draw Blocks
	float blockWidth = (float)worldWidth / (float)BlockCols;
	float blockHeight = worldHeight / ((float)BlockRows * 3.0f);
	for (int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			// this is actually flyweight
			/*
			struct
			{
			glm::vec2 blockPos;
			glm::vec2 blockDim;
			int TileType;
			glm::vec4 ColorSelect;
			}
			*/
			glm::vec2 blockPos = levelBricks[rowIdx * BlockCols + colIdx];
			unsigned int TileType = gameLevel [rowIdx * BlockCols + colIdx] ;
			glm::vec4 ColorSelected = Colors[TileType];
			RenderQueue.push_back({ { blockWidth, blockHeight }, blockPos, ColorSelected, bricks.textureId });
		}
	}
	// really I would like a generalized renderer where I can pass an enum, so I can have a single queue
	TextRenderQueue.push_back({ "Lives: " + std::to_string(lives), {550, 50} });
	TextRenderQueue.push_back({ "Score: " + std::to_string(score), { 100, 50 } });
}



void ProcessInput(InputState& inputState, GameState& gameState)
{
	// pre input processing?
	if (inputState.pause && !(inputState.pauseProcessed))
	{
		switch (gameState.mode)
		{
		case GameMode::ACTIVE:
		{
			gameState.mode = GameMode::MENU;
			inputState.pauseProcessed = true;
			break;
		}
		case GameMode::MENU:
		{
			gameState.mode = GameMode::ACTIVE;
			inputState.pauseProcessed = true;
			break;
		}
		}
	}

	if (gameState.playerLives <= 0)
	{
		gameState.mode = GameMode::LOSE;
	}
}

void RenderMenu(InputState& inputState, std::vector<QuadRenderData>& RenderQueue,
	std::vector<TextRenderData>& TextRenderQueue, std::vector<void*>& AudioQueue, double deltaTime)
{
	static int selectedIndex = 0;
	glm::vec4 selectedColor = { 0.2, 0.7, 0.9, 1.0f };
	glm::vec4 unSelectedColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (inputState.down && !(inputState.downProcessed))
	{
		selectedIndex = glm::clamp(selectedIndex + 1, 0, 1);
		inputState.downProcessed = true;
	}
	else if (inputState.up && !(inputState.upProcessed))
	{
		selectedIndex = glm::clamp(selectedIndex - 1, 0, 1);
		inputState.upProcessed = true;
	}
	const int menuItems = 2;
	glm::vec4 menuColors[menuItems];
	for (int i = 0; i < menuItems; ++i)
	{
		if (i == selectedIndex)
			menuColors[i] = selectedColor;
		else
			menuColors[i] = unSelectedColor;
	}
	int yDim = worldHeight / 2;
	int xDim = worldWidth / 2;
	const int yDecrement = 50;
	// @todo remove opengl stuff
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TextRenderQueue.push_back({ "PAUSED" , { xDim, yDim } });
	yDim -= yDecrement;
	TextRenderQueue.push_back({ "Level Select: ", { xDim, yDim }, menuColors[0]});
	yDim -= yDecrement;
	TextRenderQueue.push_back({ "Quit", { xDim, yDim }, menuColors[1]});
}
void RenderGameOver(InputState& inputState, std::vector<QuadRenderData>& RenderQueue,
	std::vector<TextRenderData>& TextRenderQueue, std::vector<void*>& AudioQueue, double deltaTime)
{
	int yDim = worldHeight / 2;
	int xDim = worldWidth / 2;
	// @todo remove opengl stuff
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	TextRenderQueue.push_back({ "GAME OVER", { xDim, yDim } });
}
// do I peel off this layer?
void GameUpdateAndRender(GameState& gameState, InputState& inputState,
	std::vector<QuadRenderData>& RenderQueue, std::vector<TextRenderData>& TextRenderQueue,
	std::vector<void*>& AudioQueue, double deltaTime)
{	
	if (!gameState.initializedResources)
	{
		// 0 = GL_RGB, 1 = GL_RGBA;
		gameState.bricks.textureId = PlatformCreateTexture("res/textures/block.png", 0);
		for (auto& ball : gameState.balls)
		{
			ball.textureId = PlatformCreateTexture("res/textures/ball.png", 1);
		}
		gameState.player.textureId = PlatformCreateTexture("res/textures/paddle.png", 1);
		
		gameState.bleep = PlatformLoadWAV("res/audio/solid.wav");
		gameState.music = PlatformPlayMusic("res/audio/breakout.mp3");

		gameState.initializedResources = true;
	}
	ProcessInput(inputState, gameState);


	if (gameState.mode == GameMode::MENU)
	{
		RenderMenu(inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);
	}
	else if (gameState.mode == GameMode::LOSE)
	{
		RenderGameOver(inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);
	}
	else if (gameState.mode == GameMode::ACTIVE)
	{
		SimulateGame(inputState, gameState.player, gameState, deltaTime, AudioQueue);
		RenderGame(RenderQueue, TextRenderQueue,
			gameState.balls, gameState.player, gameState.bricks,
			gameState.playerScore, gameState.playerLives,
			gameState.levels[gameState.currentLevel].levelData);
	}
}