#pragma once
#include "Vertex.h"
#include <cstdint>
#include <filesystem>
#include <vector>

struct MeshData
{
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
};

bool LoadObj(const std::filesystem::path& file, MeshData& out);
