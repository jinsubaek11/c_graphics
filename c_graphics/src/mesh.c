#include <stdio.h>
#include <string.h>
#include "mesh.h"
#include "array.h"

mesh_t mesh = { NULL, NULL, {0, 0, 0}, {1.0, 1.0, 1.0}, {0, 0, 0} };
 
vec3_t cube_vertices[N_CUBE_VERTICES] = {
	{ -1, -1, -1 },
	{ -1,  1, -1 },
	{  1,  1, -1 },
	{  1, -1, -1 },
	{  1,  1,  1 },
	{  1, -1,  1 },
	{ -1,  1,  1 },
	{ -1, -1,  1 },
};

face_t cube_faces[N_CUBE_FACES] = {
	{ 1, 2, 3, 0xffffffff },
	{ 1, 3, 4, 0xffffffff },

	{ 4, 3, 5, 0xffffffff },
	{ 4, 5, 6, 0xffffffff },

	{ 6, 5, 7, 0xffffffff },
	{ 6, 7, 8, 0xffffffff },
		
	{ 8, 7, 2, 0xffffffff },
	{ 8, 2, 1, 0xffffffff },
	
	{ 2, 7, 5, 0xffffffff },
	{ 2, 5, 3, 0xffffffff },

	{ 6, 8, 1, 0xffffffff },
	{ 6, 1, 4, 0xffffffff }
};

void load_cube_mesh_data()
{
	for (int i = 0; i < N_CUBE_VERTICES; i++)
	{
		vec3_t cube_vertex = cube_vertices[i];
		array_push(mesh.vertices, cube_vertex);
	}

	for (int i = 0; i < N_CUBE_FACES; i++)
	{
		face_t cube_face = cube_faces[i];
		array_push(mesh.faces, cube_face);
	}
}

void load_obj_file_data(char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "r");

	char line[1024];

	while (fgets(line, 1024, file))
	{
		// vertex information
		if (strncmp(line, "v ", 2) == 0)
		{
			vec3_t vertex;
			sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			array_push(mesh.vertices, vertex);
		}
		
		// face information
		if (strncmp(line, "f ", 2) == 0)
		{
			int vertex_indices[3];
			int texture_indices[3];
			int normal_indices[3];

			sscanf(
				line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&vertex_indices[0], &texture_indices[0], &normal_indices[0],
				&vertex_indices[1], &texture_indices[1], &normal_indices[1],
				&vertex_indices[2], &texture_indices[2], &normal_indices[2]
			);

			face_t face = { 
				vertex_indices[0], 
				vertex_indices[1], 
				vertex_indices[2],
				0xffffffff
			};
		
			array_push(mesh.faces, face);
		}
	}
}