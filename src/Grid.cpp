#include "Grid.hpp"

#include <vector>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


Grid::Grid() : gradient_shader_("res/color.vert", "res/color.frag"), grid_shader_("res/color.vert", "res/color.frag")
{
    // Create fullscreen quad for gradient
    std::vector<Vertex> quad = {
        {{0.f, 0.f, 0.f}, {0.90f, 0.90f, 0.85f}},
        {{1.f, 0.f, 0.f}, {0.85f, 0.85f, 0.80f}},
        {{0.f, 1.f, 0.f}, {0.85f, 0.85f, 0.80f}},
        {{1.f, 1.f, 0.f}, {0.80f, 0.80f, 0.75f}},
    };

    quad_buffer_.create();
    quad_buffer_.bind();
    glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(Vertex), quad.data(), GL_STATIC_DRAW);
    quad_buffer_.set_attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    quad_buffer_.set_attribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    quad_buffer_.unbind();
}

void Grid::create_2d_grid(int width, int height, float spacing) {
    std::vector<Vertex> lines;
    float w = static_cast<float>(width);
    float h = static_cast<float>(height);

    for (float x = fmodf(0, spacing); x < w; x += spacing) {
        lines.push_back({{x, 0, 0}, {0.71f, 0.71f, 0.67f}});
        lines.push_back({{x, h, 0}, {0.71f, 0.71f, 0.67f}});
    }

    for (float y = fmodf(0, spacing); y < h; y += spacing) {
        lines.push_back({{0, y, 0}, {0.71f, 0.71f, 0.67f}});
        lines.push_back({{w, y, 0}, {0.71f, 0.71f, 0.67f}});
    }

    grid2d_buffer_.create();
    grid2d_buffer_.bind();
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vertex), lines.data(), GL_STATIC_DRAW);
    grid2d_buffer_.set_attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    grid2d_buffer_.set_attribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    grid2d_buffer_.unbind();
    grid2d_vertex_count_ = static_cast<int>(lines.size());
}

void Grid::create_3d_grid(float grid_size, float grid_spacing)
{
    std::vector<Vertex> lines;
    int num_lines = static_cast<int>(grid_size / grid_spacing);

    for (int i = -num_lines; i <= num_lines; ++i) {
        float x = i * grid_spacing;

        // XY plane
        lines.push_back({ {-grid_size, x, 0.0f}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {grid_size, x, 0.0f}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {x, -grid_size, 0.0f}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {x, grid_size, 0.0f}, {0.82f, 0.82f, 0.82f} });

        // XZ plane
        lines.push_back({ {-grid_size, 0.0f, x}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {grid_size, 0.0f, x}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {x, 0.0f, -grid_size}, {0.82f, 0.82f, 0.82f} });
        lines.push_back({ {x, 0.0f, grid_size}, {0.82f, 0.82f, 0.82f} });
    }

    grid3d_buffer_.create();
    grid3d_buffer_.bind();
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vertex), lines.data(), GL_STATIC_DRAW);
    grid3d_buffer_.set_attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    grid3d_buffer_.set_attribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    grid3d_buffer_.unbind();
    grid3d_vertex_count_ = static_cast<int>(lines.size());
}

void Grid::draw_2d(const Camera& camera, int width, int height) {
    // Draw gradient
    gradient_shader_.use();
    glm::mat4 proj = glm::ortho(0.f, 1.f, 0.f, 1.f); // This is fine for the quad
    gradient_shader_.setUniform("uMVP", proj);
    quad_buffer_.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quad_buffer_.unbind();

    // Draw grid lines
    if (grid2d_vertex_count_ == 0) {
        create_2d_grid(width, height, 50.0f);
    }

    grid_shader_.use();
    // Bottom-left origin for 2D grid
    glm::mat4 grid_proj = glm::ortho(0.f, float(width), 0.f, float(height));
    grid_shader_.setUniform("uMVP", grid_proj);
    grid2d_buffer_.bind();
    glDrawArrays(GL_LINES, 0, grid2d_vertex_count_);
    grid2d_buffer_.unbind();
}

void Grid::draw_3d(const Camera& camera, int width, int height) {
    if (grid3d_vertex_count_ == 0) {
        create_3d_grid(1000.0f, 100.0f);
    }

    grid_shader_.use();
    glm::mat4 mvp = camera.get_proj(float(width) / height) * camera.get_view();
    grid_shader_.setUniform("uMVP", mvp);
    grid3d_buffer_.bind();
    glDrawArrays(GL_LINES, 0, grid3d_vertex_count_);
    grid3d_buffer_.unbind();
}
