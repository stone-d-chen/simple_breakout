#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include "breakout.cpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "main.h"

void GetOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	int MajorVersion;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion);
	std::cout << "GET: " << MajorVersion << std::endl;
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
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
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
	SDL_Window* Window = initSDLOpenGLWindow(WindowName, Width, Height);
	initSDLOpenGLBinding();

	return Window;
}

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

	glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GLpixelFormat, GL_UNSIGNED_BYTE, (void*)image);
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

	// surface was nullptr
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

void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition, float rotateRadians, const glm::vec4 Color, unsigned int texture,
						unsigned int Vao, unsigned int modelLoc, unsigned int colorLoc)
{
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(pixelPosition, 0.0f));

	model = glm::translate(model, glm::vec3(0.5f * pixelDimensions.x, 0.5f * pixelDimensions.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotateRadians), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5 * pixelDimensions.x, -0.5f * pixelDimensions.y, 0.0f));

	model = glm::scale(model, glm::vec3(pixelDimensions, 1.0f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(colorLoc, 1, glm::value_ptr(Color));

	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(Vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition, float rotation, const glm::vec4 Color, unsigned int texture, mainOGLContext context)
{
	DrawQuad(pixelDimensions, pixelPosition, rotation, Color, texture, context.Vao, context.modelLoc, context.colorLoc);
}

void PlatformClear(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return;
}

void Clear(glm::vec4 color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
unsigned int PlatformCreateTexture(const char* filename, int pixelFormat) {
	if (pixelFormat == 0)
	{
		return CreateTexture(filename, GL_RGB);
	}
	return CreateTexture(filename, GL_RGBA);
}

void* PlatformLoadWAV(const char* filename)
{
	Mix_Chunk* sound = Mix_LoadWAV(filename);
	return (void*)sound;
}

void* PlatformPlayMusic(const char* filename)
{
	Mix_Music* music = Mix_LoadMUS(filename);
	Mix_PlayMusic(music, -1);
	return (void*)music;
}

int main(int argc, char** args)
{
	std::srand(1231412);
	// window 
	SDL_Window* Window = initWindowing("Breakout", 640, 480);

	// audio
	initSDLMixerAudio();

	//shaders
	ShaderProgramSource source = ParseShader("res/shaders/Sprite.shader");
	unsigned int shaderProgram = CreateShaderProgram(source);

	// textures now loaded in the platform via PlatformCreateTexture;

	// Font Init
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("res/fonts/Urbanist-Medium.ttf", 64);

	unsigned int windowWidth = 640, windowHeight = 480;
	unsigned int Fbo;
	glGenFramebuffers(1, &Fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, Fbo);

	// texture binding
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// MODEL
	glm::mat4 model(1.0f);

	// gamedata
	GameState gameState = initGameState();

	uint64_t LAST = SDL_GetPerformanceCounter();
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

		/////////////////////////// GAME UPDATE & Render ////////////////////////////
		GameUpdateAndRender(gameState, gameState.inputState, RenderQueue, TextRenderQueue, AudioQueue, deltaTime);

		// Sprite Render Queue
		for (QuadRenderData quadData : RenderQueue)
		{
			DrawQuad(quadData.pixelDimensions, quadData.pixelPosition, quadData.rotation, quadData.Color, quadData.textureId, oglFlipContext);
		}
		for (auto object : gameState.powerUps)
		{
			DrawQuad(object.dimension, object.position, 0, object.color, object.textureId, oglFlipContext);
		}
		RenderQueue.clear();


		// Text Render Queue
		for (TextRenderData textData : TextRenderQueue)
		{
			SDL_Surface* surfaceText = TTF_RenderUTF8_Blended(font, textData.text.c_str(), { 255, 255, 255 });
			SDL_Surface* surfaceRGBA = SDL_ConvertSurfaceFormat(surfaceText, SDL_PIXELFORMAT_ABGR8888, 0);
			TextTextureInfo fonttexture = CreateTextTexture(surfaceRGBA, GL_RGBA);
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
			SDL_FreeSurface(surfaceRGBA);
			SDL_FreeSurface(surfaceText);
		}
		TextRenderQueue.clear();

		// Audio "Queue"
		for (void* sound : AudioQueue)
		{
			Mix_PlayChannel(-1, (Mix_Chunk*)sound, 0);
		}
		AudioQueue.clear();
	

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glDisable(GL_DEPTH_TEST);
		DrawQuad({ 640, 480 }, { 0, 0 }, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, texture, oglContext);

		SDL_GL_SwapWindow(Window);
	}
	return(0);
}