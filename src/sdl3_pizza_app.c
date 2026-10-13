#include <stdio.h>  // For printf()
#include <stdlib.h>
#include <stdint.h> // For integer-related type aliases
#include <math.h>

#include <SDL3/SDL.h> // For opening a window
#include <SDL3_mixer/SDL_mixer.h> // For sound effects
#include <SDL3_ttf/SDL_ttf.h> // For text rendering

#ifndef HELPFUL_TOOLS_IMPLEMENTATION
#define HELPFUL_TOOLS_IMPLEMENTATION
#include "helpful_tools.h" // Includes non-main functions and macro definitions

// NOTE: this also includes the SDL libraries and stdint.h but these use macros to handle mutiple includes to just include each header file once.
// NOTE: this also allows helpful_tools.h to be included in any SDL program, I'm reincluding them at the top for readability.

#endif

#include "colors.h" // Color name macros in capital found here 

// For differentiating what the program is doing and what GUI elements need to be displayed
enum MODE
{
	MODE_INIT,
	MODE_ORDER_TYPE,
	MODE_CUSTOMER_INFO,
	MODE_MENU,
	MODE_ALL_ORDERS
};

int main(void)
{
	SDL_Renderer *renderer = {0};
	SDL_Window   *window   = {0};
	SDL_Event    event     = {0};
	
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		printf("Failed to init SDL.\n");
		return 0;
	}

	TTF_Init();
	MIX_Init();
	
	TTF_Font *font = TTF_OpenFont("../fonts/Text.ttf", 80);

	window = SDL_CreateWindow("Pizza App", 1920, 1080, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		printf("Failed to create window.\n");
		return 0;
	}

	renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer)
	{
		printf("Failed to create renderer.\n");
		return 0;
	}

	SDL_Texture *button_texture = load_bmp_into_texture("../assets/Button.bmp", "Button", renderer);
	
	SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN.w, SCREEN.h);

	GUI_Text title = {0};
	title.color = DARK_BLUE;
	title.color.a = 0;

	title.real_alpha = 0.0f;
	title.texture = update_text(renderer, font, title.color, "Pizza App");

	SDL_GetTextureSize(title.texture, &title.rect.w, &title.rect.h);
	center_rect_to(&title.rect, SCREEN);

	title.rect.y = -title.rect.h;

	SDL_ShowWindow(window);
	
	bool32 quit = FALSE;

	uint64 now  = SDL_GetPerformanceCounter();
    uint64 last = 0;

	real64 dt = 0.0f;
	
	real64 elapsed_time = 0.0f;

	GUI_Button button = create_button(renderer, font, button_texture, "Order", DARK_BLUE);

	GUI_Button button2 = create_button(renderer, font, button_texture, "Exit", DARK_BLUE);
	button2.rect.y += button2.rect.h * 1.33;
	center_rect_to(&button2.text.rect, button2.rect);

	int32 program_mode = MODE_INIT;
	
	while (!quit)
	{
		calculate_delta_time(&dt, &now, &last);
		elapsed_time += dt;

		SDL_FPoint cursor = {0};
		SDL_FRect canvas_rect = screen_to_window_visuals(renderer, &cursor);
		
		switch (program_mode)
		{
		case MODE_INIT:
		{
			if (SDL_PointInRectFloat(&cursor, &button.rect)) { brighten_color(&button.color, button.orig_color, &dt, 0.1f); }
			else                                             { reset_color(&button.color, button.orig_color, &dt, 0.1f); }

			if (SDL_PointInRectFloat(&cursor, &button2.rect)) { brighten_color(&button2.color, button2.orig_color, &dt, 0.1f); }
			else                                              { reset_color(&button2.color, button2.orig_color, &dt, 0.1f); }

			title.real_alpha = r64_lerp(title.real_alpha, 255.0f, dt / 2);
			title.color.a = (uint8)title.real_alpha;

			title.rect.y = r32_lerp(title.rect.y, SCREEN.h * 0.12, dt);

			SDL_SetTextureAlphaMod(title.texture, title.color.a);
		} break;
		case MODE_ORDER_TYPE:
		{
			button.color.a = r32_lerp(button.color.a, 0.0f, dt * 5);
			button.text.color.a = r32_lerp(button.text.color.a, 0.0f, dt * 5);

			button2.color.a = r32_lerp(button2.color.a, 0.0f, dt * 5);
			button2.text.color.a = r32_lerp(button2.text.color.a, 0.0f, dt * 5);

			title.real_alpha = r64_lerp(title.real_alpha, 0.0f, dt / 2);
			title.color.a = (uint8)title.real_alpha;

			title.rect.y = r32_lerp(title.rect.y, -title.rect.h, dt);

			SDL_SetTextureAlphaMod(title.texture, title.color.a);
		} break;
		}
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
			{
				printf("QUIT.\n");
				quit = TRUE;
			} break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					switch (program_mode)
					{
					case MODE_INIT:
					{
						if (SDL_PointInRectFloat(&cursor, &button.rect))
						{
							program_mode = MODE_ORDER_TYPE;
							button.color = BLACK;
						}
						else
						{
							button.color = button2.orig_color;
						}
						if (SDL_PointInRectFloat(&cursor, &button2.rect))
						{
							button2.color = BLACK;
							quit = TRUE;
						}
						else
						{
							button.color = button2.orig_color;
						}
					} break;
					}
				}
			} break;
			}
		}

		SDL_SetRenderTarget(renderer, canvas);
		SDL_SetRenderDrawColor(renderer, 230, 230, 255, 255);
		SDL_RenderClear(renderer);

		render_gui_element(renderer, &title,  TYPE_GUI_TEXT);
		render_gui_element(renderer, &button, TYPE_GUI_BUTTON);
		render_gui_element(renderer, &button2, TYPE_GUI_BUTTON);

		SDL_SetRenderTarget(renderer, NULL);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_RenderTexture(renderer, canvas, NULL, &canvas_rect);
		
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(title.texture);
	TTF_CloseFont(font);

	cleanup_sdl(renderer, window);
	
	return 0;
}
