#pragma once
#include <windows.h>
#include <cstdio>

inline bool Check(HRESULT hr, const char* what)
{
    if (FAILED(hr))
    {
        std::fprintf(stderr, "[D3D11] %s failed (0x%08X)\n", what, static_cast<unsigned>(hr));
        return false;
    }
    return true;
}