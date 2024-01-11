//
// Created by W-Mai on 2024/1/11.
//

#ifndef METAL_CMAKE_EXAMPLE_RENDER_TYPES_H
#define METAL_CMAKE_EXAMPLE_RENDER_TYPES_H

#include "simd/simd.h"

typedef enum {
    VertexInputIndexVertices = 0,
    VertexInputIndexViewportSize = 1,
} VertexInputIndex;

struct Vertex {
    vector_float2 position;
    vector_float4 color;
};

#endif //METAL_CMAKE_EXAMPLE_RENDER_TYPES_H
