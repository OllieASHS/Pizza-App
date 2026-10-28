#ifndef HELPFUL_TOOLS_H
#define HELPFUL_TOOLS_H

#include <stdint.h> // For integer-related type aliases
#include <stdlib.h> // For malloc() and calloc()
#include <string.h> // For strlen()

#include <SDL3/SDL.h> // For opening a window
#include <SDL3_mixer/SDL_mixer.h> // For sound effects
#include <SDL3_ttf/SDL_ttf.h> // For text rendering

// Helpful redefinitions of type-aliases
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32 bool32;

typedef float  real32;
typedef double real64;

#define TRUE  1
#define FALSE 0

#define PI32 3.141592653589793f

// I am developing with a 1920x1080 screen, passing these values directly is ok because we will resize the screen later.
#define SCREEN (SDL_FRect){0, 0, 1920, 1080}

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

// Enum to differentiate pointers passed as void *, as we cannot dereference void pointers. This is to keep all rendering in one function.
enum GUI_ELEMENTS
{
	TYPE_GUI_LABEL,
	TYPE_GUI_BUTTON,
	TYPE_GUI_TEXTBOX
};

// For differentiating what the program is doing and what GUI elements need to be displayed
enum MODE
{
	MODE_INIT,
	MODE_ORDER_TYPE,
	MODE_CUSTOMER_INFO,
	MODE_MENU,
	MODE_ALL_ORDERS
};

// For simple 2D data, union is used such that x = start and y = end, so I can use both names.
typedef union
{
	struct
	{
		real32 x, y;
	};
	struct
	{
		real32 start, end;
	};
} v2;

// Storing frame data from an entire texture, a.k.a. a sprite sheet.
typedef struct
{
	SDL_FRect *frames;
	int32 total_frames;
} Frame_Src;

// Struct to hold GUI element data in.
typedef struct
{
	SDL_Texture *texture;
	char *text;
	
	SDL_FRect rect;
	SDL_FColor orig_color;
	SDL_FColor color;

	v2 start;
	v2 end;
	v2 alpha_dist;

	real64 lerp_t;
} GUI_Label;

// Struct to hold GUI element data in.
typedef struct
{
	SDL_Texture *texture;
	GUI_Label label;
	SDL_FRect rect;
	SDL_FColor orig_color;
	SDL_FColor color;

	v2 start;
	v2 end;
	v2 alpha_dist;

	real64 lerp_t;
	
	bool32 clickable;
} GUI_Button;

// Struct to hold GUI element data in.
typedef struct
{
	SDL_Texture *texture;
	Frame_Src *frame_src;

	GUI_Label label;
	GUI_Label input_label;

	// These are for scaling textboxes without having to create multiple asset files.
	SDL_FRect corners[4];
	SDL_FRect horiz_sides[2];
	SDL_FRect vert_sides[2];
	SDL_FRect center;

	SDL_FRect total_rect;
	
	SDL_FRect pointer;
	SDL_FColor color;

	v2 alpha_dist;

	real64 lerp_t;
	real64 pointer_switch_time;

	int32 pointer_pos;

	bool32 pointer_visible;
	bool32 clickable;
	bool32 clicked;
} GUI_Textbox;

// Struct to hold data for different pizza types
typedef struct
{
	SDL_Texture *texture;

	GUI_Label name_label;
	GUI_Label price_label;
	GUI_Label description_label;
	
	SDL_FRect rect;
	
	real32 price;
} Pizza;

// Struct to hold data for all orders done in a session.
typedef struct
{
	char *name;
	char *address;
	int32 phone_number;

	bool32 for_pickup;
} Customer;

// Function declarations

static inline real32 calculate_frames(SDL_Texture *texture, uint32 frame_width, uint32 frame_height);

static inline Frame_Src create_frame_src(SDL_Texture *texture, int32 frame_width, int32 frame_height);

static inline void fade_button(GUI_Button *button, real32 value);

static inline void reset_element_lerp_data(void *element, int32 type);

static inline void reset_label_lerp_data(GUI_Label *label);

