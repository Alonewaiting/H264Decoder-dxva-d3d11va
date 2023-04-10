/*!
 * \file D3D11RenderData.h
 *
 * \author dwy
 * \date 2023/1/2
 *
 * 
 */
#pragma once
#include <cstdint>
#include <directxmath.h>
struct Size2d
{
    uint64_t m_width;
    uint64_t m_height;
};

struct VertexPos {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex0;
};