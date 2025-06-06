#include "Overlay.hpp"

#include <cmath>
#include <string>
#include <iostream>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

#include "Input.hpp"
#include "Chain.hpp"
#include "Camera.hpp"


Overlay::Overlay(std::shared_ptr<Camera> camera, std::shared_ptr<Chain> chain) : camera_(std::move(camera)), chain_(chain)
{
    load_fonts();
}

void Overlay::load_fonts()
{
    ImGuiIO& io = ImGui::GetIO();

    // Load Roboto-Regular.ttf as the default font
    const char* regular_path = "res/Roboto-Regular.ttf";
    if (!std::filesystem::exists(regular_path)) {
        std::cerr << "Font not found: " << regular_path << '\n';
    }
    ImFont* regular = io.Fonts->AddFontFromFileTTF(regular_path, 14.0f);
    if (!regular) {
        std::cerr << "Failed to load " << regular_path << '\n';
    }
    fonts_["Regular"] = regular;

    // Load Roboto-Bold.ttf as the axis font
    const char* axis_path = "res/Roboto-Bold.ttf";
    if (!std::filesystem::exists(axis_path)) {
        std::cerr << "Font not found: " << axis_path << '\n';
    }
    ImFont* axis = io.Fonts->AddFontFromFileTTF(axis_path, 20.0f);
    if (!axis) {
        std::cerr << "Failed to load " << axis_path << '\n';
    }
    fonts_["Axis"] = axis;

    io.Fonts->Build();
}

ImFont* Overlay::get_font(const std::string& name) const
{
    auto it = fonts_.find(name);
    return (it != fonts_.end()) ? it->second : nullptr;
}

