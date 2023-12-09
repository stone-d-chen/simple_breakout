#include "breakout.h"
#include "SDL.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Players Levels and Tiles
glm::vec2 playerPosition = { 640 / 2.0f , 480 * 1.0 / 10.0f };
glm::vec2 playerDimensions = { 100.0f, 20.0f };
glm::vec4 playerColor = { 1.0, 0.0, 0.0, 1.0 };

float ballSpeedScale = 0.3f;
glm::vec2 ballVelocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
glm::vec2 ballPosition = { 640 / 2.0f, 480 / 2.0f };
glm::vec2 ballDimensions = { 20.0f, 20.0f };
glm::vec4 ballColor = { 0.0, 0.7, 0.0, 1.0 };

const int BlockRows = 3;
const int BlockCols = 6;
int gameLevel[BlockRows * BlockCols] =
{
	0, 1, 2, 3, 4, 1,
	2, 1, 0, 0, 1, 2,
	3, 0, 4, 2, 1, 0,
};

glm::vec4 Colors[] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.3f, 0.4f, 0.5f, 1.0f },
	{ 0.0f, 0.4f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 0.5f, 1.0f },
	{ 0.3f, 0.0f, 0.5f, 1.0f },
};

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


std::vector<glm::vec2> CreateBrickPositions(unsigned int BlockRows, unsigned int BlockCols /*, windowWidth, windowHeight */)
{
	unsigned int windowWidth = 640, windowHeight = 480;

	std::vector<glm::vec2> brickPositions;

	float blockWidth = (float)640 / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);

	for (int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			float blockPositionX = colIdx * blockWidth;
			float blockPositionY = windowHeight - (rowIdx + 1) * blockHeight;

			brickPositions.push_back({ blockPositionX, blockPositionY });
		}
	}
	return brickPositions;
}

const std::vector<glm::vec2> levelBricks = CreateBrickPositions(BlockRows, BlockCols);

bool AABBCollisionDetection(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo)
{
	bool collisionX = (positionOne.x + dimensionOne.x >= positionTwo.x) && (positionOne.x <= positionTwo.x + dimensionTwo.x);
	bool collisionY = (positionOne.y + dimensionOne.y >= positionTwo.y) && (positionOne.y <= positionTwo.y + dimensionTwo.y);
	return collisionX && collisionY;
}

Collision CheckCollision(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo) // AABB - Circle collision
{
	glm::vec2 Center = positionOne + 0.5f * dimensionOne; // get center point circle first 
	
	glm::vec2 aabbHalfExtents = 0.5f * dimensionTwo;    // calculate AABB info (center, half-extents)
	glm::vec2 aabbCenter = positionTwo + aabbHalfExtents;
	
	glm::vec2 Difference = Center - aabbCenter;  // get difference vector between both centers
	glm::vec2 Clamped = glm::clamp(Difference, -aabbHalfExtents, aabbHalfExtents);

	glm::vec2 Closest = aabbCenter + Clamped; // add clamped value to AABB_center and we get the value of box closest to circle
	
	Difference = Center - Closest; // retrieve vector between center circle and closest point AABB and check if length <= radius

	if (glm::length(Difference) <= glm::length(dimensionOne * 0.5f))
		return std::make_tuple(true, VectorDirection(Difference), Difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}


void UpdateBallOnCollision(glm::vec2& ballVelocity, glm::vec2& ballPosition, const glm::vec2& ballDimensions, Collision collision)
{
	if (std::get<0>(collision))
	{
		Direction dir = std::get<1>(collision);
		glm::vec2 diffVector = std::get<2>(collision);
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

void GameUpdateAndRender(double deltaTime, InputState inputState, std::vector<QuadRenderData>& RenderQueue)
{
	unsigned int windowWidth = 640, windowHeight = 480; // @TODO: some hardcoded window stuff

	///////////// UPDATE POSITIONS ///////////////

	if (inputState.r)
	{
		ballSpeedScale = 0.3f;
		ballVelocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
		ballPosition = { 640 / 2.0f, 480 / 2.0f };
	}

	// player position 
	UpdatePlayerPosition(&playerPosition, inputState, (float)deltaTime);

	// ball position
	ballPosition += ballVelocity * (float)deltaTime;

	//////////////// PHYSICS REESOLUTION /////////////////
	// Ball-Brick collision detection
	float blockWidth = (float)windowWidth / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);
	for (int i = 0; i < levelBricks.size(); ++i)
	{
		if (*((int*)(gameLevel)+i) != 0)
		{
			Collision collision = CheckCollision(ballPosition, ballDimensions, levelBricks[i], { blockWidth, blockHeight });
			if (std::get<0>(collision))
			{
				UpdateBallOnCollision(ballVelocity, ballPosition, ballDimensions, collision);
				gameLevel[i] = 0;
			}
		}
	}

	// Ball-wall collision detection
	if (ballPosition.x + ballDimensions.x > windowWidth)
	{
		ballVelocity.x = -ballVelocity.x;
		ballPosition.x = windowWidth - ballDimensions.x;
	}
	else if (ballPosition.y + ballDimensions.y > windowHeight)
	{
		ballVelocity.y = -ballVelocity.y;
		ballPosition.y = windowHeight - ballDimensions.y;
	}
	else if (ballPosition.x < 0)
	{
		ballVelocity.x = -ballVelocity.x;
		ballPosition.x = 0;
	}
	else if (ballPosition.y < 0)
	{
		ballVelocity = { 0.0f, 0.0f };
		ballPosition.y = 0;
	}

	// ball-paddle collision detection
	Collision collision = CheckCollision(ballPosition, ballDimensions, playerPosition, playerDimensions);
	UpdateBallOnCollision(ballVelocity, ballPosition, ballDimensions, collision);
	glm::vec2 difference = std::get<2>(collision);
	ballVelocity += 0.005 * glm::length(difference);


	//////////////////// DRAW PHASE /////////////////////////////////

	// Draw Ball
	RenderQueue.push_back({ ballDimensions, ballPosition, ballColor });

	// Draw Player
	RenderQueue.push_back({ playerDimensions, playerPosition, playerColor });

	// Draw Blocks
	for (int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			glm::vec2 blockPos = levelBricks[rowIdx * BlockCols + colIdx];
			unsigned int TileType = gameLevel[rowIdx * BlockCols + colIdx];

			glm::vec4 ColorSelected = Colors[TileType];

			RenderQueue.push_back({ { blockWidth, blockHeight }, blockPos, ColorSelected });
		}
	}

}