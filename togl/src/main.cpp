#include <SDL.h>
#include <glad/glad.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <tuple>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendors/stb_image/stb_image.h"

float clamp(float value, float min, float max) {
	return std::max(min, std::min(max, value));
}


enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT
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

typedef std::tuple<bool, Direction, glm::vec2> Collision;

bool running = true;

struct InputState
{
	bool up;
	bool down;
	bool left;
	bool right;
};
InputState inputState = {};


void GetOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

SDL_Window* initSDLOpenGLWindow(const char* WindowName, int Width, int Height)
{
	SDL_Window* Window = SDL_CreateWindow(WindowName,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		Width, Height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GLContext Context = SDL_GL_CreateContext(Window);

	return Window;
}
void initSDLOpenGLBinding()
{
	SDL_Init(SDL_INIT_VIDEO);
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	glEnable(GL_DEPTH_TEST);

	GetOpenGLInfo();
}

SDL_Window* initWindowing(const char* WindowName, int Width, int Height)
{
	SDL_Window* Window = initSDLOpenGLWindow("My Window", 800, 600);
	initSDLOpenGLBinding();

	return Window;
}

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);
	std::string line;
	std::stringstream ss[2];

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

unsigned int CreateShaderProgram(const ShaderProgramSource& source)
{
	const char* vertexShaderSource = source.VertexSource.c_str();

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//Fragment Shader
	const char* fragmentShaderSource = source.FragmentSource.c_str();

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Shader Program //
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);
	return shaderProgram;
}


void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition, const glm::vec4 Color, unsigned int Vao, unsigned int modelLoc, unsigned int colorLoc)
{
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(pixelPosition, 0.0f));
	model = glm::scale(model, glm::vec3(pixelDimensions, 1.0f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, glm::value_ptr(Color));

	glBindVertexArray(Vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


bool UpdateInputState(InputState& inputstate)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			return false;
		};
		if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			{
				// Up Arrow
				inputstate.up = true;
				break;
			}
			case SDLK_DOWN:
			{
				inputstate.down = true;
				break;
			}
			case SDLK_LEFT:
			{
				// Left Arrow
				inputstate.left = true;
				break;
			}
			case SDLK_RIGHT:
			{
				inputstate.right = true;
				break;
			}
			}
		}
		if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			{
				// Up Arrow
				inputstate.up = false;
				break;
			}
			case SDLK_DOWN:
			{
				inputstate.down = false;
				break;
			}
			case SDLK_LEFT:
			{
				// Left Arrow
				inputstate.left = false;
				break;
			}
			case SDLK_RIGHT:
			{
				inputstate.right = false;
				break;
			}
			}
		}
	}
	return true;
}

unsigned int windowWidth = 640, windowHeight = 480;

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

glm::vec2 playerPosition = { windowWidth / 2.0f , windowHeight * 1.0/ 10.0f };

float playerVertices[] = {
	 0.0f,	0.0f, 0.0f,	0.0f,
	 0.0f,	1.0f, 0.0f,	1.0f,
	 1.0f,  1.0f, 1.0f,	1.0f,
	 1.0f,  0.0f, 1.0f, 0.0f,
};

unsigned int playerElementIndices[] =
{
	0,1,2,
	0,2,3
};

// levels and tiles
const int BlockRows = 3;
const int BlockCols = 6;
int gameLevel[BlockRows * BlockCols] = {
	0, 1, 2, 3, 4, 1,
	2, 1, 0, 0, 1, 2,
	3, 0, 4, 2, 1, 0,
};

