#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>
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

void GetOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
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

int main(int argc, char** args)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* Window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GLContext Context = SDL_GL_CreateContext(Window);
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); f:
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	glEnable(GL_DEPTH_TEST);

	GetOpenGLInfo();



	int width, height, nrChannels;
	unsigned char* data = stbi_load("res/textures/container.jpg", &width, &height, &nrChannels, 0);

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

	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

	//Vertex Shader
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




	// generate a vertex array buffer
	unsigned int Vao;
	glGenVertexArrays(1, &Vao);
	glBindVertexArray(Vao);

	// generate a vertex buffer
	//float Positions[] =
	//{
	//     0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
	//     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
	//    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
	//    -0.5f,  0.5f, -0.5f, 0.3f, 0.5f, 0.8f, 0.0f, 1.0f  // top left 
	//};

#include "../cube_vertices"

	OpenGLVertexBuffer Vbo(Positions, sizeof(Positions));

	//unsigned int Vbo;
	//glGenBuffers(1, &Vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, Vbo);
	////send data
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Positions), Positions, GL_STATIC_DRAW);

	//index buffer instead
	unsigned int indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	unsigned int Ebo;
	glGenBuffers(1, &Ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//specify the attrib pointer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	int interPLocation = glGetUniformLocation(shaderProgram, "interP");
	glUniform4f(interPLocation, 1.0f, 0.5f, 0.1f, 1.0f);

	// Model View Matrices
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	// model Locs
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	int viewLoc = glGetUniformLocation(shaderProgram, "view");
	int projLoc = glGetUniformLocation(shaderProgram, "projection");

	int transformLoc = glGetUniformLocation(shaderProgram, "transform");

	SDL_Event e;
	bool running = true;
	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) running = false;
		}

		//glm::mat4 trans = glm::mat4(1.0f);
		//trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		//trans = glm::rotate(trans, (float)SDL_GetTicks()/100, glm::vec3(0.0, 0.0, 1.0));
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, (float)(SDL_GetTicks() / 5000.0f) * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		SDL_GL_SwapWindow(Window);
	}
	return(0);
}