#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "breakout.cpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/matrix_transform_2d.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "main.h"

#include "SDLMixerAudio.cpp"

#include "RendererOGL.cpp"

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
			case SDLK_r:
			{
				inputstate.reset = true;
				break;
			}
			case SDLK_p:
			{
				inputstate.pause = true;
				break;
			}
			case SDLK_SPACE:
			{
				inputstate.space = true;
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
				inputstate.upProcessed = false;
				break;
			}
			case SDLK_DOWN:
			{
				inputstate.down = false;
				inputstate.downProcessed = false;
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
			case SDLK_r:
			{
				inputstate.reset = false;
				break;
			}
			case SDLK_p:
			{
				inputstate.pause = false;
				inputstate.pauseProcessed = false;
				break;
			}
			case SDLK_SPACE:
			{
				inputstate.space = false;
				inputstate.spaceProcessed = false;
			}
			}
		}
	}
	return true;
}

// calls TO platform FROM game
unsigned int PlatformCreateTexture(const char* filename, int pixelFormat) {
	if (pixelFormat == 0)
	{
		return CreateTexture(filename, GL_RGB);
	}
	return CreateTexture(filename, GL_RGBA);
}

void* PlatformLoadFont(const char* filename, unsigned int pts)
{
	TTF_Font* font = TTF_OpenFont(filename, pts);
	return (void*)font;
}

