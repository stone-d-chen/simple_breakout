#pragma once

// GLOBALS 
bool running = true;
std::vector<QuadRenderData> RenderQueue;
std::vector<TextRenderData> TextRenderQueue;
std::vector<void*> AudioQueue;

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

float quadVertices[] = {
	 0.0f,	0.0f, /* texture flip */ 0.0f, 1.0f,
	 0.0f,	1.0f, /* texture flip */ 0.0f, 0.0f,
	 1.0f,   1.0f, /* texture flip */ 1.0f, 0.0f,
	 1.0f,   0.0f, /* texture flip */ 1.0f, 1.0f,
};
unsigned int quadElementIndices[] = {
	0,1,2,
	0,2,3
};

struct mainOGLContext {
	unsigned int Vao;
	unsigned int modelLoc;
	unsigned int colorLoc;
};

struct TextTextureInfo {
	unsigned int textureID;
	unsigned int width;
	unsigned int height;
};