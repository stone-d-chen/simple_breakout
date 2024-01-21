#pragma once

// GLOBALS 
std::vector<QuadRenderData> RenderQueue;
std::vector<TextRenderData> TextRenderQueue;
std::vector<void*> AudioQueue;

struct TextTextureInfo {
	unsigned int textureID;
	unsigned int width;
	unsigned int height;
};