bool Overlay::draw_menu(Input& input)
{
    bool changed = false;
    int plane_idx = static_cast<int>(chain_->view_plane);
    const char* plane_items[] = { "XY Plane", "ZX Plane", "YZ Plane", "3D Perspective" };

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(340, 0), ImGuiCond_Always);
    ImGui::Begin("##Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGui::TextUnformatted((plane_idx == 3) ? "View: Perspective" : "View: Orthographic");
    if (ImGui::Combo("##View Plane", &plane_idx, plane_items, IM_ARRAYSIZE(plane_items))) {
        changed = true;
    }
    ImGui::Separator();

    // Add checkbox for chain visibility in 3D
    if (plane_idx == 3) {
        ImGui::Checkbox("Hide Chain", &hide_chain_);
        ImGui::Separator();
    }

    int active_joint = chain_->active_joint();
    auto& joints = chain_->joints();
    int joint_to_show = (active_joint >= 0 && active_joint < (int)joints.size()) ? active_joint : 0;
    auto& joint = joints.empty() ? *(Joint*)nullptr : joints[joint_to_show];

    bool disabled = (active_joint < 0 || active_joint >= (int)joints.size());
    if (disabled) ImGui::BeginDisabled();

    ImGui::Text("Joint");

    // 3D View
    if (chain_->view_plane == ViewPlane::XYZ) {
        static int last_joint_idx = -1;
        static glm::vec3 drag_angles_deg(0.0f);

        auto get_axis_angles = [](const glm::quat& q) -> glm::vec3 {
            glm::vec3 angles;
            for (int i = 0; i < 3; ++i) {
                glm::vec3 axis(0.0f); axis[i] = 1.0f;
                glm::vec3 local_axis = q * axis;
                float dot = glm::clamp(glm::dot(glm::normalize(local_axis), axis), -1.0f, 1.0f);
                float angle_deg = glm::degrees(std::acos(dot));
                if (angle_deg < 0.0f) angle_deg += 360.0f;
                angles[i] = angle_deg;
            }
            return angles;
        };

        if (last_joint_idx != joint_to_show) {
            glm::quat q = (joint_to_show == 0) ? chain_->root_quat() : joint.local_rot;
            drag_angles_deg = get_axis_angles(q);
            last_joint_idx = joint_to_show;
        }

        glm::quat q = (joint_to_show == 0) ? chain_->root_quat() : joint.local_rot;
        glm::vec3 current_angles_deg = get_axis_angles(q);
        drag_angles_deg = current_angles_deg;

        glm::vec3 prev_angles = drag_angles_deg;
        bool changed_drag = ImGui::DragFloat3("World", glm::value_ptr(drag_angles_deg), 0.5f, 0.0f, 360.0f, "%.2f°");

        if (changed_drag) {
            for (int i = 0; i < 3; ++i) {
                float delta = drag_angles_deg[i] - prev_angles[i];
                if (delta > 180.0f) delta -= 360.0f;
                if (delta < -180.0f) delta += 360.0f;
                if (std::abs(delta) > 1e-4f) {
                    glm::vec3 axis(0.0f); axis[i] = 1.0f;
                    glm::quat dq = glm::angleAxis(glm::radians(delta), axis);
                    if (joint_to_show == 0) {
                        glm::quat new_root = dq * chain_->root_quat();
                        chain_->set_root_quat(new_root);
                    } else {
                        joint.local_rot = dq * joint.local_rot;
                    }
                    chain_->forward_kinematics();
                }
            }
            q = (joint_to_show == 0) ? chain_->root_quat() : joint.local_rot;
            drag_angles_deg = get_axis_angles(q);
        }

        glm::vec3 euler = glm::degrees(glm::eulerAngles(q));
        ImGui::BeginDisabled();
        ImGui::DragFloat4("Local", glm::value_ptr(q), 0.01f, -1.0f, 1.0f, "%.3f");
        ImGui::EndDisabled();
    }
    // 2D Views
    else {
        auto get_joint_to_next_angle = [&](const Joint& joint, const Joint* next, ViewPlane plane) -> float {
            if (!next) return 0.0f;
            glm::vec3 v = next->pos - joint.pos;
            float angle = 0.0f;
            switch (plane) {
                case ViewPlane::XY: angle = std::atan2(v.y, v.x); break;
                case ViewPlane::XZ: angle = std::atan2(v.z, v.x); break;
                case ViewPlane::YZ: angle = std::atan2(v.y, v.z); break;
                default: angle = 0.0f; break;
            }
            angle = glm::degrees(angle);
            return angle;
        };

        float geometric_angle = 0.0f;
        bool is_last_joint = (joint_to_show == (int)joints.size() - 1);

        if (!is_last_joint) {
            geometric_angle = get_joint_to_next_angle(joint, &joints[joint_to_show + 1], chain_->view_plane);
        } else {
            glm::quat world_rot = chain_->world_rotation(joint_to_show);
            glm::vec3 x_axis = world_rot * glm::vec3(1, 0, 0);
            switch (chain_->view_plane) {
                case ViewPlane::XY: geometric_angle = glm::degrees(std::atan2(x_axis.y, x_axis.x)); break;
                case ViewPlane::XZ: geometric_angle = glm::degrees(std::atan2(x_axis.z, x_axis.x)); break;
                case ViewPlane::YZ: geometric_angle = glm::degrees(std::atan2(x_axis.y, x_axis.z)); break;
                default: geometric_angle = 0.0f; break;
            }
        }

        static int last_joint = -1;
        static ViewPlane last_plane = ViewPlane::XY;
        static float prev_angle = 0.0f;
        static float angle_input = 0.0f;

        if (last_joint != joint_to_show || last_plane != chain_->view_plane) {
            prev_angle = geometric_angle;
            angle_input = geometric_angle;
            last_joint = joint_to_show;
            last_plane = chain_->view_plane;
        }

        if (is_last_joint) {
            angle_input = geometric_angle;
            prev_angle = geometric_angle;
        }

        if (!is_last_joint && chain_->view_plane != ViewPlane::XYZ) {
            int scrolled_joint = chain_->active_joint();
            if (scrolled_joint == joint_to_show && scrolled_joint > 0 && scrolled_joint < (int)joints.size()) {
                float new_angle = 0.0f;
                const Joint& joint = joints[scrolled_joint];
                const Joint& next = joints[scrolled_joint + 1];
                glm::vec3 v = next.pos - joint.pos;
                switch (chain_->view_plane) {
                    case ViewPlane::XY: new_angle = glm::degrees(std::atan2(v.y, v.x)); break;
                    case ViewPlane::XZ: new_angle = glm::degrees(std::atan2(v.z, v.x)); break;
                    case ViewPlane::YZ: new_angle = glm::degrees(std::atan2(v.y, v.z)); break;
                    default: break;
                }
                float revolutions = std::floor((angle_input - new_angle) / 360.0f + 0.5f);
                angle_input = new_angle + revolutions * 360.0f;
                prev_angle = angle_input;
            }
        }

        std::string angle_label = "Angle";
        switch (chain_->view_plane) {
            case ViewPlane::XY: angle_label += " X"; break;
            case ViewPlane::XZ: angle_label += " Y"; break;
            case ViewPlane::YZ: angle_label += " Z"; break;
            default: break;
        }

        bool changed_drag = ImGui::DragFloat(angle_label.c_str(), &angle_input, 1.0f, -10000.0f, 10000.0f, "%.1f°");

        if (!disabled && changed_drag && joint_to_show >= 0) {
            float delta = angle_input - prev_angle;

            int axis = 2;
            if (chain_->view_plane == ViewPlane::YZ) { axis = 0; }
            if (chain_->view_plane == ViewPlane::XZ) { axis = 1; }

            glm::vec3 axis_vec(0.0f);
            axis_vec[axis] = 1.0f;
            glm::quat q = glm::angleAxis(glm::radians(delta), axis_vec);

            if (joint_to_show == 0) {
                glm::quat new_root = q * chain_->root_quat();
                chain_->set_root_quat(new_root);
                chain_->forward_kinematics();
            } else {
                glm::quat parent_world_rot = joints[joint_to_show - 1].rot;
                for (size_t i = joint_to_show; i < joints.size(); ++i) {
                    joints[i].pos = joints[joint_to_show].pos + q * (joints[i].pos - joints[joint_to_show].pos);
                    joints[i].rot = q * joints[i].rot;
                }
                auto& joint_ref = joints[joint_to_show];
                joint_ref.local_rot = glm::inverse(parent_world_rot) * joint_ref.rot;
                for (size_t i = 0; i + 1 < joints.size(); ++i) {
                    joints[i].length = glm::length(joints[i + 1].pos - joints[i].pos);
                }
                chain_->forward_kinematics();
            }

            if (joint_to_show < (int)joints.size() - 1) {
                float new_angle = get_joint_to_next_angle(joint, &joints[joint_to_show + 1], chain_->view_plane);
                float revolutions = std::floor((angle_input - new_angle) / 360.0f + 0.5f);
                angle_input = new_angle + revolutions * 360.0f;
                prev_angle = angle_input;
            } else {
                angle_input = 0.0f;
                prev_angle = 0.0f;
            }
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            if (joint_to_show < (int)joints.size() - 1) {
                float new_angle = get_joint_to_next_angle(joint, &joints[joint_to_show + 1], chain_->view_plane);
                angle_input = std::fmod(new_angle, 360.0f);
                if (angle_input < 0.0f) {
                    angle_input += 360.0f;
                }
                prev_angle = angle_input;
            } else {
                angle_input = 0.0f;
                prev_angle = 0.0f;
            }
        }

        ImGui::BeginDisabled();
        ImGui::DragFloat4("Local", glm::value_ptr(joint.local_rot), 0.01f, -1.0f, 1.0f, "%.3f");
        ImGui::EndDisabled();
    }

    ImGui::Separator();
    ImGui::Text("Bone");

    int bone_length_joint = (joint_to_show >= 0 && joint_to_show < (int)joints.size() - 1) ? joint_to_show : 0;
    bool bone_length_disabled = (active_joint < 0 || active_joint >= (int)joints.size() || bone_length_joint >= (int)joints.size() - 1);

    if (bone_length_disabled) ImGui::BeginDisabled();
    if (!joints.empty() && bone_length_joint < (int)joints.size() - 1) {
        float min_length = 25.0f;
        float max_length = 500.0f;
        float& length = joints[bone_length_joint].length;
        float prev_length = length;

        if (ImGui::DragFloat("Length", &length, 1.0f, min_length, max_length, "%.1f")) {
            if (length < min_length) { length = min_length; }
            if (length > max_length) { length = max_length; }
            chain_->forward_kinematics();
        }
    } else {
        float length = (!joints.empty()) ? joints[0].length : 0.0f;
        ImGui::DragFloat("Bone Length", &length, 1.0f, 0.0f, 0.0f, "%.1f", ImGuiSliderFlags_NoInput);
    }
    if (bone_length_disabled) ImGui::EndDisabled();

    if (disabled) ImGui::EndDisabled();

    ImGui::End();

    if (changed) {
        chain_->view_plane = static_cast<ViewPlane>(plane_idx);
        camera_->view_plane = static_cast<ViewPlane>(plane_idx);
    }

    return changed;
}

