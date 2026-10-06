#pragma once
#include <filesystem>

#include "mesh.hpp"

// При ошибке возвращает пустой меш
Mesh load_obj(const std::filesystem::path& filename);