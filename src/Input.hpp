#pragma once

#include <imgui.h>
#include <glm/glm.hpp>
#include <array>
#include <unordered_map>


class Input
{
public:
    void update();

    // Mouse
    glm::vec2 mouse_pos() const { return mouse_pos_; }
    glm::vec2 mouse_delta() const { return mouse_delta_; }
    bool mouse_down(int button) const { return mouse_down_[button]; }
    bool mouse_clicked(int button) const { return mouse_clicked_[button]; }
    float mouse_wheel() const { return mouse_wheel_; }

    // Keyboard
    bool key_down(ImGuiKey key) const;
    bool key_pressed(ImGuiKey key) const;

    // ImGui capture
    bool want_capture_mouse() const { return want_capture_mouse_; }
    bool want_text_input() const { return want_text_input_; }

    // Display size (window size in pixels)
    glm::vec2 display_size() const { return display_size_; }

    // Frame delta time (seconds)
    float delta_time() const;

private:
    glm::vec2 mouse_pos_{ 0.0f };
    glm::vec2 last_mouse_pos_{ 0.0f };
    glm::vec2 mouse_delta_{ 0.0f };
    std::array<bool, 5> mouse_down_{};
    std::array<bool, 5> mouse_clicked_{};
    float mouse_wheel_{ 0.0f };

    // Only track keys you use
    std::unordered_map<ImGuiKey, bool> key_down_;
    std::unordered_map<ImGuiKey, bool> key_pressed_;

    bool want_capture_mouse_{ false };
    bool want_text_input_{ false };

    glm::vec2 display_size_{ 0.0f, 0.0f };

    float delta_time_{ 1.0f / 60.0f };
};
