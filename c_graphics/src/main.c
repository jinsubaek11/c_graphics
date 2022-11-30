#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "display.h"
#include "camera.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "clipping.h"
#include "upng.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

vec3_t camera_position = { 0, 0, -3 };

float fov_factor = 640;

bool is_running;
int previous_frame_time = 0;
float delta_time = 0;

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

void setup()
{ 
	set_render_method(RENDER_WIRE);
	set_cull_method(CULL_BACKFACE);

	init_light(vec3_new(0, 0, 1));
	init_camera((vec3_t) { 0, 0, 0 }, (vec3_t) { 0, 0, 1 });

	float aspect_x = (float)get_window_width() / (float)get_window_height(6);
	float aspect_y = (float)get_window_height() / (float)get_window_width();
	float fov_y = M_PI / 3.0f;
	float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2.0;
	float z_near = 0.1;
	float z_far = 100.0;
	proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

	init_frustum_planes(fov_x, fov_y, z_near, z_far);

	//mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
	//texture_width = 64;
	//texture_height = 64;

	load_mesh(
		"./assets/f117.obj",
		"./assets/f117.png",
		vec3_new(1, 1, 1),
		vec3_new(-3, 0, 8),
		vec3_new(0, 0, 0)
	);

	load_mesh(
		"./assets/f22.obj",
		"./assets/f22.png",
		vec3_new(1, 1, 1),
		vec3_new(3, 0, 8),
		vec3_new(0, 0, 0)
	);
}

void process_input()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				is_running = false;
				break;
			}
			else if (event.key.keysym.sym == SDLK_1)
			{
				set_render_method(RENDER_WIRE_VERTEX);
				break;
			}
			else if (event.key.keysym.sym == SDLK_2)
			{
				set_render_method(RENDER_WIRE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_3)
			{
				set_render_method(RENDER_FILL_TRIANGLE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_4)
			{
				set_render_method(RENDER_FILL_TRIANGLE_WIRE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_5)
			{
				set_render_method(RENDER_TEXTURED);
				break;
			}
			else if (event.key.keysym.sym == SDLK_6)
			{
				set_render_method(RENDER_TEXTURED_WIRE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_c)
			{
				set_cull_method(CULL_BACKFACE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_x)
			{
				set_cull_method(CULL_NONE);
				break;
			}
			else if (event.key.keysym.sym == SDLK_UP)
			{
				vec3_t position = get_camera_position();
				position.y += 3.0 * delta_time;
				update_camera_position(position);
				break;
			}
			else if (event.key.keysym.sym == SDLK_DOWN)
			{
				vec3_t position = get_camera_position();
				position.y += -3.0 * delta_time;
				update_camera_position(position);
				break;
			}
			else if (event.key.keysym.sym == SDLK_a)
			{
				rotate_camera_yaw(1.0 * delta_time);
				break;
			}
			else if (event.key.keysym.sym == SDLK_d)
			{
				rotate_camera_yaw(-1.0 * delta_time);
				break;
			}
			else if (event.key.keysym.sym == SDLK_w)
			{

				vec3_t forward_velocity = vec3_mul(get_camera_direction(), 1.0 * delta_time);
				vec3_t position = vec3_add(get_camera_position(), forward_velocity);
				update_camera_forward_velocity(forward_velocity);
				update_camera_position(position);
				break;
			}
			else if (event.key.keysym.sym == SDLK_s)
			{
				vec3_t forward_velocity = vec3_mul(get_camera_direction(), 1.0 * delta_time);
				vec3_t position = vec3_sub(get_camera_position(), forward_velocity);
				update_camera_forward_velocity(forward_velocity);
				update_camera_position(position);
				break;
			}
			break;
		}
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

	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;
	previous_frame_time = SDL_GetTicks();

	num_triangles_to_render = 0;

	for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
	{
		mesh_t* mesh = get_mesh(mesh_index);

		mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
		mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
		mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
		mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
		mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

		//vec3_t target = { 0, 0, 1 };
		vec3_t target = get_camera_lookat_target();
		vec3_t up_direction = vec3_new(0, 1, 0);
		view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

		int num_faces = array_length(mesh->faces);
		for (int i = 0; i < num_faces; i++)
		{
			face_t mesh_face = mesh->faces[i];

			vec3_t face_vertices[3];
			face_vertices[0] = mesh->vertices[mesh_face.a];
			face_vertices[1] = mesh->vertices[mesh_face.b];
			face_vertices[2] = mesh->vertices[mesh_face.c];

			vec4_t transformed_vertices[3];

			for (int j = 0; j < 3; j++)
			{
				vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

				world_matrix = mat4_identity();
				world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
				world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
				world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
				world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
				world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

				transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
				transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);


				transformed_vertices[j] = transformed_vertex;
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

			vec3_t origin = { 0, 0, 0 };
			vec3_t camera_ray = vec3_sub(origin, vector_a);
			//vec3_normalize(&camera_ray);

			float dot_normal_camera = vec3_dot(normal, camera_ray);

			if (is_cull_backface())
			{
				if (dot_normal_camera < 0)
				{
					continue;
				}
			}

			polygon_t polygon = create_polygon_from_triangle(
				vec3_from_vec4(transformed_vertices[0]),
				vec3_from_vec4(transformed_vertices[1]),
				vec3_from_vec4(transformed_vertices[2]),
				mesh_face.a_uv,
				mesh_face.b_uv,
				mesh_face.c_uv
			);

			clip_polygon(&polygon);

			triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
			int num_triangles_after_clipping = 0;

			triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

			for (int t = 0; t < num_triangles_after_clipping; t++)
			{
				triangle_t triangle_after_clipping = triangles_after_clipping[t];

				vec4_t projected_points[3];

				for (int j = 0; j < 3; j++)
				{
					projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);
					//projected_points[j] = project(transformed_vertices[j].x, transformed_vertices[j].y, transformed_vertices[j].z);

					projected_points[j].x *= (get_window_width() / 2.0);
					projected_points[j].y *= (get_window_height() / 2.0);

					projected_points[j].y *= -1;

					projected_points[j].x += (get_window_width() / 2.0);
					projected_points[j].y += (get_window_height() / 2.0);

					//printf("%f, %f, %f", projected_points[j].x, projected_points[j].y, projected_points[j].z);
				}

				float light_intensity_factor = -vec3_dot(normal, get_light_direction());
				uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

				triangle_t triangle_to_render = {
					.points = {
						{projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
						{projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
						{projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
					},
					.texcoords = {
						{ triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
						{ triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
						{ triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
					},
					.color = triangle_color,
					.texture = mesh->texture
				};

				if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
				{
					triangles_to_render[num_triangles_to_render++] = triangle_to_render;
				}
			}
		}
	}
}

void render()
{
	clear_color_buffer(0xff000000);
	clear_z_buffer();

	draw_grid();

	for (int i = 0; i < num_triangles_to_render; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		
		if (should_render_filled_triangles())
		{
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
				triangle.color
			);
		}

		if (should_render_textured_triangles())
		{
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
				triangle.texture
			);
		}

		if (should_render_wireframe())
		{
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				0xffffffff
			);
		}

		if (should_render_wire_vertex())
		{
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xffff0000);
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xffff0000);
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xffff0000);
		}
	}
	

	//draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xff00ff00);

	render_color_buffer();
}

void free_resources()
{
	free_meshes();
	destroy_window();
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

	free_resources();

	return 0;
}