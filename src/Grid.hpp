#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Shader.hpp"


class Grid {
public:
    Grid();

    void draw_2d(const Camera& camera, int width, int height);
    void draw_3d(const Camera& camera, int width, int height);

private:
    void create_gradient_quad();
    void create_2d_grid(int width, int height, float spacing);
    void create_3d_grid(float grid_size, float grid_spacing);

    Buffer quad_buffer_;
    Buffer grid2d_buffer_;
    Buffer grid3d_buffer_;
    Shader gradient_shader_;
    Shader grid_shader_;

    int grid2d_vertex_count_ = 0;
    int grid3d_vertex_count_ = 0;
};