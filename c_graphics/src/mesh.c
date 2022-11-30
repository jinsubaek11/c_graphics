#include <stdio.h>
#include <string.h>
#include "mesh.h"
#include "array.h"

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(
	char* obj_filename, char* png_filename,
	vec3_t scale, vec3_t translation, vec3_t rotation
)
{
	load_obj_file_data(&meshes[mesh_count], obj_filename);
	load_mesh_png_data(&meshes[mesh_count], png_filename);

	meshes[mesh_count].scale = scale;
	meshes[mesh_count].translation = translation;
	meshes[mesh_count].rotation = rotation;

	mesh_count++;
}

void load_obj_file_data(mesh_t* mesh, char* obj_filename)
{
	FILE* file;
	fopen_s(&file, obj_filename, "r");

	char line[1024];

	tex2_t* texCoords = NULL;

	while (fgets(line, 1024, file))
	{
		// vertex information
		if (strncmp(line, "v ", 2) == 0)
		{
			vec3_t vertex;
			sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			array_push(mesh->vertices, vertex);
		}
		
		if (strncmp(line, "vt ", 3) == 0)
		{
			tex2_t texCoord;
			sscanf(line, "vt %f %f", &texCoord.u, &texCoord.v);
			array_push(texCoords, texCoord);
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
				vertex_indices[0] - 1, 
				vertex_indices[1] - 1, 
				vertex_indices[2] - 1,
				texCoords[texture_indices[0] - 1],
				texCoords[texture_indices[1] - 1],
				texCoords[texture_indices[2] - 1],
				0xffffffff
			};
		
			array_push(mesh->faces, face);
		}
	}
}

void load_mesh_png_data(mesh_t* mesh, char* png_filename)
{
	upng_t* png_image = upng_new_from_file(png_filename);
	if (png_image != NULL)
	{
		upng_decode(png_image);
		if (upng_get_error(png_image) == UPNG_EOK)
		{
			//mesh_texture = (uint32_t*)upng_get_buffer(png_texture);
			//texture_width = upng_get_width(png_texture);
			//texture_height = upng_get_height(png_texture);

			mesh->texture = png_image;
		}
	}
}

int get_num_meshes()
{
	return mesh_count;
}

mesh_t* get_mesh(int index)
{
	return &meshes[index];
}

void free_meshes()
{
	for (int i = 0; i < mesh_count; i++)
	{
		upng_free(meshes[i].texture);
		array_free(meshes[i].faces);
		array_free(meshes[i].vertices);
	}
}