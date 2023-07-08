#pragma once

#include "defines.h"
#include "mesh.h"

vertex_t cube_vertices[] = {
    {{-0.1f, -0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 0.0f, 0.0f}, 0.0f},
    {{ 0.1f, -0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 1.0f, 0.0f}, 0.0f},
    {{ 0.1f,  0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 1.0f, 1.0f}, 0.0f},
    {{ 0.1f,  0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 1.0f, 1.0f}, 0.0f},
    {{-0.1f,  0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 0.0f, 1.0f}, 0.0f},
    {{-0.1f, -0.1f, -0.1f}, { 0.0f,  0.0f, -1.0f}, { 0.0f, 0.0f}, 0.0f},

    {{-0.1f, -0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  0.0f, 0.0f}, 0.0f},
    {{ 0.1f, -0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  1.0f, 0.0f}, 0.0f},
    {{ 0.1f,  0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  1.0f, 1.0f}, 0.0f},
    {{ 0.1f,  0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  1.0f, 1.0f}, 0.0f},
    {{-0.1f,  0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  0.0f, 1.0f}, 0.0f},
    {{-0.1f, -0.1f,  0.1f}, { 0.0f,  0.0f, 1.0f}, {  0.0f, 0.0f}, 0.0f},

    {{-0.1f,  0.1f,  0.1f}, {-1.0f,  0.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{-0.1f,  0.1f, -0.1f}, {-1.0f,  0.0f,  0.0f}, { 1.0f, 1.0f}, 0.0f},
    {{-0.1f, -0.1f, -0.1f}, {-1.0f,  0.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{-0.1f, -0.1f, -0.1f}, {-1.0f,  0.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{-0.1f, -0.1f,  0.1f}, {-1.0f,  0.0f,  0.0f}, { 0.0f, 0.0f}, 0.0f},
    {{-0.1f,  0.1f,  0.1f}, {-1.0f,  0.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},

    {{ 0.1f,  0.1f,  0.1f}, { 1.0f,  0.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{ 0.1f,  0.1f, -0.1f}, { 1.0f,  0.0f,  0.0f}, { 1.0f, 1.0f}, 0.0f},
    {{ 0.1f, -0.1f, -0.1f}, { 1.0f,  0.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{ 0.1f, -0.1f, -0.1f}, { 1.0f,  0.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{ 0.1f, -0.1f,  0.1f}, { 1.0f,  0.0f,  0.0f}, { 0.0f, 0.0f}, 0.0f},
    {{ 0.1f,  0.1f,  0.1f}, { 1.0f,  0.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},

    {{-0.1f, -0.1f, -0.1f}, { 0.0f, -1.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{ 0.1f, -0.1f, -0.1f}, { 0.0f, -1.0f,  0.0f}, { 1.0f, 1.0f}, 0.0f},
    {{ 0.1f, -0.1f,  0.1f}, { 0.0f, -1.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{ 0.1f, -0.1f,  0.1f}, { 0.0f, -1.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{-0.1f, -0.1f,  0.1f}, { 0.0f, -1.0f,  0.0f}, { 0.0f, 0.0f}, 0.0f},
    {{-0.1f, -0.1f, -0.1f}, { 0.0f, -1.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},

    {{-0.1f,  0.1f, -0.1f}, { 0.0f,  1.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
    {{ 0.1f,  0.1f, -0.1f}, { 0.0f,  1.0f,  0.0f}, { 1.0f, 1.0f}, 0.0f},
    {{ 0.1f,  0.1f,  0.1f}, { 0.0f,  1.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{ 0.1f,  0.1f,  0.1f}, { 0.0f,  1.0f,  0.0f}, { 1.0f, 0.0f}, 0.0f},
    {{-0.1f,  0.1f,  0.1f}, { 0.0f,  1.0f,  0.0f}, { 0.0f, 0.0f}, 0.0f},
    {{-0.1f,  0.1f, -0.1f}, { 0.0f,  1.0f,  0.0f}, { 0.0f, 1.0f}, 0.0f},
};
