#pragma once

#include "heretic.h"

vertex_t cube_vertices[] = {
    // positions            texcoords       normals
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f,}, { 0.0f,  0.0f, -1.0f,}},
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f,}, { 0.0f,  0.0f, -1.0f,}},
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f,}},
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f,}},
    {{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f,}},
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f,}, { 0.0f,  0.0f, -1.0f,}},

    {{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f,}, { 0.0f,  0.0f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f,}, { 0.0f,  0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 1.0f,}, { 0.0f,  0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 1.0f,}, { 0.0f,  0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f,}, { 0.0f,  0.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f,}, { 0.0f,  0.0f, 1.0f}},

    {{-0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},

    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},

    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},

    {{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{-0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f,}, { 0.0f,  1.0f,  0.0f}},
};
