#include "Renderer.hpp"

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <filesystem>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glut.h>
#include <imgui_impl_opengl3.h>

#include "Grid.hpp"
#include "Main.hpp"
#include "Chain.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Overlay.hpp"


static std::vector<Vertex> axis_data = {
    Vertex{{0, 0, 0}, {0.75f, 0.15f, 0.20f}},
    Vertex{{200, 0, 0}, {0.75f, 0.15f, 0.20f}},
    Vertex{{0, 0, 0}, {0.10f, 0.50f, 0.20f}},
    Vertex{{0, 200, 0}, {0.10f, 0.50f, 0.20f}},
    Vertex{{0, 0, 0}, {0.22f, 0.40f, 0.90f}},
    Vertex{{0, 0, 200}, {0.22f, 0.40f, 0.90f}},
};


Renderer*& Renderer::instance()
{
    static Renderer* inst = nullptr;
    return inst;
}


Renderer::Renderer(int argc, char** argv) : 
    shader_{}, main_buffer_{}, axis_buffer_{}, 
    grid_{ nullptr }, camera_{ nullptr }, chain_{ nullptr }, overlay_{ nullptr }
{
    instance() = this;

    glutInit(&argc, argv);
    int win_w = WINDOW_WIDTH, win_h = WINDOW_HEIGHT;

    // Get screen size
    int screen_w = glutGet(GLUT_SCREEN_WIDTH);
    int screen_h = glutGet(GLUT_SCREEN_HEIGHT);

    // Center position
    int pos_x = (screen_w - win_w) / 2;
    int pos_y = (screen_h - win_h) / 2;

    // Initialize GLUT
    glutInitWindowSize(win_w, win_h);
    glutInitWindowPosition(pos_x, pos_y); // Center the window
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutSetOption(GLUT_MULTISAMPLE, 4);
    glutCreateWindow("Artichoke - Articulated Chain Viewer");

    // GLUT callbacks
    glutDisplayFunc([]() { 
        instance()->display();
    });
    glutReshapeFunc([](int w, int h) { 
        instance()->reshape(w, h);
    });
    glutIdleFunc([]() { 
        glutPostRedisplay();
    });

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(err));
        exit(1);
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    // Initialize shaders and buffers
    shader_.load("res/color.vert", "res/color.frag");

    main_buffer_.create();

    axis_buffer_.create();
    axis_buffer_.bind();
    glBufferData(GL_ARRAY_BUFFER, axis_data.size() * sizeof(Vertex), axis_data.data(), GL_STATIC_DRAW);
    Buffer::set_vertex_attributes(); // Unified attribute setup
    axis_buffer_.unbind();

    // Initialize chain and camera
    grid_ = std::make_unique<Grid>();
    camera_ = std::make_shared<Camera>();
    chain_ = std::make_shared<Chain>(camera_);
    overlay_ = std::make_unique<Overlay>(camera_, chain_);
}

Renderer::~Renderer()
{
    delete_buffers();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::run()
{
    glutMainLoop();
}

void Renderer::display()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    input_.update();

    bool changed = overlay_->draw_menu(input_);

    if (changed) {
        chain_->view_plane = camera_->view_plane;
    }

    if (changed || !input_.want_capture_mouse()) {
        camera_->update(input_, chain_->active_joint());
    }

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) { 
        chain_->update(input_, chain_->view_plane, chain_->view_plane != ViewPlane::XYZ, 
            camera_->get_proj(static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT), camera_->get_view());
    }

    // Use Grid class for background gradient and grid
    if (camera_->view_plane != ViewPlane::XYZ) {
        grid_->draw_2d(*camera_, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    else {
        grid_->draw_3d(*camera_, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    glClearColor(0.85f, 0.85f, 0.80f, 1.0f);
    glEnable(GL_MULTISAMPLE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 mvp = camera_->get_proj(static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT) * camera_->get_view();

    if (camera_->view_plane == ViewPlane::XYZ) {
        // Draw the axis lines
        shader_.use();
        shader_.setUniform("uMVP", mvp);
        axis_buffer_.bind();
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 6);
        glLineWidth(1.0f);
        axis_buffer_.unbind();
    }

    // Render the articulated chain
    chain_->render(mvp, overlay_->hide_chain());

    // Draw the UI overlays
    overlay_->draw_overlays();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();
}

void Renderer::reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
}

void Renderer::delete_buffers()
{
    main_buffer_.destroy();
    axis_buffer_.destroy();
}

void Renderer::create_axis_buffer()
{

}
