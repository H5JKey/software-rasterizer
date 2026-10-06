#ifndef MESH_UTILS_H
#define MESH_UTILS_H
#include "vectors.h"

// Треугольный меш. Все атрибуты индексируются одним индексом вершины:
// verts[i], normals[i], tcs[i]. Треугольник t - вершины indices[3*t+0..2].
//
// Нормали и текстурные координаты не генерируются: если в файле их нет (или они
// заданы не для всех вершин), normals / tcs равны NULL.
typedef struct
{
    int num_vertices;
    int num_triangles;
    vec3 *verts;
    vec3 *normals;
    vec2 *tcs;
    int *indices;
} mesh;

void free_mesh(mesh *m);
// При ошибке возвращает пустой меш (num_vertices = num_triangles = 0)
mesh load_obj(const char *filename);

#endif