static inline void update_text(SDL_Texture **texture, SDL_FRect *rect, SDL_Renderer *renderer, TTF_Font *font, SDL_FColor color, char *text);

static inline void center_rect_to(SDL_FRect *target, SDL_FRect anchor);

static inline void calculate_delta_time(real64 *dt, uint64 *now, uint64 *last);

static inline void render_gui_element(SDL_Renderer *renderer, void* element, int32 type);

static inline int32 r32_round(real32 num);

static inline real32 r32_lerp(real32 a, real32 b, real32 t);

static inline real64 r64_lerp(real64 a, real64 b, real64 t);

static inline real32 r32_ease_in_out(real64 t, real32 start, real32 end);

static inline void cleanup_sdl(SDL_Renderer *renderer, SDL_Window *window);

static inline SDL_FRect screen_to_window_visuals(SDL_Renderer *renderer, SDL_FPoint *cursor);

static inline SDL_Texture *load_bmp_into_texture(char *path, char *name, SDL_Renderer *renderer);

static inline void brighten_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds);

static inline void darken_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds);

static inline void reset_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds);

static inline GUI_Button create_button(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *texture, char *text, SDL_FColor color);

static inline GUI_Textbox create_textbox(char *title, SDL_FColor color, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *texture, Frame_Src *src, SDL_FRect total_area);

static inline void reposition_textbox(GUI_Textbox *textbox, SDL_Renderer *renderer, TTF_Font *font);

static inline void textbox_read_input(SDL_Renderer *renderer, TTF_Font *font, GUI_Textbox *textbox, char key);

#ifdef HELPFUL_TOOLS_IMPLEMENTATION

// Function definitions

// Calculates the amount of frames in a sprite sheet, goes from top to bottom, left to right.
static inline real32 calculate_frames(SDL_Texture *texture, uint32 frame_width, uint32 frame_height)
{
	real32 w, h;
	SDL_GetTextureSize(texture, &w, &h);

	w = (uint32)w;
	h = (uint32)h;
	
	uint32 frames_x = w / frame_width;
    uint32 frames_y = h / frame_height;
	uint32 total_frames = frames_x * frames_y;

	return total_frames;
}

// Allocates memory for the frame data
static inline Frame_Src create_frame_src(SDL_Texture *texture, int32 frame_width, int32 frame_height)
{
	Frame_Src frame_data = {0};
	
    int32 offset_x = 0;
	int32 offset_y = 0;

	real32 w, h;
	SDL_GetTextureSize(texture, &w, &h);

	frame_data.total_frames = calculate_frames(texture, frame_width, frame_height);

	frame_data.frames = calloc(frame_data.total_frames, sizeof(SDL_FRect));
	
	for (int32 i = 0; i < frame_data.total_frames; i++)
	{		
		if (offset_x + frame_width > w)
		{
			offset_x = 0;
			offset_y += frame_height;
		}
		frame_data.frames[i] = (SDL_FRect){offset_x, offset_y, frame_width, frame_height};
		offset_x += frame_width;
	}

	return frame_data;
}

// Moved to its own inline function to keep verbosity low.
static inline void fade_button(GUI_Button *button, real32 value)
{
	button->lerp_t += value;
	button->label.lerp_t += value;
	button->color.a       = r32_ease_in_out(button->lerp_t, button->alpha_dist.start, button->alpha_dist.end);
	button->label.color.a = r32_ease_in_out(button->label.lerp_t, button->label.alpha_dist.start, button->label.alpha_dist.end);
}