void Overlay::draw_overlays()
{
    // 2D grid background
    if (camera_->view_plane != ViewPlane::XYZ) {
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        const ImVec2 display_size = ImGui::GetIO().DisplaySize;
        float grid_spacing = 50.0f;
        ImU32 grid_col = IM_COL32(180, 180, 170, 60);
        for (float x = fmodf(0, grid_spacing); x < display_size.x; x += grid_spacing)
            draw_list->AddLine(ImVec2(x, 0), ImVec2(x, display_size.y), grid_col, 1.0f);
        for (float y = fmodf(0, grid_spacing); y < display_size.y; y += grid_spacing)
            draw_list->AddLine(ImVec2(0, y), ImVec2(display_size.x, y), grid_col, 1.0f);
    }

    // 2D axes overlay
    if (camera_->view_plane != ViewPlane::XYZ) {
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        const ImVec2 display_size = ImGui::GetIO().DisplaySize;
        const float axis_len_x = 0.5f * display_size.x;
        const float axis_len_y = 0.5f * display_size.y;
        const float margin = 16.0f;

        ImVec2 origin;
        ImVec2 h_dir, v_dir;
        ImU32 h_col, v_col;

        const float thickness = 2.5f;

        // Use unified mapping
        PlaneAxisInfo axis_info = get_plane_axis_info(camera_->view_plane);
        const char* h_label = axis_info.h_label;
        const char* v_label = axis_info.v_label;
        glm::vec3 h_col_vec = axis_info.h_color;
        glm::vec3 v_col_vec = axis_info.v_color;
        h_col = IM_COL32((int)(h_col_vec.r * 255), (int)(h_col_vec.g * 255), (int)(h_col_vec.b * 255), 255);
        v_col = IM_COL32((int)(v_col_vec.r * 255), (int)(v_col_vec.g * 255), (int)(v_col_vec.b * 255), 255);

        switch (camera_->view_plane) {
            case ViewPlane::XY:
                origin = ImVec2(margin, display_size.y - margin);
                h_dir = ImVec2(axis_len_x, 0);
                v_dir = ImVec2(0, -axis_len_y);
                break;
            case ViewPlane::YZ:
            case ViewPlane::XZ:
                origin = ImVec2(display_size.x - margin, display_size.y - margin);
                h_dir = ImVec2(-axis_len_x, 0);
                v_dir = ImVec2(0, -axis_len_y);
                break;
            default:
                origin = ImVec2(0, 0);
                h_dir = v_dir = ImVec2(0, 0);
                break;
        }

        draw_list->AddLine(origin, ImVec2(origin.x + h_dir.x, origin.y + h_dir.y), h_col, thickness);
        draw_list->AddLine(origin, ImVec2(origin.x + v_dir.x, origin.y + v_dir.y), v_col, thickness);

        ImVec2 h_label_pos = (camera_->view_plane == ViewPlane::YZ || camera_->view_plane == ViewPlane::XZ)
            ? ImVec2(origin.x + h_dir.x - margin, origin.y + h_dir.y - 4)
            : ImVec2(origin.x + h_dir.x + 4, origin.y + h_dir.y - 4);
        ImVec2 v_label_pos = (camera_->view_plane == ViewPlane::YZ || camera_->view_plane == ViewPlane::XZ)
            ? ImVec2(origin.x + v_dir.x - 1, origin.y + v_dir.y - margin)
            : ImVec2(origin.x + v_dir.x - 3, origin.y + v_dir.y - margin);

        ImFont* axisFont = get_font("Axis");
        if (axisFont) ImGui::PushFont(axisFont);
        draw_list->AddText(h_label_pos, h_col, h_label);
        draw_list->AddText(v_label_pos, v_col, v_label);
        if (axisFont) ImGui::PopFont();
    }
}