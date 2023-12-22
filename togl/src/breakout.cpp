#include "breakout.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

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

	if (glm::length(Difference) <= glm::length(dimensionOne * 0.5f))
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

std::vector<glm::vec2> CreateBrickPositions(unsigned int BlockRows, unsigned int BlockCols /*, windowWidth, windowHeight */)
{
	unsigned int windowWidth = 640, windowHeight = 480;

	std::vector<glm::vec2> brickPositions;

	float blockWidth = (float)windowWidth / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);

	for (unsigned int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (unsigned int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			float blockPositionX = colIdx * blockWidth;
			float blockPositionY = windowHeight - (rowIdx + 1) * blockHeight;

			brickPositions.push_back({ blockPositionX, blockPositionY });
		}
	}
	return brickPositions;
}

const std::vector<glm::vec2> levelBricks = CreateBrickPositions(BlockRows, BlockCols);

int gameLevel[] =
{
  0, 1, 2, 3, 4, 1,
  2, 1, 0, 0, 1, 2,
  3, 0, 4, 2, 1, 0,
};

GameState initGameState() {
	GameState result;

	float ballSpeedScale = 0.3f;
	objectData ball =
	{
		{ 20.0f, 20.0f },
		{ 0.0, 0.7, 0.0, 1.0 },
		{ 640 / 2.0f, 480 / 2.0f },
		{ 1.0f * ballSpeedScale, 1.0f * ballSpeedScale },
	};
	objectData player =
	{
		{ 100.0f, 20.0f },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 640 / 2.0f , 480 * 1.0 / 10.0f },
		{},
	};

	bool running = true;
	result.ball = ball;
	result.player = player;
	result.gameLevel = gameLevel;
	result.playerScore = 0;
	result.playerLives = 3;
	result.inputState = {};

	return result;
}


void ProcessInput(InputState& inputState, GameState& gameState)
{
	// pre input processing?
	if (inputState.pause && !(inputState.pauseProcessed)	)
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
}

void SimulateGame(InputState& inputState, objectData& ball, objectData& player, GameState& gameState,
	double deltaTime, uint32_t* AudioQueue)
{
	unsigned int windowWidth = 640, windowHeight = 480; // @TODO: some hardcoded window stuff
	///////////// UPDATE POSITIONS ///////////////////
	if (inputState.reset) {
		float ballSpeedScale = 0.3f;
		ball.velocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
		ball.position = { 640 / 2.0f, 480 / 2.0f };
	}

	// player position 
	UpdatePlayerPosition(&player.position, inputState, (float)deltaTime);

	// ball position
	ball.position += ball.velocity * (float)deltaTime;

	//////////////// PHYSICS REESOLUTION /////////////////
	// Ball-Brick collision detection
	float blockWidth = (float)windowWidth / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);
	for (int i = 0; i < levelBricks.size(); ++i)
	{
		if (*((int*)(gameState.gameLevel) + i) != 0)
		{
			Collision collision = CheckCollision(ball.position, ball.dimension, levelBricks[i], { blockWidth, blockHeight });
			if (collision.hasCollided) // on collision
			{
				UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
				gameState.playerScore += gameState.gameLevel[i] * 10;
				gameState.gameLevel[i] = 0;
				AudioQueue[0] = 1;
			}
		}
	}

	// Ball-wall collision detection
	if (ball.position.x + ball.dimension.x > windowWidth)
	{
		ball.velocity.x = -ball.velocity.x;
		ball.position.x = windowWidth - ball.dimension.x;
	}
	else if (ball.position.y + ball.dimension.y > windowHeight)
	{
		ball.velocity.y = -ball.velocity.y;
		ball.position.y = windowHeight - ball.dimension.y;
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
	}

	// ball-paddle collision detection
	Collision collision = CheckCollision(ball.position, ball.dimension, player.position, player.dimension);
	UpdateBallOnCollision(ball.velocity, ball.position, ball.dimension, collision);
	if (collision.hasCollided)
	{
		AudioQueue[0] = 1;
		// this is essentially the command pattern, the command pattern states that you're yield a command, in this case the render queue implicitly describes the function to be called
		// the parameters then are the members of the command function essentially
		// this contrasts to the original version where I'd immediately call the function
		// separates when do do something with how to do it
		glm::vec2 difference = collision.difference;
		ball.velocity += 0.0005 * glm::length(difference);
	}
}

void RenderGame(std::vector<QuadRenderData>& RenderQueue, std::vector<TextRenderData>& TextRenderQueue,
	objectData ball, objectData player, int score, int* gameLevel)
{
	unsigned int windowWidth = 640, windowHeight = 480; // @TODO: some hardcoded window stuff
	//////////////////// DRAW PHASE /////////////////////////////////
	// Draw Ball
	RenderQueue.push_back({ ball.dimension, ball.position, ball.color, ball.textureId });

	// Draw Player
	RenderQueue.push_back({ player.dimension, player.position, player.color, ball.textureId });

	// Draw Blocks
	float blockWidth = (float)windowWidth / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);
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
			unsigned int TileType = gameLevel[rowIdx * BlockCols + colIdx];
			glm::vec4 ColorSelected = Colors[TileType];
			RenderQueue.push_back({ { blockWidth, blockHeight }, blockPos, ColorSelected, ball.textureId });
		}
	}
	// DrawText("SCORE: %d\r", data.playerScore);
	// really I would like a generalized renderer where I can pass an enum, so I can have a single queue
	TextRenderQueue.push_back({ "Score: " + std::to_string(score), { 100, 100 } });

	printf("SCORE: %d\r", score);
}

void RenderMenu(InputState& inputState, std::vector<QuadRenderData>& RenderQueue, std::vector<TextRenderData>& TextRenderQueue, uint32_t* AudioQueue, double deltaTime)
{
	unsigned int windowWidth = 640, windowHeight = 480; // @TODO: some hardcoded window stuff
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TextRenderQueue.push_back({ "PAUSED" , {windowWidth/2, windowHeight/2 } });
}

// do I peel off this layer?
void GameUpdateAndRender(GameState& gameState, InputState& inputState, std::vector<QuadRenderData>& RenderQueue, std::vector<TextRenderData>& TextRenderQueue, uint32_t* AudioQueue, double deltaTime)
{	
	if (!gameState.initializedResources)
	{
		gameState.ball.textureId = PlatformCreateTexture("res/textures/block.png");
		gameState.initializedResources = true;
	}
	ProcessInput(inputState, gameState);

	if (gameState.mode == GameMode::MENU)
	{
		RenderMenu(inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);
	}
	else if (gameState.mode == GameMode::ACTIVE)
	{
		SimulateGame(inputState, gameState.ball, gameState.player, gameState, deltaTime, AudioQueue);
		RenderGame(RenderQueue, TextRenderQueue, gameState.ball, gameState.player,
			gameState.playerScore, gameState.gameLevel);
	}
}