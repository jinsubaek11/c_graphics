#include <stdio.h>
#include "mesh.h"
#include "array.h"

mesh_t mesh = { NULL, NULL, {0, 0, 0} };

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
	{ 1, 2, 3 },
	{ 1, 3, 4 },

	{ 4, 3, 5 },
	{ 4, 5, 6 },

	{ 6, 5, 7 },
	{ 6, 7, 8 },

	{ 8, 7, 2 },
	{ 8, 2, 1 },

	{ 2, 7, 5 },
	{ 2, 5, 3 },

	{ 6, 8, 1 },
	{ 6, 1, 4 }
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