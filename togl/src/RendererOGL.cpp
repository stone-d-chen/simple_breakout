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

SDL_Window* initWindowing(const char* WindowName, int Width, int Height)
{
	SDL_Window* Window = initSDLOpenGLWindow(WindowName, Width, Height);
	initSDLOpenGLBinding();

	return Window;
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

unsigned int CreateVao(float* Vertices, size_t VertexArraySize, unsigned int* Indices, size_t IndexArraySize)
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
TextTextureInfo CreateTextTexture(std::string text, TTF_Font* font)
{
	SDL_Surface* surfaceText = TTF_RenderUTF8_Blended(font, text.c_str(), { 255, 255, 255 });
	SDL_Surface* surfaceRGBA = SDL_ConvertSurfaceFormat(surfaceText, SDL_PIXELFORMAT_ABGR8888, 0);
	TextTextureInfo fonttexture = CreateTextTexture(surfaceRGBA, GL_RGBA);
	SDL_FreeSurface(surfaceRGBA);
	SDL_FreeSurface(surfaceText);

	return fonttexture;
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
void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition,
	float rotation, const glm::vec4 Color, unsigned int texture,
	mainOGLContext context)
{
	DrawQuad(pixelDimensions, pixelPosition, rotation, Color, texture, context.Vao, context.modelLoc, context.colorLoc);
}

void DebugDrawString(glm::vec2 pos, std::string text, TTF_Font* font,
	mainOGLContext context)
{
	TextTextureInfo fonttexture = CreateTextTexture(text, font);
	DrawQuad(
		{ fonttexture.width / 3, fonttexture.height / 3 },
		pos,
		0,
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		fonttexture.textureID,
		context);
	glDeleteTextures(1, &fonttexture.textureID);
}

void Clear(glm::vec4 color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}