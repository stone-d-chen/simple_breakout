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


#include "breakout.cpp"

float quadVertices[] =
{
	 0.0f,	0.0f, 0.0f,	0.0f,
	 0.0f,	1.0f, 0.0f,	1.0f,
	 1.0f,  1.0f, 1.0f,	1.0f,
	 1.0f,  0.0f, 1.0f, 0.0f,
};
unsigned int quadElementIndices[] =
{
	0,1,2,
	0,2,3
};

bool running = true;

InputState inputState = {};

std::vector<QuadRenderData> RenderQueue;

unsigned int windowWidth = 640, windowHeight = 480;


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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	return Vao;
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

int main(int argc, char** args)
{
	SDL_Window* Window = initWindowing("My Window", windowWidth, windowHeight);

	//shaders
	ShaderProgramSource source = ParseShader("res/shaders/Sprite.shader");
	unsigned int shaderProgram = CreateShaderProgram(source);

	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	int projLoc = glGetUniformLocation(shaderProgram, "projection");
	int colorLoc = glGetUniformLocation(shaderProgram, "TileColor");

	unsigned int Vao = CreateVao(quadVertices, sizeof(quadVertices), quadElementIndices, sizeof(quadElementIndices));
	
	// PROJECTION
	glm::mat4 projection = glm::ortho(0.0f,(float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// MODEL
	glm::mat4 model(1.0f);

	uint64_t LAST = SDL_GetPerformanceCounter();
	while (running)
	{
		uint64_t NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
		LAST = NOW;


		running = UpdateInputState(inputState);

		/////////////////////////// GAME UPDATE //////////////////////////////////////////
		GameUpdateAndRender(deltaTime, inputState, RenderQueue);

		for (QuadRenderData Data : RenderQueue)
		{
			DrawQuad(Data.pixelDimensions, Data.pixelPosition, Data.Color, Vao, modelLoc, colorLoc);
		}
		RenderQueue.clear();

		SDL_GL_SwapWindow(Window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		
	}
	return(0);
}