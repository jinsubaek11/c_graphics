#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "upng.h"

typedef struct
{
	vec3_t* vertices;
	face_t* faces;
	upng_t* texture;
	vec3_t rotation;
	vec3_t scale;
	vec3_t translation;
} mesh_t;

extern mesh_t mesh;

void load_mesh(
	char* obj_filename, char* png_filename,
	vec3_t scale, vec3_t translation, vec3_t rotation
);
void load_obj_file_data(mesh_t* mesh, char* obj_filename);
void load_mesh_png_data(mesh_t* mesh, char* png_filename);

int get_num_meshes();
mesh_t* get_mesh(int index);
void free_meshes();

#endif