#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

// RGBA8, top-left origin
struct ImageData
{
    uint32_t             width = 0;
    uint32_t             height = 0;
    std::vector<uint8_t> pixels;
};

bool LoadImageFile(const std::filesystem::path& file, ImageData& out);
