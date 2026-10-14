#include <stdio.h>  // For printf()
#include <stdint.h> // For integer-related type aliases

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

	GUI_Label title = {0};
	title.color = DARK_BLUE;
	title.color.a = 0.0f;
	title.text = "Pizza App";
	update_text(&title.texture, &title.rect, renderer, font, title.color, title.text);

	center_rect_to(&title.rect, SCREEN);
	title.rect.y = -title.rect.h;
	title.start = (v2){{ title.rect.x, title.rect.y }};
	title.end   = (v2){ title.rect.x, SCREEN.h * 0.12};

	title.alpha_dist = (v2){{ 0.0f, 1.0f }};

	SDL_ShowWindow(window);
	
	bool32 quit = FALSE;

	uint64 now  = SDL_GetPerformanceCounter();
    uint64 last = 0;

	real64 dt = 0.0f;
	
	real64 elapsed_time = 0.0f;

	GUI_Button button = create_button(renderer, font, button_texture, "Order", DARK_BLUE);
	GUI_Button button2 = create_button(renderer, font, button_texture, "Exit", DARK_BLUE);
	button2.rect.y += button2.rect.h * 1.33;

	GUI_Button pickup_button = create_button(renderer, font, button_texture, "Pickup", DARK_BLUE);
	GUI_Button delivery_button = create_button(renderer, font, button_texture, "Delivery", DARK_BLUE);
	pickup_button.rect.x -= pickup_button.rect.w * 0.7;
	pickup_button.rect.y += pickup_button.rect.h * 0.33;
	pickup_button.color.a = 0.0f;
	pickup_button.label.color.a = 0.0f;
	
	delivery_button.rect.x += delivery_button.rect.w * 0.7;
	delivery_button.rect.y += delivery_button.rect.h * 0.33;
	delivery_button.color.a = 0.0f;
	delivery_button.label.color.a = 0.0f;
	
	center_rect_to(&button2.label.rect, button2.rect);
	
	center_rect_to(&pickup_button.label.rect, pickup_button.rect);
	center_rect_to(&delivery_button.label.rect, delivery_button.rect);

	int32 program_mode = MODE_INIT;

	real32 global_seconds = 2.0f;

	bool32 for_pickup = FALSE;

	// Global Array of Structs to store order data and display its info.
	Customer orders[50] = {0};


	// Main loop of the program
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
			if (SDL_PointInRectFloat(&cursor, &button.rect))  { brighten_color(&button.color, button.orig_color, &dt, 0.1f); }
			else                                              { reset_color(&button.color, button.orig_color, &dt, 0.1f); }

			if (SDL_PointInRectFloat(&cursor, &button2.rect)) { brighten_color(&button2.color, button2.orig_color, &dt, 0.1f); }
			else                                              { reset_color(&button2.color, button2.orig_color, &dt, 0.1f); }

			title.lerp_t += dt / global_seconds;
			
			title.rect.y  = r32_ease_in_out(title.lerp_t, title.start.y, title.end.y);
			title.color.a = r32_ease_in_out(title.lerp_t, title.alpha_dist.start, title.alpha_dist.end);
		} break;
		case MODE_ORDER_TYPE:
		{
			if (SDL_PointInRectFloat(&cursor, &pickup_button.rect))   { brighten_color(&pickup_button.color, pickup_button.orig_color, &dt, 0.1f); }
			else                                                      { reset_color(&pickup_button.color, pickup_button.orig_color, &dt, 0.1f); }

			if (SDL_PointInRectFloat(&cursor, &delivery_button.rect)) { brighten_color(&delivery_button.color, delivery_button.orig_color, &dt, 0.1f); }
			else                                                      { reset_color(&delivery_button.color, delivery_button.orig_color, &dt, 0.1f); }

			fade_button(&button,  dt / global_seconds);
			fade_button(&button2, dt / global_seconds);

			if (strcmp(title.text, "Choose Method") == 0)
			{
				fade_button(&pickup_button,  dt / global_seconds);
				fade_button(&delivery_button, dt / global_seconds);
			}
			
			title.lerp_t += dt / global_seconds;

			title.rect.y  = r32_ease_in_out(title.lerp_t, title.start.y, title.end.y);
			title.color.a = r32_ease_in_out(title.lerp_t, title.alpha_dist.start, title.alpha_dist.end);

			if (title.lerp_t >= 1.0f && strcmp(title.text, "Pizza App") == 0)
			{
				title.text = "Choose Method";
				update_text(&title.texture, &title.rect, renderer, font, title.color, title.text);
				center_rect_to(&title.rect, SCREEN);
				title.rect.y = -title.rect.h;
				
				title.lerp_t = 0.0f;
				
				title.start = (v2){{ title.rect.x, -title.rect.h }};
				title.end   = (v2){{ title.rect.x, SCREEN.h * 0.12 }};

				title.alpha_dist = (v2){{ 0.0f, 1.0f }};
			}
		} break;
		case MODE_CUSTOMER_INFO:
		{
			fade_button(&pickup_button,  dt / global_seconds);
			fade_button(&delivery_button, dt / global_seconds);
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
							reset_label_lerp_data(&title);
							global_seconds = 0.8f;
							program_mode = MODE_ORDER_TYPE;
							button.color = BLACK;
							
							reset_button_lerp_data(&button);
							reset_button_lerp_data(&button2);
						}
						else
						{
							real32 alpha = button.color.a;
							button.color = button2.orig_color;
							button.color.a = alpha;
						}
						if (SDL_PointInRectFloat(&cursor, &button2.rect))
						{
							button2.color = BLACK;
							quit = TRUE;
						}
						else
						{
							real32 alpha = button.color.a;
							button.color = button2.orig_color;
							button.color.a = alpha;
						}
					} break;
					case MODE_ORDER_TYPE:
					{
						if (SDL_PointInRectFloat(&cursor, &pickup_button.rect))
						{
							reset_button_lerp_data(&pickup_button);
							reset_button_lerp_data(&delivery_button);
							
							program_mode = MODE_CUSTOMER_INFO;
							for_pickup = TRUE;
							pickup_button.color = BLACK;
						}
						else
						{
							real32 alpha = pickup_button.color.a;
							pickup_button.color = pickup_button.orig_color;
							pickup_button.color.a = alpha;
						}
						if (SDL_PointInRectFloat(&cursor, &delivery_button.rect))
						{
							reset_button_lerp_data(&pickup_button);
							reset_button_lerp_data(&delivery_button);
							
							program_mode = MODE_CUSTOMER_INFO;
							for_pickup = FALSE;
							delivery_button.color = BLACK;
						}
						else
						{
							real32 alpha = delivery_button.color.a;
							delivery_button.color = delivery_button.orig_color;
							delivery_button.color.a = alpha;
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

		render_gui_element(renderer, &title,  TYPE_GUI_LABEL);

		render_gui_element(renderer, &button, TYPE_GUI_BUTTON);
		render_gui_element(renderer, &button2, TYPE_GUI_BUTTON);
		
		if (program_mode != MODE_INIT) { render_gui_element(renderer, &pickup_button, TYPE_GUI_BUTTON); }
		if (program_mode != MODE_INIT) { render_gui_element(renderer, &delivery_button, TYPE_GUI_BUTTON); }

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
