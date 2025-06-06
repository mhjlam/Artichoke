#pragma once

#include <memory>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Input.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"


class Grid;
class Chain;
class Camera;
class Overlay;


class Renderer
{
public:
    Renderer(int argc, char** argv);
    ~Renderer();

    void run();

    // Main display and reshape callbacks
    void display();
    void reshape(int w, int h);

    // OpenGL buffer management
    void delete_buffers();
    void create_axis_buffer();

private:
    Input input_;

    // Shader program
    Shader shader_;

    // Render buffers
    Buffer main_buffer_;
    Buffer axis_buffer_;

    // Articulated chain and camera
    std::unique_ptr<Grid> grid_;
    std::shared_ptr<Chain> chain_;
    std::shared_ptr<Camera> camera_;
    std::unique_ptr<Overlay> overlay_;

    // Singleton instance
    static Renderer*& instance();
};
