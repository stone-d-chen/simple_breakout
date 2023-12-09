#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>
#include <iostream>

void GetOpenGLInfo()
{
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Lang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
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
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    GetOpenGLInfo();

    //vertex shader
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 oColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "   oColor = aColor;\n"
        "};\n";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 oColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(oColor, 1.0f);\n"
        "};\n";

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

    //create a vertex array
    unsigned int Vao;
    glGenVertexArrays(1, &Vao);
    glBindVertexArray(Vao);

    // create a vertex buffer object
    float Positions[] =
    {
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom left
        -0.5f,  0.5f, 0.0f, 0.3f, 0.5f, 0.8f   // top left 
    };

    unsigned int Vbo;
    glGenBuffers(1, &Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), Positions, GL_STATIC_DRAW);

    // create element index
    unsigned int Indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int Ebo;
    glGenBuffers(1, &Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Indices, GL_STATIC_DRAW);

    // attrib pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    SDL_Event e;
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) running = false;
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(Window);
    }
    return(0);
}