std::vector<glm::vec2> CreateBrickPositions(unsigned int BlockRows, unsigned int BlockCols /*, windowWidth, windowHeight */)
{
	std::vector<glm::vec2> brickPositions;

	float blockWidth = (float)windowWidth / (float)BlockCols;
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

float blockVertex[] = {
	 0.0f,	0.0f, 0.0f,	0.0f,
	 0.0f,	1.0f, 0.0f,	1.0f,
	 1.0f,  1.0f, 1.0f,	1.0f,
	 1.0f,  0.0f, 1.0f, 0.0f,
};
unsigned int blockIndices[] = {
	0,1,2,
	0,2,3
};
glm::vec4 Color1 = { 0.3f,0.4f,0.5f,1.0f };
glm::vec4 Color2 = { 0.0f,0.4f,0.0f,1.0f };
glm::vec4 Color3 = { 0.0f,0.0f,0.5f,1.0f };
glm::vec4 Color4 = { 0.3f,0.0f,0.5f,1.0f };
glm::vec4 ColorDefault = { 0.0f,0.0f,0.0f, 0.0f };

float ballSpeedScale = 0.3f;
glm::vec2 ballVelocity = { 1.0f * ballSpeedScale, 1.0f * ballSpeedScale };
glm::vec2 ballPosition = { windowWidth/2.0f, windowHeight/2.0f  };
float ballVertices[] = {
	 0.0f,	0.0f, 0.0f,	0.0f,
	 0.0f,	1.0f, 0.0f,	1.0f,
	 1.0f,  1.0f, 1.0f,	1.0f,
	 1.0f,  0.0f, 0.0f, 0.0f,
};
unsigned int ballIndices[] = {
	0,1,2,
	0,2,3
};



void UpdatePlayerPosition(glm::vec2* playerPosition, InputState inputState, float deltaTime)
{
	glm::vec2 playerPositionDelta = { 0.0f, 0.0f };
	if (inputState.right)
	{
		playerPositionDelta = glm::vec2(1.0f, 0.0f) * 0.5f * deltaTime;
	}
	if (inputState.left)
	{
		playerPositionDelta = glm::vec2(-1.0f, 0.0f) * 0.5f * deltaTime;
	}

	*playerPosition += playerPositionDelta;
}

bool AABBCollisionDetection(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo)
{
	bool collisionX = (positionOne.x + dimensionOne.x >= positionTwo.x) && (positionOne.x <= positionTwo.x + dimensionTwo.x);
	bool collisionY = (positionOne.y + dimensionOne.y >= positionTwo.y) && (positionOne.y <= positionTwo.y + dimensionTwo.y);
	return collisionX && collisionY;
}

Collision CheckCollision(glm::vec2 positionOne, glm::vec2 dimensionOne, glm::vec2 positionTwo, glm::vec2 dimensionTwo) // AABB - Circle collision
{
	// get center point circle first 
	glm::vec2 Center = positionOne + 0.5f * dimensionOne;

	// calculate AABB info (center, half-extents)
	glm::vec2 aabbHalfExtents = 0.5f * dimensionTwo;
	glm::vec2 aabbCenter = positionTwo + aabbHalfExtents;

	// get difference vector between both centers
	glm::vec2 Difference = Center - aabbCenter;
	glm::vec2 Clamped = glm::clamp(Difference, -aabbHalfExtents, aabbHalfExtents);
	
	// add clamped value to AABB_center and we get the value of box closest to circle
	glm::vec2 Closest = aabbCenter + Clamped;
	
	// retrieve vector between center circle and closest point AABB and check if length <= radius
	Difference = Center - Closest;

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
			{
				ballPosition.x -= penetration;
			}
			else
			{
				ballPosition.x += penetration;
			}
		}
		else
		{
			float penetration = glm::length(ballDimensions * 0.5f) - std::abs(diffVector.y);

			ballVelocity.y = -ballVelocity.y;

			if (dir == DOWN)
			{
				ballPosition.y -= penetration;
			}
			else
			{
				ballPosition.y += penetration;
			}
		}
	}
}



