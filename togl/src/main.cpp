#include <SDL.h>
#include <glad/glad.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "breakout.cpp"
#include "main.h"



void GetOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

SDL_Window* initSDLOpenGLWindow(const char* WindowName, int Width, int Height)
{
	SDL_Init(SDL_INIT_VIDEO);
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
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GetOpenGLInfo();
}

SDL_Window* initWindowing(const char* WindowName, int Width, int Height)
{
	SDL_Window* Window = initSDLOpenGLWindow("My Window", Width, Height);
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

unsigned int CreateTexture(const char* filename, int GLpixelFormat)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* image = stbi_load(filename, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GLpixelFormat, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

TextTextureInfo CreateTextTexture(SDL_Surface* surface, unsigned int GLpixelFormat)
{
	TextTextureInfo result = {};
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, GLpixelFormat, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	result.textureID = texture;
	result.width = surface->w;
	result.height = surface->h;

	return result;
}

unsigned int CreateVao(float* Vertices, size_t VertexArraySize , unsigned int* Indices, size_t IndexArraySize)
{
	unsigned int Vao;
	glGenVertexArrays(1, &Vao);
	glBindVertexArray(Vao);

	unsigned int Vbo;
	glGenBuffers(1, &Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, Vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexArraySize, Vertices, GL_STATIC_DRAW);

	unsigned int Ebo;
	glGenBuffers(1, &Ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexArraySize, Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	return Vao;
}

void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition,const glm::vec4 Color, unsigned int texture,
						unsigned int Vao, unsigned int modelLoc, unsigned int colorLoc)
{
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(pixelPosition, 0.0f));
	model = glm::scale(model, glm::vec3(pixelDimensions, 1.0f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, glm::value_ptr(Color));

	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(Vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition, const glm::vec4 Color, unsigned int texture ,mainOGLContext context)
{
	DrawQuad(pixelDimensions, pixelPosition, Color, texture, context.Vao, context.modelLoc, context.colorLoc);
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
			}
		}
	}
	return true;
}

void initSDLMixerAudio()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf(SDL_GetError());

	}
	Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);
	return;
}

// calls TO platform FROM game
unsigned int PlatformCreateTexture(const char* filename) {
	return CreateTexture(filename, GL_RGB);
}

int main(int argc, char** args)
{
	// window 
	SDL_Window* Window = initWindowing("My Window", 976, 720);

	// audio
	initSDLMixerAudio();
	Mix_Chunk* bleep = Mix_LoadWAV("res/audio/solid.wav");
	Mix_Music* music = Mix_LoadMUS("res/audio/breakout.mp3");
	Mix_PlayMusic(music, -1);

	//shaders
	ShaderProgramSource source = ParseShader("res/shaders/Sprite.shader");
	unsigned int shaderProgram = CreateShaderProgram(source);

	// textures
	// unsigned int texture = CreateTexture("res/textures/block.png", GL_RGB);

	// Font Init
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("res/fonts/Urbanist-Medium.ttf", 64);


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
	unsigned int windowWidth = 640, windowHeight = 480;
	int projLoc = glGetUniformLocation(shaderProgram, "projection");
	glm::mat4 projection = glm::ortho(0.0f,(float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// MODEL
	glm::mat4 model(1.0f);

	// gamedata
	GameState gameState = initGameState();

	uint64_t LAST = SDL_GetPerformanceCounter();
	while (running)
	{
		uint64_t NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
		LAST = NOW;

		running = UpdateInputState(gameState.inputState);

		/////////////////////////// GAME UPDATE & Render ////////////////////////////
		GameUpdateAndRender(gameState, gameState.inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);

		// Sprite Render Queue
		for (QuadRenderData quadData : RenderQueue)
		{
			DrawQuad(quadData.pixelDimensions, quadData.pixelPosition, quadData.Color, quadData.textureId, oglContext);
		}
		RenderQueue.clear();

		// Text Render Queue
		for (TextRenderData textData : TextRenderQueue)
		{
			SDL_Surface* surfaceText = TTF_RenderUTF8_Blended(font, textData.text.c_str(), {255,255, 255});
			SDL_Surface* surfaceRGBA = SDL_ConvertSurfaceFormat(surfaceText, SDL_PIXELFORMAT_ABGR8888, 0);
			TextTextureInfo fonttexture = CreateTextTexture(surfaceRGBA, GL_RGBA);
			// draw centerd
			DrawQuad({ fonttexture.width / 2, fonttexture.height / 2 }, textData.pixelPosition - glm::vec2{ fonttexture.width / 4, fonttexture.height / 4 }, {1.0, 1.0, 1.0, 1.0}, fonttexture.textureID, oglContext);
			glDeleteTextures(1, &fonttexture.textureID);
		}
		TextRenderQueue.clear();

		// Audio "Queue"
		if (AudioQueue[0] == 1)
		{
			Mix_PlayChannel(-1, bleep, 0);
		}
		else if((AudioQueue[0] == 2))
		{
		}
		AudioQueue[0] = 10;


			
		SDL_GL_SwapWindow(Window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	return(0);
}