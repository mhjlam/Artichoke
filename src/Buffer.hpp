#pragma once

#include <vector>
#include <cstddef>
#include <functional>

#include <GL/glew.h>

#include "Main.hpp"


class Buffer
{
public:
    Buffer() = default;
    ~Buffer();

    // Create empty VAO/VBO
    void create();

    // Create and upload vertex data (for std::vector<Vertex> or compatible struct)
    void create(const void* data, size_t vertex_count, size_t vertex_stride, GLenum usage = GL_STATIC_DRAW);

    // For indexed drawing (optional for grid)
    void set_index_data(const void* index_data, size_t index_size, GLenum usage = GL_STATIC_DRAW);

    // Bind/unbind VAO
    void bind() const;
    void unbind() const;

    // Destroy VAO/VBO/EBO
    void destroy();

    // Set vertex attribute pointer (for custom layouts)
    void set_attribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void update_data(const void* data, size_t size, GLenum usage = GL_DYNAMIC_DRAW);
    void draw(GLenum mode, GLsizei count) const;
    static void set_vertex_attributes();

    GLuint vao() const { return vao_; }
    GLuint vbo() const { return vbo_; }
    GLuint ebo() const { return ebo_; }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};
