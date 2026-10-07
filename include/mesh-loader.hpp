#pragma once
#include <filesystem>

#include "mesh.hpp"

Mesh load_obj(const std::filesystem::path& filename);