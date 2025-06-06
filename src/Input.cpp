#include "Input.hpp"

#include <vector>
#include <imgui.h>


void Input::update()
{
    ImGuiIO& io = ImGui::GetIO();

    last_mouse_pos_ = mouse_pos_;
    mouse_pos_ = glm::vec2(io.MousePos.x, io.MousePos.y);
    mouse_delta_ = mouse_pos_ - last_mouse_pos_;

    for (int i = 0; i < 5; ++i) {
        bool down = io.MouseDown[i];
        mouse_clicked_[i] = down && !mouse_down_[i];
        mouse_down_[i] = down;
    }
    mouse_wheel_ = io.MouseWheel;

    // Only track the keys you use
    static const std::vector<ImGuiKey> tracked_keys = {
        ImGuiKey_X, ImGuiKey_Y, ImGuiKey_Z
    };

    for (ImGuiKey key : tracked_keys) {
        bool down = ImGui::IsKeyDown(key);
        key_pressed_[key] = down && !key_down_[key];
        key_down_[key] = down;
    }

    want_capture_mouse_ = io.WantCaptureMouse;
    want_text_input_ = io.WantTextInput;

    // Store display size for use in picking, etc.
    display_size_ = glm::vec2(io.DisplaySize.x, io.DisplaySize.y);

    // Store delta time for use in camera animation, etc.
    delta_time_ = io.DeltaTime;
}

bool Input::key_down(ImGuiKey key) const
{
    auto it = key_down_.find(key);
    return it != key_down_.end() ? it->second : false;
}

bool Input::key_pressed(ImGuiKey key) const
{
    auto it = key_pressed_.find(key);
    return it != key_pressed_.end() ? it->second : false;
}

float Input::delta_time() const
{
    return delta_time_;
}