int main(int argc, char** args)
{
	SDL_Window* Window = initWindowing("My Window", 800, 600);

	//shaders
	ShaderProgramSource source = ParseShader("res/shaders/Sprite.shader");
	unsigned int shaderProgram = CreateShaderProgram(source);

	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	int projLoc = glGetUniformLocation(shaderProgram, "projection");
	int colorLoc = glGetUniformLocation(shaderProgram, "TileColor");

	struct objectData
	{
		glm::vec2 position;
		glm::vec2 dimension;

		glm::vec4 color;
	};

	// Create Vao //
	unsigned int Vao;
	glGenVertexArrays(1, &Vao);
	unsigned int Vbo;
	glBindVertexArray(Vao);

	glGenBuffers(1, &Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, Vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(playerVertices), playerVertices, GL_STATIC_DRAW);

	unsigned int Ebo;
	glGenBuffers(1, &Ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(playerElementIndices), playerElementIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	

	// PROJECTION
	glm::mat4 projection = glm::ortho(0.0f,(float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// MODEL
	glm::mat4 model(1.0f);
	std::vector<glm::vec2> levelBricks = CreateBrickPositions(BlockRows, BlockCols);

	while (running)
	{
		double deltaTime = 0;
		Uint64 NOW = SDL_GetPerformanceCounter();
		Uint64 LAST = 0;
		LAST = NOW;


		//////////////////// DRAW PHASE /////////////////////////////////
		// 
		// Draw Ball


		glm::vec2 ballDimensions = { 20.0f, 20.0f };
		glm::vec4 ballColor(0.0, 0.7, 0.0, 1.0);

		objectData ball =
		{
			ballPosition,
			{20.0f, 20.0f},
		};
		ball.color = ballColor;

		DrawQuad(ball.dimension, ballPosition, ball.color, Vao, modelLoc, colorLoc);

		
		// Draw Player
		glm::vec2 playerDimensions = { 100.0f, 20.0f };
		glm::vec4 playerColor(1.0, 0.0, 0.0, 1.0);
		DrawQuad(playerDimensions, playerPosition, playerColor, Vao, modelLoc, colorLoc);

		// Draw Blocks
		float blockWidth = (float)windowWidth / (float) BlockCols;
		float blockHeight = windowHeight / ((float)BlockRows * 3.0f);
		for (int rowIdx = 0; rowIdx < BlockRows; ++rowIdx)
		{
			for (int colIdx = 0; colIdx < BlockCols; ++colIdx)
			{
				glm::vec2 blockPos = levelBricks[rowIdx * BlockCols + colIdx];

				glm::vec4 ColorSelected = ColorDefault;
				{ // color select
					unsigned int TileType = gameLevel[rowIdx * BlockCols + colIdx];
					switch (TileType)
					{
					case 0:
					{
						ColorSelected = Color1;
					} break;
					case 1:
					{
						ColorSelected = Color2;
					} break;
					case 2:
					{
						ColorSelected = Color3;
					} break;
					case 3:
					{
						ColorSelected = Color4;
					} break;
					default:
					{
						ColorSelected = ColorDefault;
					}
					}
				}
				DrawQuad({ blockWidth, blockHeight }, blockPos, ColorSelected, Vao, modelLoc, colorLoc);
			}
		}

		SDL_GL_SwapWindow(Window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// DeltaTime ticks
		NOW = SDL_GetPerformanceCounter();
		deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		/////////////////////////// GAME UPDATE //////////////////////////////////////////
		
		running = UpdateInputState(inputState);
		UpdatePlayerPosition(&playerPosition, inputState, (float)deltaTime);
		
		// Update ball velocity
		ballPosition += ballVelocity * (float)deltaTime;

		// Ball-Brick collision detection
		for (int i = 0; i < levelBricks.size(); ++i)
		{
			if (*((int*)(gameLevel)+i) != 4)
			{
				Collision collision = CheckCollision(ballPosition, ballDimensions, levelBricks[i], {blockWidth, blockHeight});
				if (std::get<0>(collision))
				{
					UpdateBallOnCollision(ballVelocity, ballPosition, ballDimensions, collision);
					gameLevel[i] = 4;
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
		
	}
	return(0);
}