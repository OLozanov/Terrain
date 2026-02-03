#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "App.h"

#include <iostream>

namespace
{
	constexpr int width = 1280;
	constexpr int height = 720;
}

int main(int argc, char* args[])
{
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

	SDL_Window* window = SDL_CreateWindow("Terra", width, height, SDL_WINDOW_VULKAN);
	VkSurfaceKHR surface;

	SDL_SetWindowRelativeMouseMode(window, true);

	SDL_Vulkan_CreateSurface(window, Render::VulkanInstance::GetInstance(), NULL, &surface);

	App app(surface);

	unsigned long curtime = SDL_GetTicks();

	while (true)
	{
		SDL_Event event;

		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT) break;
			else
				if (event.type == SDL_EVENT_KEY_UP && event.key.key == SDLK_ESCAPE) break;

			app.input(event);
		}

		unsigned long time = SDL_GetTicks();
		float dt = (time - curtime) / 1000.0;
		curtime = time;

		app.update(dt);
		app.display();
	}

	SDL_DestroyWindow(window);

	return 0;
}