// Moved to its own inline function to keep verbosity low.
static inline void reset_element_lerp_data(void *element, int32 type)
{
	switch (type)
	{
	case TYPE_GUI_BUTTON:
	{
		GUI_Button *button = (GUI_Button *)element;
		
		button->alpha_dist = (v2){{ 1.0f, 0.0f }};
		button->label.alpha_dist = (v2){{ 1.0f, 0.0f }};
		button->lerp_t = 0.0f;
		button->label.lerp_t = 0.0f;
	} break;
	case TYPE_GUI_TEXTBOX:
	{
		GUI_Textbox *textbox = (GUI_Textbox *)element;

		textbox->lerp_t = 0.0f;
		textbox->alpha_dist = (v2){{ 1.0f, 0.0f }};
		
		textbox->label.lerp_t = 0.0f;
		textbox->label.alpha_dist = (v2){{ 1.0f, 0.0f }};
		
		textbox->input_label.lerp_t = 0.0f;
		textbox->input_label.alpha_dist = (v2){{ 1.0f, 0.0f }};
	} break;
	}
}

// Moved to its own inline function to keep verbosity low.
static inline void reset_label_lerp_data(GUI_Label *label)
{
	label->end = label->start;
	label->start = (v2){{ label->rect.x, label->rect.y }};
	label->lerp_t = 0.0f;
	label->alpha_dist = (v2){{ 1.0f, 0.0f }};
}

// Wrapper function which uses SDL's structs and a target string and color and returns the texture to be drawn.
void update_text(SDL_Texture **texture, SDL_FRect *rect, SDL_Renderer *renderer, TTF_Font *font, SDL_FColor color, char *text)
{
	SDL_Color int_color =
	{
		(uint8)(color.r * 255),
		(uint8)(color.g * 255),
		(uint8)(color.b * 255),
		(uint8)(color.a * 255)
	};
	
	SDL_Surface *surface = TTF_RenderText_Blended(font, text, 0, int_color);

	if (*texture) { SDL_DestroyTexture(*texture); }
	*texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_GetTextureSize(*texture, &rect->w, &rect->h);
	
	SDL_DestroySurface(surface);	
}

// Used to center GUI elements to positions defined by an SDL_FRect.
static inline void center_rect_to(SDL_FRect *target, SDL_FRect anchor)
{
	target->x = anchor.x + (anchor.w / 2) - (target->w / 2);
	target->y = anchor.y + (anchor.h / 2) - (target->h / 2);
}

// Calculates delta time, self explanatory. Used to prevent problems with different frame rates.
static inline void calculate_delta_time(real64 *dt, uint64 *now, uint64 *last)
{
	*last = *now;
	*now  = SDL_GetPerformanceCounter();

	*dt = (real64)((*now - *last) / (real64)SDL_GetPerformanceFrequency());
}

