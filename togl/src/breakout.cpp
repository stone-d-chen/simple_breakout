#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "breakout.h"

#include <cstdlib>

#include <algorithm>

static const int worldWidth = 960;
static const int worldHeight = 720;

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

Collision AABBCollisionDetection(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo)
{
	bool collisionX = (positionOne.x + dimensionOne.x >= positionTwo.x) && (positionOne.x <= positionTwo.x + dimensionTwo.x);
	bool collisionY = (positionOne.y + dimensionOne.y >= positionTwo.y) && (positionOne.y <= positionTwo.y + dimensionTwo.y);
	Collision collision = {};
	collision.hasCollided = collisionX && collisionY;
	return collision;
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

 void UpdatePlayerPositionn(glm::vec2* playerPosition, glm::vec2 playerDimension, InputState inputState, float deltaTime)
{
	glm::vec2 playerPositionDelta = { 0.0f, 0.0f };
	if (inputState.right)
	{
		playerPositionDelta = glm::vec2(1.0f, 0.0f) * 0.5f * deltaTime;
		if (playerPositionDelta.x + playerPosition->x + playerDimension.x > (float)worldWidth)
		{
			playerPosition->x = (float)worldWidth - playerDimension.x;
		}
		else
		{
			*playerPosition += playerPositionDelta;
		}
	}
	if (inputState.left)
	{
		playerPositionDelta = glm::vec2(-1.0f, 0.0f) * 0.5f * deltaTime;
		printf("playerposition x: %f\r", playerPosition->x);
		if ((playerPosition->x + playerPositionDelta.x) <= 0.0f)
		{
			playerPosition->x = 0.0f;
		}
		else
		{
			*playerPosition += playerPositionDelta;
		}
	}
}
  
glm::vec2 UpdatePlayerPosition(glm::vec2 playerPosition, glm::vec2 playerDimension, InputState inputState, float deltaTime)
{
	glm::vec2 playerPositionDelta = { 0.0f, 0.0f };
	float playerSpeed = 0.7f;
	if (inputState.right)
	{
		playerPositionDelta = glm::vec2(1.0f, 0.0f) * playerSpeed * deltaTime;
		if(playerPositionDelta.x + playerPosition.x + playerDimension.x > (float)worldWidth)
			playerPositionDelta.x = (float)worldWidth - playerPosition.x - playerDimension.x;
	}
	if (inputState.left)
	{
		playerPositionDelta = glm::vec2(-1.0f, 0.0f) * playerSpeed * deltaTime;
		if (playerPosition.x + playerPositionDelta.x <= 0.0f)
			playerPositionDelta.x = 0.0f - playerPosition.x;
	}
	return playerPositionDelta;
}

// /// ///              Level init       /////////// 

const int BlockRows = 6;
const int BlockCols = 24;

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

Ball initBall()
{
	Ball ball = {};
	ball.followsPaddle = true;
	ball.passThrough = false;
	ball.velocity = { 0.0f, 0.0f };
	ball.position = { 0.0f, 0.0f };
	ball.rotation = 0;
	ball.rotVel = 0;
	ball.dimension = { 15.0f, 15.0f };
	ball.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	ball.textureId = -1;
	return ball;
}

glm::vec2 BallPositionToPaddleCenter(glm::vec2 ballPosition, glm::vec2 ballDimension, glm::vec2 paddlePosition, glm::vec2 paddleDimension)
{
	glm::vec2 result;
	result.x = paddlePosition.x + 0.5f * paddleDimension.x - 0.5f * ballDimension.x;
	result.y = paddlePosition.y + paddleDimension.y;
	return result;
}


GameState initGameState() {
	GameState result;

	objectData player =
	{
		{ 0.0f , 0.0f },
		{ 128.0f / 1.5, 32.0f / 2 },
		{ 3.0, 0.0, 0.0, 1.0 },
		{ 640 / 2.0f , 480 * 1.0 / 10.0f },
	};
	Ball ball = initBall();
	ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);

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

PowerUp CreateRandomPowerUp(glm::vec2 position, GameState gameState)
{
	PowerUp powerup;
	powerup.position = position;
	powerup.dimension = { 30.0f, 30.0f };
	powerup.velocity = { 0.0f, -0.1f };
	float rand = std::rand() / (float)RAND_MAX;
	if (rand < 0.4)
	{
		powerup.type = PowerUpType::PASSTHROUGH;
		powerup.color = { 0.5, 1.0f, 0.5, 1.0f };
		powerup.textureId = gameState.passthroughTextureId;
	}
	else if (rand < 0.6)
	{
		powerup.type = PowerUpType::SPEED;
		powerup.color = { 0.5f, 0.0f, 0.5, 1.0f };
		powerup.textureId = gameState.speedTextureId;

	}
	else if (rand < 0.8)
	{
		powerup.type = PowerUpType::STICKY;
		powerup.color = { 0.5, 1.0f, 0.5, 1.0f };
		powerup.textureId = gameState.stickyTextureId;
	}
	else if (rand < 1.0)
	{
		powerup.type = PowerUpType::INCREASE;
		powerup.color = { 0.7, 6.0f, 0.3, 1.0f };
		powerup.textureId = gameState.increaseTextureId;
	}
	return powerup;
}

void SimulateGame(InputState& inputState, GameState& gameState,
	double deltaTime, std::vector<void*>& AudioQueue)
{
	///////////// UPDATE POSITIONS ///////////////////

	auto& player = gameState.player;
	
	/// Player positions 
	glm::vec2 playerPositionDelta = UpdatePlayerPosition(player.position, player.dimension, inputState, (float)deltaTime);
	player.position += playerPositionDelta;

	/// Ball
	for (auto& ball : gameState.balls)
	{
		if (inputState.reset)
		{
			ball.onPaddle = true;
			ball.followsPaddle = true;
			ball.position = BallPositionToPaddleCenter(ball.position, ball.dimension, player.position, player.dimension);
			ball.velocity = { 0.0f , 0.0f };
			ball.rotVel = 0;
		}
		if (ball.onPaddle && inputState.space && !(inputState.spaceProcessed)) // need to know if it's on the paddle to release; 
		{
			ball.onPaddle = false;
			ball.followsPaddle = false;
			ball.sticky = false;
			float ballSpeedScale = 0.3f;
			ball.velocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
			ball.rotVel = 1;

			inputState.spaceProcessed = true;
		}
		else if (ball.followsPaddle == true || ball.onPaddle)
		{
			ball.position += playerPositionDelta;
		}

		/// resolve powerups
		if (ball.passThrough > 0)
		{
			ball.passThrough -= (float)deltaTime;
		}

		/// Motion and collision update
		if (!inputState.reset && !ball.followsPaddle && !ball.onPaddle) // I removed the else if and introduced a bug which is very very annoying
		{
			ball.position += ball.velocity * (float)deltaTime;
			ball.rotation += ball.rotVel * (float)deltaTime;

			//////////////// PHYSICS RESOLUTION /////////////////
			// Ball-Brick collision detection
			for (int i = 0; i < levelBricks.size(); ++i)
			{
				if (*((int*)(gameState.levels[gameState.currentLevel].levelData) + i) != 0)
				{
					const float blockWidth = (float)worldWidth / (float)BlockCols;
					const float blockHeight = worldHeight / ((float)BlockRows * 3.0f);
					Collision collision = CheckCollision(ball.position, ball.dimension, levelBricks[i], { blockWidth, blockHeight });
					if (collision.hasCollided) // on collision
					{
						if (ball.passThrough <= 0)
						{
							UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
						}
						gameState.playerScore += gameState.levels[gameState.currentLevel].levelData[i] * 10;
						//gameState.playerScore += 10;
						gameState.levels[gameState.currentLevel].levelData[i] = 0;
						AudioQueue.push_back(gameState.bleep);
						--gameState.levels[gameState.currentLevel].brickCount;

						if ((std::rand() / (float) RAND_MAX) < 0.25)
						{
							PowerUp powerup = CreateRandomPowerUp(levelBricks[i], gameState);
							gameState.powerUps.push_back(powerup);
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
				ball.rotVel = 0;
				ball.position.y = 0;
				gameState.playerLives -= 1;
			}

			// ball-paddle collision detection
			Collision collision = CheckCollision(ball.position, ball.dimension, player.position, player.dimension);
			if (collision.hasCollided)
			{
				if (ball.sticky)
				{
					ball.onPaddle = true;
					ball.followsPaddle = true;
					ball.sticky = false; // turn off sticky
					ball.position.y = player.position.y + player.dimension.y;
					ball.velocity = { 0.0f, 0.0f };
					ball.rotVel = 0;
				}
				else
				{
					AudioQueue.push_back(gameState.bleep);
					UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
					// this is essentially the command pattern, the command pattern states that you're yield a command, in this case the render queue implicitly describes the function to be called
					// the parameters then are the members of the command function essentially
					// this contrasts to the original version where I'd immediately call the function
					// separates when do do something with how to do it
					glm::vec2 difference = collision.difference;
					ball.velocity += 0.001 * difference.x;
				}
			}
		}
	}
	
	for (int i = 0; i < gameState.powerUps.size(); i++)
	{
		auto& powerup = gameState.powerUps[i];
		powerup.position += powerup.velocity * (float)deltaTime;
		Collision collision = AABBCollisionDetection(player.position, player.dimension, powerup.position, powerup.dimension);
		if (collision.hasCollided)
		{
			powerup.position.y = 0; // for erasing?
			AudioQueue.push_back(gameState.powerSound);
			if (powerup.type == PowerUpType::PASSTHROUGH)
			{
				for (auto& ball : gameState.balls)
					ball.passThrough = 3000;
			}
			if (powerup.type == PowerUpType::SPEED)
			{
				for (auto& ball : gameState.balls)
					ball.velocity *= glm::vec2(1.1f, 1.1f);
			}
			if (powerup.type == PowerUpType::STICKY)
			{
				for (auto& ball : gameState.balls)
					ball.sticky = true;
			}
			if (powerup.type == PowerUpType::INCREASE)
			{

			}
		}
		if (powerup.position.y <= 0)
			gameState.powerUps.erase(gameState.powerUps.begin() + i);
	}
}

void RenderGame(
	std::vector<QuadRenderData>& RenderQueue,
	std::vector<TextRenderData>& TextRenderQueue,
	GameState gameState)
{
	auto balls = gameState.balls;
	auto player = gameState.player;
	auto bricks = gameState.bricks;
	auto score = gameState.playerScore;
	auto lives = gameState.playerLives;
	auto gameLevel = gameState.levels[gameState.currentLevel];

	// Draw Balls
	for (auto& ball : balls)
		RenderQueue.push_back({ ball.dimension, ball.position, ball.rotation, ball.color, ball.textureId });

	// Draw Player
	RenderQueue.push_back({ player.dimension, player.position, 0, player.color, player.textureId });
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
			unsigned int TileType = gameLevel.levelData[rowIdx * BlockCols + colIdx];
			if (TileType != 0)
			{
				glm::vec2 blockPos = levelBricks[rowIdx * BlockCols + colIdx];
				glm::vec4 ColorSelected = Colors[TileType];
				RenderQueue.push_back({ { blockWidth, blockHeight }, blockPos, 0, ColorSelected, bricks.textureId });
			}
		}
	}
	// really I would like a generalized renderer where I can pass an enum, so I can have a single queue
	TextRenderQueue.push_back({ gameState.font, "Lives: " + std::to_string(lives), {550, 50} });
	TextRenderQueue.push_back({ gameState.font, "Score: " + std::to_string(score), { 100, 50 } });
	TextRenderQueue.push_back({ gameState.font, "Time: " + std::to_string(balls[0].passThrough/1000), {325,50} });
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
		gameState.mode = GameMode::LOSE;

	if (gameState.levels[gameState.currentLevel].brickCount == 0)
	{
		if (gameState.currentLevel < 2)
		{
			++gameState.currentLevel;
			Ball newBall = initBall();
			newBall.textureId = gameState.balls[0].textureId;
			newBall.position = BallPositionToPaddleCenter(newBall.position, newBall.dimension, gameState.player.position, gameState.player.dimension);
			gameState.balls.clear();
			gameState.balls.push_back(newBall);
		}
		else
			gameState.mode = GameMode::WIN;
		
		gameState.powerUps.clear();
	}
}

void RenderMenu(InputState& inputState, std::vector<QuadRenderData>& RenderQueue,
	std::vector<TextRenderData>& TextRenderQueue, std::vector<void*>& AudioQueue, GameState& gameState, double deltaTime)
{
	static int selectedIndex = 0; // hmm is this a good way to go?
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

	if (selectedIndex == 1 && inputState.space)
	{
		gameState.running = false;
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
	PlatformClear();
	TextRenderQueue.push_back({ gameState.font, "PAUSED" , { xDim, yDim } });
	yDim -= yDecrement;
	TextRenderQueue.push_back({ gameState.font, "Level Select: " + std::to_string(gameState.currentLevel), {xDim, yDim}, menuColors[0]});
	yDim -= yDecrement;
	TextRenderQueue.push_back({ gameState.font, "Quit", { xDim, yDim }, menuColors[1]});
}
void RenderGameOver(InputState& inputState, std::vector<QuadRenderData>& RenderQueue,
	std::vector<TextRenderData>& TextRenderQueue, std::vector<void*>& AudioQueue, GameState gameState ,double deltaTime)
{
	int yDim = worldHeight / 2;
	int xDim = worldWidth / 2;
	PlatformClear();
	TextRenderQueue.push_back({ gameState.font, "GAME OVER", { xDim, yDim } });
}

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

		gameState.passthroughTextureId = PlatformCreateTexture("res/textures/powerup_passthrough.png", 1);
		gameState.speedTextureId = PlatformCreateTexture("res/textures/powerup_speed.png", 1);
		gameState.increaseTextureId = PlatformCreateTexture("res/textures/powerup_increase.png", 1);
		gameState.stickyTextureId = PlatformCreateTexture("res/textures/powerup_sticky.png", 1);

		// basic shader
		// font
		gameState.font = PlatformLoadFont("res/fonts/Urbanist-Medium.ttf", 64);
		gameState.bleep = PlatformLoadWAV("res/audio/solid.wav");
		gameState.powerSound = PlatformLoadWAV("res/audio/bleep.wav");
		gameState.music = PlatformPlayMusic("res/audio/breakout.mp3");

		gameState.initializedResources = true;
	}
	ProcessInput(inputState, gameState);

	if (gameState.mode == GameMode::MENU)
	{
		RenderMenu(inputState, RenderQueue, TextRenderQueue, AudioQueue, gameState, deltaTime);
	}
	else if (gameState.mode == GameMode::LOSE)
	{
		RenderGameOver(inputState, RenderQueue, TextRenderQueue, AudioQueue, gameState, deltaTime);
	}
	else if (gameState.mode == GameMode::ACTIVE)
	{
		SimulateGame(inputState, gameState, deltaTime, AudioQueue);
		RenderGame(RenderQueue, TextRenderQueue, gameState);
	}
}