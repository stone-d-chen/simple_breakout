#include <SDL.h>
#include <glad/glad.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendors/stb_image/stb_image.h"

#include "Buffer.h"
#include "Camera.h"
//#include "VertexArray.h"

glm::vec3 positionDelta = {};
bool running = true;



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

static GLenum ShaderDataTypeTOOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float: return GL_FLOAT;
	case ShaderDataType::Float2: return GL_FLOAT;
	case ShaderDataType::Float3: return GL_FLOAT;
	case ShaderDataType::Float4: return GL_FLOAT;
	case ShaderDataType::Int2: return GL_INT;
	case ShaderDataType::Int: return GL_INT;
	case ShaderDataType::Int3: return GL_INT;
	case ShaderDataType::Int4: return GL_INT;
	case ShaderDataType::Bool: return GL_INT;
	case ShaderDataType::Mat3: return GL_FLOAT;
	case ShaderDataType::Mat4: return GL_FLOAT;

	}
}

void GetOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
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

void LoadImageAsTexture(const char* filename)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	unsigned int Texture;
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}

void UpdateInputState(InputState& inputstate)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) running = false;
		if (e.type == SDL_KEYDOWN)
		{
			printf("SDL_KEYDOWN: ");
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			{
				// Up Arrow
				inputstate.up = true;
				printf("SDL_UP\n");
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
}

int main(int argc, char** args)
{
	SDL_Window* Window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GLContext Context = SDL_GL_CreateContext(Window);


	initSDLOpenGLBinding();

	LoadImageAsTexture("res/textures/container.jpg");

	/////////////////////////
	// ----- Shader ------ //
	/////////////////////////

	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

	// verxet shader
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

	//shader program
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	// ---- load mesh data --- // 
	#include "../cube_vertices"
	unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

	// ----- Vertex Array Buffers ----- //
	// generate a vertex array buffer
	unsigned int Vao;
	glGenVertexArrays(1, &Vao);
	glBindVertexArray(Vao);

	//vertex buffer and index buffer
	OpenGLVertexBuffer Vbo(Positions, sizeof(Positions));
	OpenGLIndexBuffer Ebo(indices, sizeof(indices));
	

	// ----- buffer layout / atrib pointer ----- ///
	{
		BufferLayout layout = {
			{  "aPos", ShaderDataType::Float3},
			{  "inColor", ShaderDataType::Float3},
			{  "inTexCoord", ShaderDataType::Float2},
		};
		Vbo.SetLayout(layout);
	}

	unsigned int index = 0;
	BufferLayout layout = Vbo.GetLayout();
	for (const auto& element : layout)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index,
			element.GetComponentCount(),
			ShaderDataTypeTOOpenGLBaseType(element.type),
			element.normalized ? GL_TRUE : GL_FALSE ,
			layout.GetStride(),
			(void*)element.offset);
		++index;
	}

	//specify the attrib pointer
	// glEnableVertexAttribArray(0);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	// glEnableVertexAttribArray(1);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(2);
	// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	// --------------------------------------------- // buff layout attrib




	InputState inputstate = {};


	// --- projection matrix --- // converts from NDC to screen space
	 glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	// ---- camera / view matrix ---- //
	Camera camera = {};
	camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
	camera.lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
	camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
	camera.right = glm::normalize(glm::cross(camera.lookAt, camera.up));
	glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.lookAt, camera.up);


	//  ----- model matrix ---- //
	 glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// model Locs
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	int viewLoc = glGetUniformLocation(shaderProgram, "view");
	int projLoc = glGetUniformLocation(shaderProgram, "projection");
	int transformLoc = glGetUniformLocation(shaderProgram, "transform");

	while (running)
	{

		Uint64 NOW = SDL_GetPerformanceCounter();
		Uint64 LAST = 0;
		LAST = NOW;

		// --- draw loop --- //
		for (unsigned int i = 0; i < 10; i++)
		{
			// model
			float angle = 20.0f * i;
			glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
			model = glm::rotate(model, (float)(SDL_GetTicks() / 500.0f) * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

			// camera
			view = glm::lookAt(camera.position, camera.position + camera.lookAt, camera.up);

			// set uniforms and draw
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		SDL_GL_SwapWindow(Window);


		NOW = SDL_GetPerformanceCounter();
		double deltaTime = 0;
		deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());


		UpdateInputState(inputstate);
		
		float velocity = 0.0025;
		positionDelta = CalculatePositionChange(camera, inputstate, deltaTime, velocity);
		camera.position += positionDelta;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	return(0);
}