// Wrapper function for parseing GUI_Label struct data.
static inline void render_gui_element(SDL_Renderer *renderer, void* element, int32 type)
{
	switch (type)
	{
	case TYPE_GUI_LABEL:
	{
		GUI_Label *text = (GUI_Label *)element;

		// Modulating textures means any other GUI elements will be modulated too. We first modulate here.
		SDL_SetTextureColorModFloat(text->texture, text->color.r, text->color.g, text->color.b);
		SDL_SetTextureAlphaModFloat(text->texture, text->color.a);
		
		SDL_RenderTexture(renderer, text->texture, NULL, &text->rect);

		// Then we reset the modulation here as we have already rendered our target GUI element.
		SDL_SetTextureColorModFloat(text->texture, 1.0f, 1.0f, 1.0f);
		SDL_SetTextureAlphaModFloat(text->texture, 1.0f);
	} break;
	case TYPE_GUI_BUTTON:
	{
		GUI_Button *button = (GUI_Button *)element;

		// Same modulation logic here
		SDL_SetTextureColorModFloat(button->texture, button->color.r, button->color.g, button->color.b);
		SDL_SetTextureAlphaModFloat(button->texture, button->color.a);
		
		SDL_RenderTexture(renderer, button->texture, NULL, &button->rect);

		SDL_SetTextureColorModFloat(button->texture, 1.0f, 1.0f, 1.0f);
		SDL_SetTextureAlphaModFloat(button->texture, 1.0f);

		// Same modulation logic here
		SDL_SetTextureColorModFloat(button->label.texture, button->label.color.r, button->label.color.g, button->label.color.b);
		SDL_SetTextureAlphaModFloat(button->label.texture, button->label.color.a);
		
		SDL_RenderTexture(renderer, button->label.texture, NULL, &button->label.rect);
		
		SDL_SetTextureColorModFloat(button->label.texture, 1.0f, 1.0f, 1.0f);
		SDL_SetTextureAlphaModFloat(button->label.texture, 1.0f);
	} break;
	case TYPE_GUI_TEXTBOX:
	{
		GUI_Textbox *textbox = (GUI_Textbox *)element;

		// Render all seperate 9 pieces to give an appearance of smooth scaling.
		SDL_SetTextureAlphaModFloat(textbox->texture, textbox->color.a);
		
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[0], &textbox->corners[0]);
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[2], &textbox->corners[1]);
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[6], &textbox->corners[2]);
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[8], &textbox->corners[3]);

		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[1], &textbox->horiz_sides[0]);
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[7], &textbox->horiz_sides[1]);

		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[3], &textbox->vert_sides[0]);
		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[5], &textbox->vert_sides[1]);

		SDL_RenderTexture(renderer, textbox->texture, &textbox->frame_src->frames[4], &textbox->center);

		SDL_SetTextureAlphaModFloat(textbox->texture, 1.0f);

		// Render the input in the textbox
		SDL_SetTextureColorModFloat(textbox->input_label.texture, textbox->input_label.color.r, textbox->input_label.color.g, textbox->input_label.color.b);
		SDL_SetTextureAlphaModFloat(textbox->input_label.texture, textbox->input_label.color.a);
		
		SDL_RenderTexture(renderer, textbox->input_label.texture, NULL, &textbox->input_label.rect);

		SDL_SetTextureColorModFloat(textbox->input_label.texture, 1.0f, 1.0f, 1.0f);
		SDL_SetTextureAlphaModFloat(textbox->input_label.texture, 1.0f);

		// Render the textbox titile, we care about color + alpha modulation here thus the functions from before.
		SDL_SetTextureColorModFloat(textbox->label.texture, textbox->label.color.r, textbox->label.color.g, textbox->label.color.b);
		SDL_SetTextureAlphaModFloat(textbox->label.texture, textbox->label.color.a);
		
		SDL_RenderTexture(renderer, textbox->label.texture, NULL, &textbox->label.rect);
		

		// Then we reset the modulation here as we have already rendered our target GUI element.
		SDL_SetTextureColorModFloat(textbox->label.texture, 1.0f, 1.0f, 1.0f);
		SDL_SetTextureAlphaModFloat(textbox->label.texture, 1.0f);

		// Render the pointer
		if (textbox->clicked && textbox->pointer_visible)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &textbox->pointer);
			SDL_SetRenderDrawColor(renderer, 230, 230, 255, 255);
		}
	} break;
	}
}

// Rounding decimal numbers to the nearest integer.
static inline int32 r32_round(real32 num)
{
	return (int32)(num + 0.5f);
}

// Used for easing and smooth effects, 32 bit floating point variant.
static inline real32 r32_lerp(real32 a, real32 b, real32 t)
{
	if (t > 1.0f) { t = 1.0f; }
	if (t < 0.0f) { t = 0.0f; }
	
    return a + t * (b - a);
}

// Used for easing and smooth effects, 64 bit floating point variant.
static inline real64 r64_lerp(real64 a, real64 b, real64 t)
{
	if (t > 1.0f) { t = 1.0f; }
	if (t < 0.0f) { t = 0.0f; }
	
    return a + t * (b - a);
}

// Specific smoothing effect for slow-to-fast movement while also decelerating. 
static inline real32 r32_ease_in_out(real64 t, real32 start, real32 end)
{
	if (t > 1.0f) { t = 1.0f; }
			
	real32 eased_t = t * t * (3.0f - 2.0f * t);
			
	return r32_lerp(start, end, eased_t);
}

// Cleans up memory SDL has allocated.
static inline void cleanup_sdl(SDL_Renderer *renderer, SDL_Window *window)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	MIX_Quit();
	SDL_Quit();
}

