#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t* color_buffer = NULL;
static float* z_buffer = NULL;
static SDL_Texture* color_buffer_texture = NULL;

static int window_width = 800;
static int window_height = 600;

static int render_method = 0;
static int cull_method = 0;

int get_window_width(void)
{
	return window_width;
}

int get_window_height(void)
{
	return window_height;
}

bool initialize_window()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	//window_width = display_mode.w;
	//window_height = display_mode.h;

	window = SDL_CreateWindow(
		"window title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_BORDERLESS
	);

	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}
	
	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	if (!color_buffer)
	{
		fprintf(stderr, "Allocation was not successful");
	}

	color_buffer_texture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height
	);

	return true;
}

void set_render_method(int method)
{
	render_method = method;
}

void set_cull_method(int method)
{
	cull_method = method;
}

bool is_cull_backface()
{
	return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangles()
{
	return (
		render_method == RENDER_FILL_TRIANGLE ||
		render_method == RENDER_FILL_TRIANGLE_WIRE
	);
}

bool should_render_textured_triangles()
{
	return (
		render_method == RENDER_TEXTURED ||
		render_method == RENDER_TEXTURED_WIRE
	);
}

bool should_render_wireframe()
{
	return (
		render_method == RENDER_WIRE || 
		render_method == RENDER_WIRE_VERTEX ||
		render_method == RENDER_FILL_TRIANGLE_WIRE || 
		render_method == RENDER_TEXTURED_WIRE
	);
}

bool should_render_wire_vertex()
{
	return (
		render_method == RENDER_WIRE_VERTEX
	);
}

void draw_grid()
{
	for (int y = 0; y < window_height; y++)
	{
		for (int x = 0; x < window_width; x++)
		{
			if (y % 10 == 0 || x % 10 == 0)
			{
				color_buffer[window_width * y + x] = 0xff333333;
			}
		}
	}
}

void draw_pixel(int x, int y, uint32_t color)
{
	if (x < 0 || y < 0 || x >= window_width || y >= window_height)
	{
		return;
	}

	color_buffer[window_width * y + x] = color;
}


void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	int deltaX = x1 - x0;
	int deltaY = y1 - y0;

	int side_length = abs(deltaX) >= abs(deltaY) ? abs(deltaX) : abs(deltaY);

	float inc_x = deltaX / (float)side_length;
	float inc_y = deltaY / (float)side_length;

	float current_x = x0;
	float current_y = y0;

	for (int i = 0; i <= side_length; i++)
	{
		draw_pixel(round(current_x), round(current_y), color);
		current_x += inc_x;
		current_y += inc_y;
	}
}


void draw_rect(int x, int y, int width, int height, uint32_t color)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int current_x = x + i;
			int current_y = y + j;
			draw_pixel(current_x, current_y, color);
		}
	}
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
}

void clear_color_buffer(uint32_t color)
{
	for (int i = 0; i < window_width * window_height; i++)
	{
		color_buffer[i] = color;
	}
}

void clear_z_buffer()
{
	for (int i = 0; i < window_width * window_height; i++)
	{
		z_buffer[i] = 1.0;
	}
}

void render_color_buffer()
{
	SDL_UpdateTexture(
		color_buffer_texture, NULL, color_buffer,
		(int)(window_width * sizeof(uint32_t))
	);

	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

float get_zbuffer_at(int x, int y)
{
	if (x < 0 || y < 0 || x >= window_width || y >= window_height)
	{
		return 1.0;
	}

	return z_buffer[(window_width * y) + x];
}

void update_zbuffer_at(int x, int y, float value)
{
	if (x < 0 || y < 0 || x >= window_width || y >= window_height)
	{
		return;
	}

	z_buffer[(window_width * y) + x] = value;
}

void destroy_window()
{
	free(color_buffer);
	free(z_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}