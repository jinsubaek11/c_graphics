#include <math.h>
#include "clipping.h"

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov, float z_near, float z_far) {
	float cos_half_fov = cos(fov / 2);
	float sin_half_fov = sin(fov / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}


polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2) {
	polygon_t polygon = {
		.vertices = { v0, v1, v2 },
		.num_vertices = 3
	};
	return polygon;
}

void clip_polygon_against_plane(polygon_t* polygon, int plane)
{
	vec3_t plane_point = frustum_planes[plane].point;
	vec3_t plane_normal = frustum_planes[plane].normal;

	vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
	int num_inside_vertices = 0;

	vec3_t* current_vertex = &polygon->vertices[0];
	vec3_t* previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

	float current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);
	float previous_dot = vec3_dot(vec3_sub(*previous_vertex, plane_point), plane_normal);

	while (current_vertex != &polygon->vertices[polygon->num_vertices])
	{
		current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);

		if (current_dot * previous_dot < 0)
		{
			float t = previous_dot / (previous_dot - current_dot);
			vec3_t intersection_point = vec3_clone(current_vertex);
			intersection_point = vec3_sub(intersection_point, *previous_vertex);
			intersection_point = vec3_mul(intersection_point, t);
			intersection_point = vec3_add(intersection_point, *previous_vertex);

			inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
			num_inside_vertices++;
		}

		if (current_dot > 0)
		{
			inside_vertices[num_inside_vertices] = vec3_clone(current_vertex);
			num_inside_vertices++;
		}

		previous_dot = current_dot;
		previous_vertex = current_vertex;
		current_vertex++;
	}

	for (int i = 0; i < num_inside_vertices; i++)
	{
		polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
	}
	polygon->num_vertices = num_inside_vertices;
}

void clip_polygon(polygon_t* polygon) {
	clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}