// Implements overscan presentation to resize the screen, we also get the cursor's position based on this calculation.
static inline SDL_FRect screen_to_window_visuals(SDL_Renderer *renderer, SDL_FPoint *cursor)
{
	int32 window_w, window_h;
	SDL_GetRenderOutputSize(renderer, &window_w, &window_h);
		
	real32 canvas_scale_x = (real32)window_w / SCREEN.w;
	real32 canvas_scale_y = (real32)window_h / SCREEN.h;

	real32 canvas_scale = (canvas_scale_x > canvas_scale_y) ? canvas_scale_x : canvas_scale_y;

	real32 dest_w = SCREEN.w * canvas_scale;
	real32 dest_h = SCREEN.h * canvas_scale;

	SDL_FRect canvas_rect = {0};
	canvas_rect.w = dest_w;
	canvas_rect.h = dest_h; 
	canvas_rect.x = (window_w - dest_w) / 2.0f;
	canvas_rect.y = (window_h - dest_h) / 2.0f;

	SDL_GetMouseState(&cursor->x, &cursor->y);
	*cursor = (SDL_FPoint){ (cursor->x - canvas_rect.x) / canvas_scale, (cursor->y - canvas_rect.y) / canvas_scale };

	return canvas_rect;
}

// Loads a bitmap file into an SDL_Texture, which is hardware accelerated, SDL_LoadBMP handles all possible bitmap formats. 
static inline SDL_Texture *load_bmp_into_texture(char *path, char *name, SDL_Renderer *renderer)
{
	if (!renderer)
	{
		printf("ERROR: Renderer is invalid.\n");
		return NULL;
	}
	
	SDL_Surface *surface = SDL_LoadBMP(path);
	if (!surface)
	{
		printf("ERROR: %s's bitmap file not found.\n", name);
		return NULL;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);	
	if (!texture)
	{
		printf("ERROR: Failed to create texture from %s's bitmap file.\n", name);
		return NULL;
	}
	
	SDL_DestroySurface(surface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	return texture;
}

// Used for GUI elements such as buttons to give a smooth feel.
static inline void brighten_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds)
{
	real64 d_time = *dt / seconds;
	
	real32 d_red   = r32_lerp(color->r, orig_color.r * 1.5, d_time);
	real32 d_green = r32_lerp(color->g, orig_color.g * 1.5, d_time);
	real32 d_blue  = r32_lerp(color->b, orig_color.b * 1.5, d_time);

	if (d_red > 1.0f)   { d_red   = 1.0f; }
	if (d_green > 1.0f) { d_green = 1.0f; }
	if (d_blue > 1.0f)  { d_blue  = 1.0f; }
	
	color->r = d_red;
	color->g = d_green;
	color->b = d_blue;
}

// Same operations as brighten_color() but darkens the color, obviously.
static inline void darken_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds)
{
	real64 d_time = *dt / seconds;
	
	real32 d_red   = r32_lerp(color->r, orig_color.r * 0.5, d_time);
	real32 d_green = r32_lerp(color->g, orig_color.g * 0.5, d_time);
	real32 d_blue  = r32_lerp(color->b, orig_color.b * 0.5, d_time);

	if (d_red > 1.0f)   { d_red   = 1.0f; }
	if (d_green > 1.0f) { d_green = 1.0f; }
	if (d_blue > 1.0f)  { d_blue  = 1.0f; }
	
	color->r = d_red;
	color->g = d_green;
	color->b = d_blue;
}

// Same operations as brighten_color() but changes the color to it's original values, obviously.
static inline void reset_color(SDL_FColor *color, SDL_FColor orig_color, real64 *dt, real64 seconds)
{
	real64 d_time = *dt / seconds;
	
	real32 d_red   = r32_lerp(color->r, orig_color.r, d_time);
	real32 d_green = r32_lerp(color->g, orig_color.g, d_time);
	real32 d_blue  = r32_lerp(color->b, orig_color.b, d_time);

	if (d_red > 1.0f)   { d_red   = 1.0f; }
	if (d_green > 1.0f) { d_green = 1.0f; }
	if (d_blue > 1.0f)  { d_blue  = 1.0f; }
	
	color->r = d_red;
	color->g = d_green;
	color->b = d_blue;
}

