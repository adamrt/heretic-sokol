#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "heretic.h"
#include "hmmmath.h"

#define MAX_VERTS 5000

typedef struct {
    vertex_t vertices[MAX_VERTS];
    u32 num_vertices;
} mesh_t;

bool load_obj_file_data(mesh_t* mesh, char* filename);