int main(int argc, char** args)
{
	std::srand(1231412);
	// window 
	unsigned int windowWidth = 960;
	unsigned int windowHeight = 720;
	SDL_Window* Window = initWindowing(	"Breakout", windowWidth, windowHeight);

	// audio
	initSDLMixerAudio();

	//shaders
	ShaderProgramSource source = ParseShader("res/shaders/Sprite.shader");
	unsigned int shaderProgram = CreateShaderProgram(source);

	// textures now loaded in the platform via PlatformCreateTexture;

	// Font Init
	TTF_Init();
	TTF_Font* debugfont = TTF_OpenFont("res/fonts/Urbanist-Medium.ttf", 64);

	/// Framebuffer
	unsigned int Fbo;
	glGenFramebuffers(1, &Fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
	// texture binding
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, worldWidth, worldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// binding the above texture 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mainOGLContext oglFlipContext = {};
	{
		int modelLoc = glGetUniformLocation(shaderProgram, "model");
		int colorLoc = glGetUniformLocation(shaderProgram, "tileColor");
		unsigned int Vao = CreateVao(quadFlipVertices, sizeof(quadFlipVertices), quadElementIndices, sizeof(quadElementIndices));

		oglFlipContext.Vao = Vao;
		oglFlipContext.modelLoc = modelLoc;
		oglFlipContext.colorLoc = colorLoc;
	}

	mainOGLContext oglContext = {};
	{
		int modelLoc = glGetUniformLocation(shaderProgram, "model");
		int colorLoc = glGetUniformLocation(shaderProgram, "tileColor");
		unsigned int Vao = CreateVao(quadVertices, sizeof(quadVertices), quadElementIndices, sizeof(quadElementIndices));

		oglContext.Vao = Vao;
		oglContext.modelLoc = modelLoc;
		oglContext.colorLoc = colorLoc;
	}

	// GLOBAL PROJECTION
	int projLoc = glGetUniformLocation(shaderProgram, "projection");

	/// need to pass the platform layer a concept of world dimension
	unsigned int WorldWidth = 800;
	unsigned int WorldHeight = 600;
	glm::mat4 projection = glm::ortho(0.0f, (float)WorldWidth, 0.0f, (float)WorldHeight, -1.0f, 1.0f);

	//glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// MODEL
	glm::mat4 model(1.0f);

	// gamedata
	GameState gameState = initGameState();

	uint64_t LAST = SDL_GetPerformanceCounter();

	glm::vec2 playerPosition = { 300, 300 };
	glm::vec2 playerDimension = { 20.f , 20.f };
	struct bullet
	{
		glm::vec2 dim;
		glm::vec2 pos;
		glm::vec2 dir;
		float speed = 0.7;
	};

	struct asteroid
	{
		glm::vec2 dim;
		glm::vec2 pos;
		glm::vec2 dir;
		float speed = 0.2;
	};

	std::vector<bullet> bullets;
	std::vector<asteroid> asteroids;
	unsigned int textureId = CreateTexture("res/textures/paddle.png", GL_RGBA);
	unsigned int notextId = CreateTexture("res/textures/ship.png", GL_RGBA);

	while (gameState.running)
	{
		uint64_t NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
		LAST = NOW;

		gameState.running = UpdateInputState(gameState.inputState);
		if (!gameState.running) break;

		glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
#if 1
		WorldWidth = 800;
		WorldHeight = 600;


		static int only1 = 0;
		if (!only1)
		{
			asteroid basicasteroid = {};
			basicasteroid.dim = { 50.0f, 50.f };
			basicasteroid.pos = { 0.0f, 0.f };
			basicasteroid.dir = { 800, 600 };
			
			asteroids.push_back(basicasteroid);

			only1 = 1;
		}

		for (auto& asteroid : asteroids)
		{
			asteroid.pos += glm::normalize(asteroid.dir) * asteroid.speed * (float) deltaTime;
			DrawQuad(asteroid.dim, asteroid.pos, 0, { 2.0f, 1.0f, 2.0f, 1.0f }, textureId, oglFlipContext);
		}


		static float rotationAngle = 0.0;
		static float playerInertia = 0.0f;
		float rotationVelocity = 0.004;
		float playerSpeed = 0.00015;

		glm::vec2 orientation = { 0.0f, 1.0f };
		static glm::vec2 inertiaVector = { 0.0f, 0.0f };
		glm::mat3 rotation = glm::rotate(glm::mat3(1.0f), rotationAngle);
		orientation = glm::normalize(rotation * glm::vec3(orientation, 0.0f));
		float rotDeg = atan2f(orientation.y, orientation.x) * 180.f / 3.14;
		printf("xdir: %f, ydir: %f, rotDeg: %f\r", orientation.x, orientation.y, rotDeg);

		if (gameState.inputState.right)
		{
			rotationAngle -= rotationVelocity * deltaTime;
		}
		if (gameState.inputState.left)
		{
			rotationAngle += rotationVelocity * deltaTime;
		}
		if (gameState.inputState.up)
		{
			inertiaVector = (playerSpeed) / (playerSpeed + playerInertia) * orientation + (playerInertia) / (playerSpeed + playerInertia) * inertiaVector;
			playerInertia += glm::dot(glm::normalize(orientation), glm::normalize(inertiaVector)) / 200.f;
			if (playerInertia > 0.4)
			{
				playerInertia = 0.4;
			}
		}
		if (playerInertia > 0.00032)
		{
			playerInertia -= 0.00030;
		}
		else
		{
			playerInertia = 0.f;
		}

		playerPosition += playerInertia * inertiaVector;

		if (playerPosition.x > 790)
		{
			playerPosition.x -= 790;
		}
		if (playerPosition.y > 580)
		{
			playerPosition.y -= 580;
		}
		if (playerPosition.x < 10)
		{
			playerPosition.x += 790;
		}
		if (playerPosition.y < 10)
		{
			playerPosition.y += 580;
		}
		if (gameState.inputState.space && !(gameState.inputState.spaceProcessed))
		{
			// need a fire rate
			bullet Bullet = {};
			Bullet.dim = { 10.0f, 10.0f };
			Bullet.pos = playerPosition;
			Bullet.dir = orientation;
			Bullet.speed += playerInertia;
			bullets.push_back(Bullet);
			gameState.inputState.spaceProcessed = true;
		}
		for (auto& bullet : bullets)
		{
			bullet.pos += bullet.speed * bullet.dir;
		}


		glm::vec4 playerColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		//glm::rotate()

		DrawQuad(playerDimension, playerPosition, rotDeg, playerColor, notextId, oglFlipContext);
		for (auto& bullet : bullets)
		{
			float rotDeg = atan2f(bullet.dir.y, bullet.dir.x) * 180.f / 3.14;
			DrawQuad(bullet.dim, bullet.pos, rotDeg, playerColor, notextId, oglFlipContext);
		}


#else
		/////////////////////////// GAME UPDATE & Render ///////////////////////////
		GameUpdateAndRender(gameState, gameState.inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);

		// Sprite Render Queue
		int drawcalls = 0;
		for (QuadRenderData quadData : RenderQueue)
		{
			DrawQuad(quadData.pixelDimensions, quadData.pixelPosition, quadData.rotation, quadData.Color, quadData.textureId, oglFlipContext);
			++drawcalls;
		}
		for (auto object : gameState.powerUps)
		{
			DrawQuad(object.dimension, object.position, 0, object.color, object.textureId, oglFlipContext);
			++drawcalls;
		}
		RenderQueue.clear();


		// Text Render Queue
		for (TextRenderData textData : TextRenderQueue)
		{
			TextTextureInfo fonttexture = CreateTextTexture(textData.text, (TTF_Font*)textData.font);
			// draw centered
			// if I just constructed a blank one this wouldn't be a problem 
			DrawQuad(
				{ fonttexture.width / 2, fonttexture.height / 2 },
				textData.pixelPosition - glm::vec2{ fonttexture.width / 4, fonttexture.height / 4 },
				0,
				textData.color,
				fonttexture.textureID,
				oglFlipContext);
			glDeleteTextures(1, &fonttexture.textureID);
		}
		TextRenderQueue.clear();

		/// debug draw string
		{
			std::string MS = "MS: " + std::to_string(deltaTime);
			DebugDrawString({ 0.0, 0.0 }, MS, debugfont, oglFlipContext);
			DebugDrawString({ 0.0, 64.0 }, std::to_string(drawcalls), debugfont, oglFlipContext);
		}

		// Audio "Queue"
		for (void* sound : AudioQueue)
		{
			Mix_PlayChannel(-1, (Mix_Chunk*)sound, 0);
		}
		AudioQueue.clear();
	
#endif
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glDisable(GL_DEPTH_TEST);
		DrawQuad({ windowWidth, windowHeight}, { 0, 0 }, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, oglContext);

		SDL_GL_SwapWindow(Window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	return(0);
}