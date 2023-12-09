#include "breakout.h"
#include "SDL.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


// Players Levels and Tiles
glm::vec2 playerPosition = { 640 / 2.0f , 480 * 1.0 / 10.0f };
glm::vec2 playerDimensions = { 100.0f, 20.0f };
glm::vec4 playerColor = { 1.0, 0.0, 0.0, 1.0 };

void GameUpdateAndRender()
{
	UpdatePlayerPosition(&playerPosition, inputState, (float)deltaTime);

	// Update ball velocity
	ballPosition += ballVelocity * (float)deltaTime;

	float blockWidth = (float)windowWidth / (float)BlockCols;
	float blockHeight = windowHeight / ((float)BlockRows * 3.0f);
	// Ball-Brick collision detection
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

	// Ball collision detection
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

	// ball paddle collision detection
	Collision collision = CheckCollision(ballPosition, ballDimensions, playerPosition, playerDimensions);
	UpdateBallOnCollision(ballVelocity, ballPosition, ballDimensions, collision);


	//////////////////// DRAW PHASE /////////////////////////////////
	// 
	// Draw Ball

	objectData ball =
	{
		ballPosition,
		{20.0f, 20.0f},
	};
	ball.color = ballColor;

	DrawQuad(ball.dimension, ballPosition, ball.color, Vao, modelLoc, colorLoc);


	// Draw Player

	DrawQuad(playerDimensions, playerPosition, playerColor, Vao, modelLoc, colorLoc);

	// Draw Blocks

	for (int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
	{
		for (int colIdx = 0; colIdx < BlockCols; ++colIdx)
		{
			glm::vec2 blockPos = levelBricks[rowIdx * BlockCols + colIdx];
			unsigned int TileType = gameLevel[rowIdx * BlockCols + colIdx];

			glm::vec4 ColorSelected = Colors[TileType];

			DrawQuad({ blockWidth, blockHeight }, blockPos, ColorSelected, Vao, modelLoc, colorLoc);
		}
	}

}