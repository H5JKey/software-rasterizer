#include "mesh-loader.hpp"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyobj_loader_c.h"
#ifdef _WIN64
#define atoll(S) _atoi64(S)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#endif

static char* mmap_file(size_t* len, const char* filename) {
#ifdef _WIN64
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (file == INVALID_HANDLE_VALUE) { /* E.g. Model may not have materials. */
        return NULL;
    }

    HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    assert(fileMapping != INVALID_HANDLE_VALUE);

    LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
    char* fileMapViewChar = (char*)fileMapView;
    assert(fileMapView != NULL);

    DWORD file_size = GetFileSize(file, NULL);
    (*len) = (size_t)file_size;

    return fileMapViewChar;
#else

    struct stat sb;
    char* p;
    int fd;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        return NULL;
    }

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "%s is not a file\n", filename);
        return NULL;
    }

    p = (char*)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    if (close(fd) == -1) {
        perror("close");
        return NULL;
    }

    (*len) = sb.st_size;

    return p;

#endif
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data,
                          size_t* len) {
    // NOTE: If you allocate the buffer with malloc(),
    // You can define your own memory management struct and pass it through `ctx`
    // to store the pointer and free memories at clean up stage(when you quit an
    // app)
    // This example uses mmap(), so no free() required.
    (void)ctx;

    if (!filename) {
        fprintf(stderr, "null filename\n");
        (*data) = NULL;
        (*len) = 0;
        return;
    }

    size_t data_len = 0;

    *data = mmap_file(&data_len, filename);
    (*len) = data_len;
}

typedef struct {
    int v_idx, vt_idx, vn_idx;
    int id;  // -1 - empty
} vertex_slot;

static uint32_t hash_vertex(int v_idx, int vt_idx, int vn_idx) {
    uint32_t h = (uint32_t)v_idx * 73856093u;
    h ^= (uint32_t)vt_idx * 19349663u;
    h ^= (uint32_t)vn_idx * 83492791u;
    return h;
}

static int valid_index(int idx, unsigned int count) { return idx >= 0 && (unsigned int)idx < count; }

Mesh load_obj(const std::filesystem::path& filename) {
    Mesh m;
    m.vertices.clear();
    m.normals.clear();
    m.texcoords.clear();
    m.indices.clear();

    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, filename.c_str(),
                                get_file_data, NULL, flags);
    if (ret != TINYOBJ_SUCCESS) return m;

    const int num_corners = attrib.num_faces;

    int has_normals = attrib.num_normals > 0;
    int has_texcoords = attrib.num_texcoords > 0;
    int valid = num_corners % 3 == 0;
    for (int i = 0; i < num_corners && valid; i++) {
        valid = valid_index(attrib.faces[i].v_idx, attrib.num_vertices);
        has_normals = has_normals && valid_index(attrib.faces[i].vn_idx, attrib.num_normals);
        has_texcoords = has_texcoords && valid_index(attrib.faces[i].vt_idx, attrib.num_texcoords);
    }

    if (valid) {
        m.vertices.reserve(num_corners);
        if (has_normals) m.normals.reserve(num_corners);
        if (has_texcoords) m.texcoords.reserve(num_corners);
        m.indices.resize(num_corners);

        uint32_t table_size = 1;
        while (table_size < 2 * (uint32_t)num_corners) table_size *= 2;
        vertex_slot* table = new vertex_slot[table_size];
        for (uint32_t i = 0; i < table_size; i++) table[i].id = -1;

        for (int i = 0; i < num_corners; i++) {
            const int v_idx = attrib.faces[i].v_idx;
            const int vt_idx = has_texcoords ? attrib.faces[i].vt_idx : -1;
            const int vn_idx = has_normals ? attrib.faces[i].vn_idx : -1;

            uint32_t slot = hash_vertex(v_idx, vt_idx, vn_idx) & (table_size - 1);
            while (table[slot].id >= 0 &&
                   !(table[slot].v_idx == v_idx && table[slot].vt_idx == vt_idx && table[slot].vn_idx == vn_idx))
                slot = (slot + 1) & (table_size - 1);

            if (table[slot].id < 0) {
                const uint32_t id = static_cast<uint32_t>(m.vertices.size());

                table[slot].v_idx = v_idx;
                table[slot].vt_idx = vt_idx;
                table[slot].vn_idx = vn_idx;
                table[slot].id = static_cast<int>(id);

                m.vertices.emplace_back(attrib.vertices[3 * v_idx + 0], attrib.vertices[3 * v_idx + 1],
                                        attrib.vertices[3 * v_idx + 2]);
                if (has_normals)
                    m.normals.emplace_back(attrib.normals[3 * vn_idx + 0], attrib.normals[3 * vn_idx + 1],
                                           attrib.normals[3 * vn_idx + 2]);
                if (has_texcoords)
                    m.texcoords.emplace_back(attrib.texcoords[2 * vt_idx + 0], attrib.texcoords[2 * vt_idx + 1]);
            }

            m.indices[i] = table[slot].id;
        }

        delete[] (table);
    } else {
        std::cerr << std::format("load_obj: {} has invalid face indices", filename.string()) << std::endl;
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
    return m;
}
