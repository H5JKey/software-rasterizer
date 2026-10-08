#include <cstdlib>
#include <iostream>
#include <print>

#include "mesh-loader.hpp"

int main(int argc, char** argv) try {
    if (argc != 2) {
        std::println("Usage: {} <mesh.obj> <output.png>", argv[0]);
        return EXIT_FAILURE;
    }
    std::filesystem::path objMeshPath = argv[1];

    Mesh m = load_obj(objMeshPath);

    std::println("Mesh {}", objMeshPath.string());
    std::println("  vertices:   {}", m.vertices.size());
    std::println("  triangles:  {}", m.indices.size() / 3);
    std::println("  normals:    {}", !m.normals.empty() ? "yes" : "no");
    std::println("  tex coords: {}", !m.texcoords.empty() ? "yes" : "no");

    

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