// Repeated filling out of values for GUI_Button has it's own function.
static inline GUI_Button create_button(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *texture, char *text, SDL_FColor color)
{
	GUI_Button button = {0};
	
	button.texture = texture;
	button.rect = (SDL_FRect){0, 0, 369, 166};
	button.orig_color = color;
	button.color = color;
	button.clickable = FALSE;
	center_rect_to(&button.rect, SCREEN);
	button.label.color = (SDL_FColor){1.0f, 1.0f, 1.0f, 1.0f};
	button.label.text = text;
	update_text(&button.label.texture, &button.label.rect, renderer, font, button.label.color, text);
	button.alpha_dist       = (v2){{ 0.0f, 1.0f }};
	button.label.alpha_dist = (v2){{ 0.0f, 1.0f }};
	center_rect_to(&button.label.rect, button.rect);

	return button;
}

// Repeated filling out of values for GUI_Button has it's own function, using 9-splicing to create a textbox of virtually any size.
static inline GUI_Textbox create_textbox(char *title, SDL_FColor color, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *texture, Frame_Src *src, SDL_FRect total_area)
{
	real32 horiz_length = total_area.w - 36.0f;
	real32 vert_height = total_area.h - 36.0f;
	
	GUI_Textbox textbox =
	{
		.frame_src = src,

		.total_rect = total_area,

		.corners[0] = (SDL_FRect){total_area.x, total_area.y, 18.0f, 18.0f},
		.corners[1] = (SDL_FRect){total_area.x + horiz_length + 18.0f, total_area.y, 18.0f, 18.0f},
		.corners[2] = (SDL_FRect){total_area.x, total_area.y + vert_height + 18.0f, 18.0f, 18.0f},
		.corners[3] = (SDL_FRect){total_area.x + horiz_length + 18.0f, total_area.y + vert_height + 18.0f, 18.0f, 18.0f},
		
		.horiz_sides[0] = (SDL_FRect){textbox.corners[0].x + textbox.corners[0].w, textbox.corners[0].y, horiz_length, 18.0f},
		.horiz_sides[1] = (SDL_FRect){textbox.corners[2].x + textbox.corners[2].w, textbox.corners[2].y, horiz_length, 18.0f},

		.vert_sides[0] = (SDL_FRect){textbox.corners[0].x, textbox.corners[0].y + textbox.corners[0].h, 18.0f, vert_height},
		.vert_sides[1] = (SDL_FRect){textbox.corners[1].x, textbox.corners[1].y + textbox.corners[1].h, 18.0f, vert_height},
		
		.center = (SDL_FRect){textbox.corners[0].x + textbox.corners[0].w, textbox.corners[0].y + textbox.corners[0].h, horiz_length, vert_height},

		.texture = texture,
		.alpha_dist = (v2){{ 0.0f, 1.0f }},

		.pointer.w = 4.0f,
		.pointer.h = 40.0f,

		.pointer.x = total_area.x,
		.pointer.y = total_area.y,

		// Set bounds for text size to be in a range of 0-40, 41 here includes the null terminator,
		// which tells C when a string ends.
		
		.input_label.text = calloc(41, sizeof(char)),
		.pointer_pos = 0,

		.input_label.rect.x = total_area.x,
		.input_label.rect.y = total_area.y,

		.label.text = title,
		.label.color = color,
	    .label.orig_color = color
	};

	update_text(&textbox.label.texture, &textbox.label.rect, renderer, font, color, textbox.label.text);
	textbox.label.rect.x = total_area.x;
	textbox.label.rect.y = total_area.y - textbox.label.rect.w;

	textbox.input_label.alpha_dist = (v2){{ 0.0f, 1.0f }};
	textbox.input_label.color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};

	return textbox;
}

