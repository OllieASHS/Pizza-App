#include <stdio.h>  // For printf()
#include <stdint.h> // For integer-related type aliases

#include <SDL3/SDL.h> // For opening a window, user input, and rendering
#include <SDL3_mixer/SDL_mixer.h> // For sound effects
#include <SDL3_ttf/SDL_ttf.h> // For rendering from .ttf text format files

#ifndef HELPFUL_TOOLS_IMPLEMENTATION
#define HELPFUL_TOOLS_IMPLEMENTATION
#include "helpful_tools.h" // Includes non-main functions and macro definitions

// NOTE: this also includes the SDL libraries and stdint.h but these use macros to handle mutiple includes to just include each header file once.
// NOTE: this also allows helpful_tools.h to be included in any SDL program, I'm reincluding them at the top for readability.

#endif

#include "colors.h" // Color name macros in capital found here 

int main(void)
{
	// Initalize SDL 
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

	SDL_Texture *button_texture  = load_bmp_into_texture("../assets/Button.bmp",  "Button",  renderer);
	SDL_Texture *textbox_texture = load_bmp_into_texture("../assets/Textbox.bmp", "Textbox", renderer);

	Frame_Src textbox_src = create_frame_src(textbox_texture, 18, 18);
	
	// Main render target, we will render here so we can handle screen resizing.
	SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN.w, SCREEN.h);

	// Initalize GUI element data
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

	// Global variables, quit will tell us when to exit the program, and the rest is for storing delta time to ensure stable behavior on different frame rates.
	bool32 quit = FALSE;

	uint64 now  = SDL_GetPerformanceCounter();
    uint64 last = 0;

	real64 dt = 0.0f;
	
	real64 elapsed_time = 0.0f;

	// Get cursor image states
	SDL_Cursor *cursor_normal = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	SDL_Cursor *cursor_hover  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	SDL_Cursor *cursor_text   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);

	// Initalize more GUI elements
	GUI_Textbox textbox = create_textbox(renderer, textbox_texture, &textbox_src, (SDL_FRect){0, 0, 800, 60});
	center_rect_to(&textbox.total_rect, SCREEN);
	reposition_textbox(&textbox);
	
	GUI_Button button = create_button(renderer, font, button_texture, "Order", DARK_BLUE);
	button.clickable = TRUE;
	
	GUI_Button button2 = create_button(renderer, font, button_texture, "Exit", DARK_BLUE);
	button2.rect.y += button2.rect.h * 1.33;
	button2.clickable = TRUE;

	GUI_Button pickup_button = create_button(renderer, font, button_texture, "Pickup", DARK_BLUE);
	GUI_Button delivery_button = create_button(renderer, font, button_texture, "Delivery", DARK_BLUE);
	pickup_button.rect.x -= pickup_button.rect.w * 0.7;
	pickup_button.rect.y += pickup_button.rect.h * 0.33;
	pickup_button.color.a = 0.0f;
	pickup_button.label.color.a = 0.0f;

	// Keeping track of a global array of these GUI elements to make it easy to loop through them to handle colors and cursor states.
	#define ALL_BUTTONS 4
	GUI_Button *all_buttons[ALL_BUTTONS] =
	{
		&button,
		&button2,
		&pickup_button,
		&delivery_button
	};
	
	#define ALL_TEXTBOXES 1
	GUI_Textbox *all_textboxes[ALL_TEXTBOXES] =
	{
		&textbox
	};
	
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

		bool32 cursor_hovering_button = FALSE;
		bool32 cursor_hovering_text = FALSE;

		// Assume the cursor is not hovering over a button, then we can check all buttons to see if it is.
		for (int32 i = 0; i < ALL_BUTTONS; i++)
		{
			if (!SDL_PointInRectFloat(&cursor, &all_buttons[i]->rect)) { continue; }
			if (!all_buttons[i]->clickable) { continue; }

			SDL_SetCursor(cursor_hover);
			cursor_hovering_button = TRUE;
		}
		for (int32 i = 0; i < ALL_TEXTBOXES; i++)
		{
			if (!SDL_PointInRectFloat(&cursor, &all_textboxes[i]->total_rect)) { continue; }
			if (!all_textboxes[i]->clickable) { continue; }

			SDL_SetCursor(cursor_text);
			cursor_hovering_text = TRUE;
		}

		if (!cursor_hovering_button && !cursor_hovering_text) { SDL_SetCursor(cursor_normal); }

		// We break button logic up into the program's state to avoid strange behavior arising from buttons that are supposed to deactivate.
		switch (program_mode)
		{
		case MODE_INIT:
		{
			// Checking both buttons if they are being hovered over, then we can use linear interpolation to smooth out the color.
			
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

			// If the title has changed, we make the next set of buttons appear, this function follows an A to B transition found in .alpha_dist
			if (strcmp(title.text, "Choose Method") == 0)
			{
				fade_button(&pickup_button,  dt / global_seconds);
				fade_button(&delivery_button, dt / global_seconds);
			}
			
			title.lerp_t += dt / global_seconds;

			// Uses data from its members to decide if it is going up or down
			title.rect.y  = r32_ease_in_out(title.lerp_t, title.start.y, title.end.y);
			title.color.a = r32_ease_in_out(title.lerp_t, title.alpha_dist.start, title.alpha_dist.end);

			// Move the title up, change title, and reinitalize title data
			if (title.lerp_t >= 1.0f && strcmp(title.text, "Pizza App") == 0)
			{
				pickup_button.clickable = TRUE;
				delivery_button.clickable = TRUE;
							
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

			title.lerp_t += dt / global_seconds;

			title.rect.y  = r32_ease_in_out(title.lerp_t, title.start.y, title.end.y);
			title.color.a = r32_ease_in_out(title.lerp_t, title.alpha_dist.start, title.alpha_dist.end);

			// Same logic, we are transitioning titles, and reinitalizing title data.
			if (title.lerp_t >= 1.0f && strcmp(title.text, "Choose Method") == 0)
			{
				title.text = "Enter Info";
				update_text(&title.texture, &title.rect, renderer, font, title.color, title.text);
				center_rect_to(&title.rect, SCREEN);
				title.rect.y = -title.rect.h;
				
				title.lerp_t = 0.0f;
				
				title.start = (v2){{ title.rect.x, -title.rect.h }};
				title.end   = (v2){{ title.rect.x, SCREEN.h * 0.12 }};

				title.alpha_dist = (v2){{ 0.0f, 1.0f }};
			}
		} break;
		}

		// Use SDL to poll system level user input
		while (SDL_PollEvent(&event))
		{
			// Differentiate the types of input to the appropriate program response. 
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
			{
				printf("QUIT.\n");
				quit = TRUE;
			} break;
			case SDL_EVENT_KEY_DOWN:
			{
				if (event.key.key >= 'a' && event.key.key <= 'z')
				{
					textbox_read_input(&textbox, event.key.key);
				}
			} break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					// Break it up into program states to decide if GUI elements are worth checking to see if they have been clicked.
					switch (program_mode)
					{
					case MODE_INIT:
					{
						if (SDL_PointInRectFloat(&cursor, &button.rect) && button.clickable)
						{
							reset_label_lerp_data(&title);
							global_seconds = 0.8f;
							program_mode = MODE_ORDER_TYPE;
							button.color = BLACK;

							button.clickable = FALSE;
							button2.clickable = FALSE;
							
							reset_button_lerp_data(&button);
							reset_button_lerp_data(&button2);
						}
						else
						{
							real32 alpha = button.color.a;
							button.color = button.orig_color;
							button.color.a = alpha;
						}
						if (SDL_PointInRectFloat(&cursor, &button2.rect) && button2.clickable)
						{
							button2.color = BLACK;
							quit = TRUE;
						}
						else
						{
							real32 alpha = button2.color.a;
							button2.color = button2.orig_color;
							button2.color.a = alpha;
						}
					} break;
					case MODE_ORDER_TYPE:
					{
						if (SDL_PointInRectFloat(&cursor, &pickup_button.rect) && pickup_button.clickable)
						{
							for_pickup = TRUE;
							pickup_button.color = BLACK;
							
							reset_label_lerp_data(&title);
							global_seconds = 0.8f;
							
							reset_button_lerp_data(&pickup_button);
							reset_button_lerp_data(&delivery_button);

							pickup_button.clickable = FALSE;
							delivery_button.clickable = FALSE;
							
							program_mode = MODE_CUSTOMER_INFO;
							textbox.clickable = TRUE;
						}
						else
						{
							real32 alpha = pickup_button.color.a;
							pickup_button.color = pickup_button.orig_color;
							pickup_button.color.a = alpha;
						}
						if (SDL_PointInRectFloat(&cursor, &delivery_button.rect) && delivery_button.clickable)
						{
							for_pickup = FALSE;
							delivery_button.color = BLACK;

							reset_label_lerp_data(&title);
							global_seconds = 0.8f;

							pickup_button.clickable = FALSE;
							delivery_button.clickable = FALSE;
							
							reset_button_lerp_data(&pickup_button);
							reset_button_lerp_data(&delivery_button);

							program_mode = MODE_CUSTOMER_INFO;
							textbox.clickable = TRUE;
						}
						else
						{
							real32 alpha = delivery_button.color.a;
							delivery_button.color = delivery_button.orig_color;
							delivery_button.color.a = alpha;
						}
					} break;
					case MODE_CUSTOMER_INFO:
					{
						if (SDL_PointInRectFloat(&cursor, &textbox.total_rect) && textbox.clickable)
						{
							textbox.clicked = TRUE;
						}
					} break;
					}
				}
			} break;
			}
		}

		// Set the inital target to our canvas, our transformation from earlier will ensure it is scaled properly.
		SDL_SetRenderTarget(renderer, canvas);
		SDL_SetRenderDrawColor(renderer, 230, 230, 255, 255);
		SDL_RenderClear(renderer);

		render_gui_element(renderer, &title,  TYPE_GUI_LABEL);

		render_gui_element(renderer, &button, TYPE_GUI_BUTTON);
		render_gui_element(renderer, &button2, TYPE_GUI_BUTTON);

		// Because of the way we order states in our enum, we can check if we are "further ahead" in the process.
		
		if (program_mode > MODE_INIT) { render_gui_element(renderer, &pickup_button, TYPE_GUI_BUTTON); }
		if (program_mode > MODE_INIT) { render_gui_element(renderer, &delivery_button, TYPE_GUI_BUTTON); }

		if (program_mode > MODE_ORDER_TYPE)
		{
			render_gui_element(renderer, &textbox, TYPE_GUI_TEXTBOX);
			if (textbox.clicked)
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(renderer, &textbox.pointer);
				SDL_SetRenderDrawColor(renderer, 230, 230, 255, 255);
			}
		}

		// Change the render target back to our screen display
		SDL_SetRenderTarget(renderer, NULL);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// Render the entire screen from our canvas
		SDL_RenderTexture(renderer, canvas, NULL, &canvas_rect);

		SDL_RenderPresent(renderer);
	}

	// Cleanup function to free used memory, anything declared on the stack.
	// e.g. anything NOT using malloc() or calloc() or ISN'T allocated by SDL, which is heap allocation, is cleaned up for us.
		
	SDL_DestroyTexture(title.texture);
	TTF_CloseFont(font);

	cleanup_sdl(renderer, window);
	
	return 0;
}
