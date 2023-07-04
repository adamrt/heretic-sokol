#include "mesh.h"

bool load_obj_file_data(mesh_t* mesh, char* filename)
{
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("error");
        return false;
    }
    char line[1024];

    vec3_t temp_vertices[MAX_VERTS];
    vec3_t temp_normals[MAX_VERTS];
    vec2_t temp_uvs[MAX_VERTS];
    u32 temp_num_vertices;
    u32 temp_num_normals;
    u32 temp_num_uvs;

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            vec3_t v;
            sscanf(line, "v %f %f %f", &v.X, &v.Y, &v.Z);
            temp_vertices[temp_num_vertices++] = v;
        }
        // Normal information
        if (strncmp(line, "vn ", 3) == 0) {
            vec3_t n;
            sscanf(line, "vn %f %f %f", &n.X, &n.Y, &n.Z);
            temp_normals[temp_num_normals++] = n;
        }
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            vec2_t uv;
            sscanf(line, "vt %f %f", &uv.U, &uv.V);
            temp_uvs[temp_num_uvs++] = uv;
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            i32 vertex_indices[3];
            i32 uv_indices[3];
            i32 normal_indices[3];
            i32 matches = sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &uv_indices[0], &normal_indices[0],
                &vertex_indices[1], &uv_indices[1], &normal_indices[1],
                &vertex_indices[2], &uv_indices[2], &normal_indices[2]);
            if (matches != 9) {
                printf("File can't be parsed.\n");
                return false;
            }

            for (i32 i = 0; i < 3; i++) {
                vec3_t v = temp_vertices[vertex_indices[i]-1];
                vec2_t t = temp_uvs[uv_indices[i]-1];
                vec3_t n = temp_normals[normal_indices[i]-1];
                mesh->vertices[mesh->num_vertices] = (vertex_t){v, n, t};;
                mesh->num_vertices++;

                if (mesh->num_vertices >= MAX_VERTS) {
                    printf("too many verts\n");
                    return false;
                }
            }
        }
    }
    fclose(file);
    return true;
}