static inline void reposition_textbox(GUI_Textbox *textbox, SDL_Renderer *renderer, TTF_Font *font)
{
	real32 horiz_length = textbox->total_rect.w - 36.0f;
	real32 vert_height = textbox->total_rect.h - 36.0f;
	
	textbox->corners[0] = (SDL_FRect){textbox->total_rect.x, textbox->total_rect.y, 18.0f, 18.0f};
	textbox->corners[1] = (SDL_FRect){textbox->total_rect.x + horiz_length + 18.0f, textbox->total_rect.y, 18.0f, 18.0f};
	textbox->corners[2] = (SDL_FRect){textbox->total_rect.x, textbox->total_rect.y + vert_height + 18.0f, 18.0f, 18.0f};
	textbox->corners[3] = (SDL_FRect){textbox->total_rect.x + horiz_length + 18.0f, textbox->total_rect.y + vert_height + 18.0f, 18.0f, 18.0f};
		
	textbox->horiz_sides[0] = (SDL_FRect){textbox->corners[0].x + textbox->corners[0].w, textbox->corners[0].y, horiz_length, 18.0f};
	textbox->horiz_sides[1] = (SDL_FRect){textbox->corners[2].x + textbox->corners[2].w, textbox->corners[2].y, horiz_length, 18.0f};

	textbox->vert_sides[0] = (SDL_FRect){textbox->corners[0].x, textbox->corners[0].y + textbox->corners[0].h, 18.0f, vert_height};
	textbox->vert_sides[1] = (SDL_FRect){textbox->corners[1].x, textbox->corners[1].y + textbox->corners[1].h, 18.0f, vert_height};
		
	textbox->center = (SDL_FRect){textbox->corners[0].x + textbox->corners[0].w, textbox->corners[0].y + textbox->corners[0].h, horiz_length, vert_height};

	textbox->pointer.w = 4.0f;
	textbox->pointer.h = 40.0f;

	textbox->pointer.x = textbox->total_rect.x + (4.0 * textbox->pointer.w);
	textbox->pointer.y = textbox->total_rect.y + (0.25 * textbox->pointer.h);

	textbox->input_label.rect.x = textbox->pointer.x;
	textbox->input_label.rect.y = textbox->pointer.y;

	textbox->label.rect.x = textbox->total_rect.x;
	textbox->label.rect.y = textbox->total_rect.y - textbox->label.rect.h;

	update_text(&textbox->label.texture, &textbox->label.rect, renderer, font, (SDL_FColor){1.0f, 1.0f, 1.0f, 1.0f}, textbox->label.text);
}

static inline void textbox_read_input(SDL_Renderer *renderer, TTF_Font *font, GUI_Textbox *textbox, char key)
{
	if (!textbox || !textbox->clicked) { return; }

	if (key != SDLK_BACKSPACE)
	{
		// Keep text array index within bounds
		if (textbox->pointer_pos >= 40)
		{
			return;
		}
		
		textbox->input_label.text[textbox->pointer_pos] = key;
		textbox->pointer_pos++;

		real32 min = textbox->input_label.rect.x;
		real32 max = textbox->input_label.rect.x + textbox->input_label.rect.w;
	}
	else
	{
		textbox->pointer_pos--;

		// Keep text array index within bounds
		if (textbox->pointer_pos < 0) { textbox->pointer_pos = 0; }
		
		textbox->input_label.text[textbox->pointer_pos] = '\0';
	}

	int32 char_width, char_height;
	TTF_GetStringSize(font, "a", 1, &char_width, &char_height);
		
	textbox->pointer.x = (real32)char_width * textbox->pointer_pos + textbox->input_label.rect.x;

	if (textbox->label.texture) { SDL_DestroyTexture(textbox->input_label.texture); }

	update_text(&textbox->input_label.texture, &textbox->input_label.rect, renderer, font, (SDL_FColor){0, 0, 0, 1.0f}, textbox->input_label.text);
}

#endif // HELPFUL_TOOLS_IMPLEMENTATION

#endif // HELPFUL_TOOLS_H
