#include <SDL_mixer.h>
void initSDLMixerAudio()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf(SDL_GetError());
	}
	Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);
	return;
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
