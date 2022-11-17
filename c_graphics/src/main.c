#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "upng.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, -3 };

float fov_factor = 640;

bool is_running;
int previous_frame_time = 0;

mat4_t proj_matrix;

void setup()
{
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

	if (!color_buffer)
	{
		fprintf(stderr, "Allocation was not successful");
	}

	color_buffer_texture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height
	);

	float fov = M_PI / 3.0f;
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

	//mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
	texture_width = 64;
	texture_height = 64;

	//load_cube_mesh_data();
	//load_obj_file_data("./assets/f22.obj");
	load_obj_file_data("./assets/efa.obj");
	load_png_texture_data("./assets/efa.png");
}

void process_input()
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
		{
			is_running = false;
		}
		else if (event.key.keysym.sym == SDLK_1)
		{
			render_method = RENDER_WIRE_VERTEX;
		}
		else if (event.key.keysym.sym == SDLK_2)
		{
			render_method = RENDER_WIRE;
		}
		else if (event.key.keysym.sym == SDLK_3)
		{
			render_method = RENDER_FILL_TRIANGLE;
		}
		else if (event.key.keysym.sym == SDLK_4)
		{
			render_method = RENDER_FILL_TRIANGLE_WIRE;
		}
		else if (event.key.keysym.sym == SDLK_5)
		{
			render_method = RENDER_TEXTURED;
		}
		else if (event.key.keysym.sym == SDLK_6)
		{
			render_method = RENDER_TEXTURED_WIRE;
		}
		else if (event.key.keysym.sym == SDLK_c)
		{
			cull_method = CULL_BACKFACE;
		}
		else if (event.key.keysym.sym == SDLK_d)
		{
			cull_method = CULL_NONE;
		}
		break;
	}
}

vec3_t project(float x, float y, float z)
{
	vec3_t projected_point = {
		fov_factor * x / z, fov_factor * y / z
	};

	return projected_point;
}

void update()
{
	while (
		!SDL_TICKS_PASSED(SDL_GetTicks(),
			previous_frame_time + FRAME_TARGET_TIME)
		);

	previous_frame_time = SDL_GetTicks();

	triangles_to_render = NULL;

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.01;

	mesh.translation.z = 5.0;

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a];
		face_vertices[1] = mesh.vertices[mesh_face.b];
		face_vertices[2] = mesh.vertices[mesh_face.c];

		vec4_t transformed_vertices[3];

		for (int j = 0; j < 3; j++)
		{
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
			
			mat4_t world_matrix = mat4_identity();
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			transformed_vertices[j] = mat4_mul_vec4(world_matrix, transformed_vertex);
		}

		vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
		vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
		vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

		//vec3_t vector_ab = vec3_sub(vector_b, vector_a);
		//vec3_t vector_ac = vec3_sub(vector_c, vector_a);
		vec3_t vector_ab = vec3_sub(vector_a, vector_b);
		vec3_t vector_ac = vec3_sub(vector_a, vector_c);
		vec3_normalize(&vector_ab);
		vec3_normalize(&vector_ac);

		vec3_t normal = vec3_cross(vector_ab, vector_ac);
		vec3_normalize(&normal);

		//vec3_t origin = { 0,0,0 };
		vec3_t camera_ray = vec3_sub(camera_position, vector_a);
		//vec3_normalize(&camera_ray);

		float dot_normal_camera = vec3_dot(normal, camera_ray);

		if (cull_method == CULL_BACKFACE)
		{
			if (dot_normal_camera < 0)
			{
				continue;
			}
		}

		vec4_t projected_points[3];

		for (int j = 0; j < 3; j++) 
		{
			projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);
			//projected_points[j] = project(transformed_vertices[j].x, transformed_vertices[j].y, transformed_vertices[j].z);
			
			projected_points[j].x *= (window_width / 2.0);
			projected_points[j].y *= (window_height / 2.0);

			projected_points[j].y *= -1;

			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);

			//printf("%f, %f, %f", projected_points[j].x, projected_points[j].y, projected_points[j].z);
		}

		float avg_depth = (transformed_vertices[0].z + 
			transformed_vertices[1].z + transformed_vertices[2].z) / 3;

		float light_intensity_factor = -vec3_dot(normal, light.direction);
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

		triangle_t projected_triangle = {
			.points = {
				{projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
				{projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
				{projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
			},
			.texcoords = {
				{mesh_face.a_uv.u, mesh_face.a_uv.v},
				{mesh_face.b_uv.u, mesh_face.b_uv.v},
				{mesh_face.c_uv.u, mesh_face.c_uv.v},
			},
			.color = triangle_color,
			.avg_depth = avg_depth
		};

		//triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}

	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++)
	{
		for (int j = i; j < num_triangles; j++)
		{
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
			{
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}

void render()
{
	draw_grid();
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				triangle.color
			);
		}

		if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
		{
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
				mesh_texture
			);
		}

		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || 
			render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE)
		{
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				0xffffffff
			);
		}

		if (render_method == RENDER_WIRE_VERTEX)
		{
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xffff0000);
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xffff0000);
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xffff0000);
		}
	}
	

	//draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xff00ff00);

	array_free(triangles_to_render);

	render_color_buffer();
	clear_color_buffer(0xff000000);

	SDL_RenderPresent(renderer);
}

void free_resources()
{
	free(color_buffer);
	upng_free(png_texture);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char* args[])
{
	is_running = initialize_window();

	setup();

	while (is_running)
	{
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();

	return 0;
}