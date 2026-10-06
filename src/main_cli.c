// Пример работы с утилитами шаблона без окна:
// загружает меш, печатает его размеры, загружает текстуру и сохраняет её
// с переставленными местами красным и синим каналами.

#include "image_utils.h"
#include "mesh_utils.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *obj_path = argc > 1 ? argv[1] : "resources/Bunny.obj";
    const char *tex_path = argc > 2 ? argv[2] : "resources/textures/porcelain.jpg";
    const char *out_path = argc > 3 ? argv[3] : "swapped_rb.png";

    if (argc > 4)
    {
        printf("Usage: %s [mesh.obj] [texture.png|jpg] [output.png]\n", argv[0]);
        return 1;
    }

    mesh m = load_obj(obj_path);
    if (m.num_triangles == 0)
    {
        printf("Failed to load mesh: %s\n", obj_path);
        free_mesh(&m);
        return 1;
    }

    printf("Mesh %s\n", obj_path);
    printf("  vertices:   %d\n", m.num_vertices);
    printf("  triangles:  %d\n", m.num_triangles);
    printf("  normals:    %s\n", m.normals ? "yes" : "no");
    printf("  tex coords: %s\n", m.tcs ? "yes" : "no");
    free_mesh(&m);

    int w = 0, h = 0;
    float *img = load_image_f32_rgb(tex_path, &w, &h, 1.0f);
    if (!img)
    {
        printf("Failed to load image: %s\n", tex_path);
        return 1;
    }

    printf("Image %s: %d x %d\n", tex_path, w, h);

    for (int i = 0; i < w * h; i++)
    {
        const float r = img[3 * i + 0];
        img[3 * i + 0] = img[3 * i + 2];
        img[3 * i + 2] = r;
    }

    const int res = save_image_f32_png_rgb(img, out_path, w, h, 3, 1.0f);
    free(img);
    if (!res)
    {
        printf("Failed to save image: %s\n", out_path);
        return 1;
    }

    printf("Saved image with swapped R and B channels to %s\n", out_path);
    return 0;
}
