#include "Chain.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Main.hpp"
#include "Kinematics.hpp"


Chain::Chain(std::shared_ptr<Camera>& camera) : 
    camera_{ camera }, shader_{}, buffer_{}, 
    selected_joint_{ -1 }, joints_{}, tendons_{},
    root_pos_{ 0.0f }, root_quat_{ 1, 0, 0, 0 }, 
    dragging_{ false }, just_selected_{ false }, drag_start_world_{}, select_start_mouse_{}
{
    float bone_length = 100.0f;
    size_t num_joints = 5;

    joints_.clear();
    glm::vec3 pos = glm::vec3(0, 0, 0);
    glm::quat rot = glm::quat(1, 0, 0, 0);

    for (size_t i = 0; i < num_joints; ++i) {
        float t = static_cast<float>(i) / (num_joints - 1);
        joints_.push_back({ pos, glm::quat(1, 0, 0, 0), glm::quat(1, 0, 0, 0), bone_length });
        pos += rot * glm::vec3(0, 0, bone_length); // advance along root's +Z
    }
    
    // Set the last joint's length to 0 (no child)
    if (!joints_.empty()) { joints_.back().length = 0.0f; }

    // Root orientation: rotate 45 degrees around Y, then -45 degrees around X
    root_quat_ = Math::axis_angle_quat(glm::vec3(0, 1, 0), 45.0f) * Math::axis_angle_quat(glm::vec3(1, 0, 0), -45.0f);
    joints_[0].local_rot = glm::quat(1, 0, 0, 0);

    for (size_t i = 1; i < joints_.size(); ++i) {
        joints_[i].local_rot = glm::quat(1, 0, 0, 0);
    }

    Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
    Kinematics::rotate_joints(joints_, root_quat_);

    shader_.load("res/color.vert", "res/color.frag");
    buffer_.create();
    buffer_.bind();
    buffer_.set_vertex_attributes();
    buffer_.unbind();
}

void Chain::drag_joint(const Input& input, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view)
{
    glm::vec2 mouse = input.mouse_pos();
    int hovered_joint = -1;

    for (size_t i = 0; i < joints_.size(); ++i) {
        glm::vec4 p = proj * view * glm::vec4(joints_[i].pos, 1.0f);
        if (p.w != 0.0f) { p /= p.w; }

        float sx = (p.x * 0.5f + 0.5f) * input.display_size().x;
        float sy = (1.0f - (p.y * 0.5f + 0.5f)) * input.display_size().y;

        if (glm::distance(mouse, glm::vec2(sx, sy)) < 15.0f) {
            hovered_joint = (int)i;
        }
    }

    if (input.mouse_clicked(0) && !input.want_capture_mouse()) {
        if (hovered_joint >= 0) {
            if (selected_joint_ == hovered_joint) {
                if (hovered_joint > 0 && view_plane != ViewPlane::XYZ) {
                    dragging_ = true;
                    drag_start_world_ = joints_[selected_joint_].pos;
                }
                else {
                    dragging_ = false;
                }
                just_selected_ = false;
            }
            else {
                dragging_ = false;
                just_selected_ = true;
                selected_joint_ = hovered_joint;
                select_start_mouse_ = input.mouse_pos();
            }
        }
        else {
            dragging_ = false;
            just_selected_ = false;
            selected_joint_ = -1;
        }
    }

    if (view_plane != ViewPlane::XYZ && input.mouse_down(0) && selected_joint_ > 0 && selected_joint_ < (int)joints_.size()) {
        if (just_selected_) {
            glm::vec2 mouseNow = input.mouse_pos();
            float dist = glm::distance(mouseNow, select_start_mouse_);

            if (dist > 8.0f) {
                dragging_ = true;
                just_selected_ = false;
                drag_start_world_ = joints_[selected_joint_].pos;
            }
        }

        if (dragging_) {
            glm::vec2 mouseNow = input.mouse_pos();

            glm::vec3 plane_point;
            switch (view_plane) {
                case ViewPlane::XY: { plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d, 0.0f); break; } 
                case ViewPlane::YZ: { plane_point = camera_->target + glm::vec3(0.0f, camera_->pan_offset_2d.x, camera_->pan_offset_2d.y); break; }
                case ViewPlane::XZ: { plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d.x, 0.0f, camera_->pan_offset_2d.y); break; }
                default:            { plane_point = camera_->target; break; }
            }

            glm::vec3 worldPos = project_to_plane(ImVec2(mouseNow.x, mouseNow.y), view_plane, proj, view, plane_point);

            if (view_plane == ViewPlane::XY) { worldPos.z = drag_start_world_.z; }
            else if (view_plane == ViewPlane::YZ) { worldPos.x = drag_start_world_.x; }
            else if (view_plane == ViewPlane::XZ) { worldPos.y = drag_start_world_.y; }

            glm::vec3 delta = worldPos - joints_[selected_joint_].pos;

            for (size_t i = selected_joint_; i < joints_.size(); ++i) {
                joints_[i].pos += delta;
            }
        }
    }
    else if (!input.mouse_down(0)) {
        if (dragging_ && selected_joint_ > 0) {
            size_t parent = selected_joint_ - 1;
            glm::quat dragged_joint_world_rot = joints_[selected_joint_].rot;

            if (parent == 0) {
                glm::quat new_root_quat = Math::compute_frame_quat(root_pos_, joints_[1].pos);
                float len = glm::length(joints_[1].pos - root_pos_);

                root_quat_ = new_root_quat;
                joints_[0].length = len;

                glm::quat new_local_rot = glm::inverse(new_root_quat) * dragged_joint_world_rot;
                joints_[1].local_rot = new_local_rot;

                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
            }
            else {
                glm::quat new_parent_world_rot = Math::compute_frame_quat(joints_[parent].pos, joints_[selected_joint_].pos);
                float len = glm::length(joints_[selected_joint_].pos - joints_[parent].pos);

                glm::quat parent_parent_world_rot = (parent == 0) ? root_quat_ : joints_[parent - 1].rot;
                glm::quat new_parent_local_rot = glm::inverse(parent_parent_world_rot) * new_parent_world_rot;

                joints_[parent].local_rot = new_parent_local_rot;
                joints_[parent].length = len;

                glm::quat new_dragged_local_rot = glm::inverse(new_parent_world_rot) * dragged_joint_world_rot;
                joints_[selected_joint_].local_rot = new_dragged_local_rot;

                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
            }
        }
        dragging_ = false;
        just_selected_ = false;
    }
}

void Chain::attach_tendon(glm::vec3& pt)
{
    if (joints_.size() < 2) return;

    float minDist = std::numeric_limits<float>::max();
    size_t minIdx = 0;
    float t_best = 0.0f;

    // Find the closest segment in 2D (projected to the current view plane)
    for (size_t i = 0; i < joints_.size() - 1; ++i) {
        glm::vec3 a = joints_[i].pos, b = joints_[i + 1].pos;

        glm::vec2 pa, pb, p;
        switch (view_plane) {
            case ViewPlane::XY:
                pa = glm::vec2(a.x, a.y);
                pb = glm::vec2(b.x, b.y);
                p = glm::vec2(pt.x, pt.y);
                break;

            case ViewPlane::YZ:
                pa = glm::vec2(a.y, a.z);
                pb = glm::vec2(b.y, b.z);
                p = glm::vec2(pt.y, pt.z);
                break;

            case ViewPlane::XZ:
                pa = glm::vec2(a.x, a.z);
                pb = glm::vec2(b.x, b.z);
                p = glm::vec2(pt.x, pt.z);
                break;

            default:
                pa = glm::vec2(a.x, a.y);
                pb = glm::vec2(b.x, b.y);
                p = glm::vec2(pt.x, pt.y);
                break;
        }

        glm::vec2 ab = pb - pa;
        float ab2 = glm::dot(ab, ab);
        float t = ab2 > 0 ? glm::clamp(glm::dot(p - pa, ab) / ab2, 0.0f, 1.0f) : 0.0f;
        glm::vec2 proj2d = pa + t * ab;
        float d = glm::distance(p, proj2d);

        if (d < minDist) {
            minDist = d;
            minIdx = i;
            t_best = t;
        }
    }

    glm::vec3 a = joints_[minIdx].pos;
    glm::vec3 b = joints_[minIdx + 1].pos;
    glm::vec3 ab = b - a;
    glm::vec3 ab_dir = glm::normalize(ab);

    Bone bone(joints_, minIdx);

    // Compute the world position on the segment at t_best
    glm::vec3 seg_pos = bone.point_at(t_best);

    // Set the out-of-plane coordinate to match the segment at t
    switch (view_plane) {
        case ViewPlane::XY: pt.z = seg_pos.z; break;
        case ViewPlane::YZ: pt.x = seg_pos.x; break;
        case ViewPlane::XZ: pt.y = seg_pos.y; break;
        default: break;
    }

    // Choose a stable up vector for this segment and store it
    glm::vec3 up = glm::abs(bone.ab_dir.z) < 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    // Build the local frame
    glm::vec3 binormal = bone.binormal(up);
    glm::vec3 normal = bone.normal(up);

    glm::vec3 proj = bone.point_at(t_best);
    glm::vec3 diff = pt - proj;

    float nor = glm::dot(diff, normal);
    float bin = glm::dot(diff, binormal);

    tendons_.push_back({ minIdx, t_best, glm::vec2(nor, bin), up });
}

glm::vec3 Chain::project_to_plane(const ImVec2& mouse, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& plane_point)
{
    float win_w = ImGui::GetIO().DisplaySize.x;
    float win_h = ImGui::GetIO().DisplaySize.y;

    // No margin or pan subtraction!
    float ndc_x = 2.0f * (mouse.x / win_w) - 1.0f;
    float ndc_y = 1.0f - 2.0f * (mouse.y / win_h);

    glm::mat4 inv_vp = glm::inverse(proj * view);
    glm::vec4 near_ndc(ndc_x, ndc_y, -1.0f, 1.0f);
    glm::vec4 far_ndc(ndc_x, ndc_y, 1.0f, 1.0f);
    glm::vec4 near_world = inv_vp * near_ndc; near_world /= near_world.w;
    glm::vec4 far_world = inv_vp * far_ndc; far_world /= far_world.w;

    glm::vec3 ray_origin = glm::vec3(near_world);
    glm::vec3 ray_dir = glm::normalize(glm::vec3(far_world) - ray_origin);

    glm::vec3 plane_normal;
    switch (view_plane) {
        case ViewPlane::XY: { plane_normal = glm::vec3(0, 0, 1); break; }
        case ViewPlane::YZ: { plane_normal = glm::vec3(1, 0, 0); break; }
        case ViewPlane::XZ: { plane_normal = glm::vec3(0, 1, 0); break; }
        default: return ray_origin;
    }

    float denom = glm::dot(ray_dir, plane_normal);
    if (std::abs(denom) < 1e-6f) return ray_origin;

    float t = glm::dot(plane_point - ray_origin, plane_normal) / denom;
    return ray_origin + t * ray_dir;
}

void Chain::update_dragged_joint_from_mouse(const Input& input, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view)
{
    if (selected_joint_ <= 0 || selected_joint_ >= (int)joints_.size()) return;
    glm::vec2 mouseNow = input.mouse_pos();

    glm::vec3 plane_point;
    switch (view_plane) {
        case ViewPlane::XY:
            plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d, 0.0f);
            break;
        case ViewPlane::YZ:
            plane_point = camera_->target + glm::vec3(0.0f, camera_->pan_offset_2d.x, camera_->pan_offset_2d.y);
            break;
        case ViewPlane::XZ:
            plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d.x, 0.0f, camera_->pan_offset_2d.y);
            break;
        default:
            plane_point = camera_->target;
            break;
    }

    glm::vec3 worldPos = project_to_plane(ImVec2(mouseNow.x, mouseNow.y), view_plane, proj, view, plane_point);

    if (view_plane == ViewPlane::XY) { worldPos.z = drag_start_world_.z; }
    else if (view_plane == ViewPlane::YZ) { worldPos.x = drag_start_world_.x; }
    else if (view_plane == ViewPlane::XZ) { worldPos.y = drag_start_world_.y; }

    glm::vec3 delta = worldPos - joints_[selected_joint_].pos;

    for (size_t i = selected_joint_; i < joints_.size(); ++i) {
        joints_[i].pos += delta;
    }
}

void Chain::update(const Input& input, ViewPlane view_plane, bool allow_add_points, const glm::mat4& proj, const glm::mat4& view)
{
    drag_joint(input, view_plane, proj, view);

    if (input.mouse_clicked(1) && !input.want_capture_mouse() && selected_joint_ >= 0) {
        dragging_ = false;
        selected_joint_ = -1;
    }

    if (selected_joint_ >= 0 && input.mouse_wheel() != 0.0f && !input.want_capture_mouse()) {
        float delta = 5.0f * input.mouse_wheel();
        glm::vec3 axis(0.0f);
        switch (view_plane) {
            case ViewPlane::XY: { axis = glm::vec3(0, 0, 1); break; }
            case ViewPlane::XZ: { axis = glm::vec3(0, 1, 0); break; }
            case ViewPlane::YZ: { axis = glm::vec3(1, 0, 0); break; }
            case ViewPlane::XYZ: default: break;
        }

        if (view_plane != ViewPlane::XYZ) {
            glm::quat q = Math::axis_angle_quat(axis, delta);

            if (selected_joint_ == 0) {
                root_quat_ = q * root_quat_;
                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
            }
            else {
                glm::quat parent_world_rot = joints_[selected_joint_ - 1].rot;

                for (size_t i = selected_joint_; i < joints_.size(); ++i) {
                    joints_[i].pos = joints_[selected_joint_].pos + q * (joints_[i].pos - joints_[selected_joint_].pos);
                    joints_[i].rot = q * joints_[i].rot;
                }

                auto& joint = joints_[selected_joint_];
                joint.local_rot = glm::inverse(parent_world_rot) * joint.rot;

                Kinematics::update_segment_lengths(joints_);
                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
            }
        }
        else {
            glm::vec3 axis(0.0f);
            if (input.key_down(ImGuiKey_X)) { axis = glm::vec3(1, 0, 0); }
            else if (input.key_down(ImGuiKey_Y)) { axis = glm::vec3(0, 1, 0); }
            else if (input.key_down(ImGuiKey_Z)) { axis = glm::vec3(0, 0, 1); }

            if (axis != glm::vec3(0.0f)) {
                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
                glm::quat q = Math::axis_angle_quat(axis, delta);
                if (selected_joint_ == 0) {
                    root_quat_ = q * root_quat_;
                }
                else {
                    auto& joint = joints_[selected_joint_];
                    joint.local_rot = q * joint.local_rot;
                }
                Kinematics::forward_kinematics(joints_, root_pos_, root_quat_);
            }
        }

        if (dragging_) {
            update_dragged_joint_from_mouse(input, view_plane, proj, view);
        }
    }

    if (allow_add_points && view_plane != ViewPlane::XYZ && input.mouse_clicked(0) && !input.want_capture_mouse()) {
        glm::vec2 mouse = input.mouse_pos();
        bool on_joint = false;
        bool on_attached = false;

        for (const auto& j : joints_) {
            glm::vec4 p = proj * view * glm::vec4(j.pos, 1.0f);
            if (p.w != 0.0f) p /= p.w;
            float sx = (p.x * 0.5f + 0.5f) * input.display_size().x;
            float sy = (1.0f - (p.y * 0.5f + 0.5f)) * input.display_size().y;
            if (glm::distance(mouse, glm::vec2(sx, sy)) < 15.0f) {
                on_joint = true;
                break;
            }
        }
        for (const auto& ap : tendons_) {
            Bone bone(joints_, ap.bone_idx);
            glm::vec3 up = ap.up;
            glm::vec3 binormal = bone.binormal(up);
            glm::vec3 normal = bone.normal(up);
            glm::vec3 world_pos = bone.point_at(ap.t) + ap.local_offset.x * normal + ap.local_offset.y * binormal;
            
            glm::vec4 p = proj * view * glm::vec4(world_pos, 1.0f);
            if (p.w != 0.0f) p /= p.w;
            
            float sx = (p.x * 0.5f + 0.5f) * input.display_size().x;
            float sy = (1.0f - (p.y * 0.5f + 0.5f)) * input.display_size().y;
            
            if (glm::distance(mouse, glm::vec2(sx, sy)) < 15.0f) {
                on_attached = true;
                break;
            }
        }

        if (!on_joint && !on_attached) {
            glm::vec3 plane_point;
            switch (view_plane) {
                case ViewPlane::XY:
                    plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d, 0.0f);
                    break;
                case ViewPlane::YZ:
                    plane_point = camera_->target + glm::vec3(0.0f, camera_->pan_offset_2d.x, camera_->pan_offset_2d.y);
                    break;
                case ViewPlane::XZ:
                    plane_point = camera_->target + glm::vec3(camera_->pan_offset_2d.x, 0.0f, camera_->pan_offset_2d.y);
                    break;
                default:
                    plane_point = camera_->target;
                    break;
            }

            glm::vec3 world_pos = project_to_plane(ImVec2(mouse.x, mouse.y), view_plane, proj, view, plane_point);
            attach_tendon(world_pos);
        }
    }
}

void Chain::draw_batch(const std::vector<Vertex>& verts, GLenum mode, float size_or_width) {
    if (verts.empty()) { return; }
    if (mode == GL_POINTS) { glPointSize(size_or_width); }
    if (mode == GL_LINES) { glLineWidth(size_or_width); }
    buffer_.update_data(verts.data(), verts.size() * sizeof(Vertex));
    buffer_.draw(mode, (GLsizei)verts.size());
}

void Chain::render(const glm::mat4& mvp, bool tendons_only)
{
    glm::vec3 outline_color = glm::vec3(0, 0, 0);
    glm::vec3 main_color = glm::vec3(0.85f, 0.85f, 0.85f);

    shader_.use();
    shader_.set_mvp(mvp);
    buffer_.bind();
    buffer_.set_vertex_attributes();

    // Batch tendons
    std::vector<Vertex> tendon_borders, tendon_points;
    for (const auto& ap : tendons_) {
        Bone bone(joints_, ap.bone_idx);
        glm::vec3 up = ap.up;
        glm::vec3 binormal = bone.binormal(up);
        glm::vec3 normal = bone.normal(up);
        glm::vec3 world_pos = bone.point_at(ap.t) + ap.local_offset.x * normal + ap.local_offset.y * binormal;

        tendon_borders.push_back({ world_pos, glm::vec3(0.0f) });
        tendon_points.push_back({ world_pos, glm::vec3(1.0f, 0.85f, 0.2f) });
    }
    draw_batch(tendon_borders, GL_POINTS, 14.0f);
    draw_batch(tendon_points, GL_POINTS, 10.0f);

    if (tendons_only) {
        buffer_.unbind();
        shader_.unuse();
        return;
    }

    // Batch bones
    std::vector<Vertex> bone_outline, bone_main;
    for (size_t i = 0; i + 1 < joints_.size(); ++i) {
        bone_outline.push_back({ joints_[i].pos, outline_color });
        bone_outline.push_back({ joints_[i + 1].pos, outline_color });
        bone_main.push_back({ joints_[i].pos, main_color });
        bone_main.push_back({ joints_[i + 1].pos, main_color });
    }
    draw_batch(bone_outline, GL_LINES, 8.0f);
    draw_batch(bone_main, GL_LINES, 4.0f);

    // Batch joints
    std::vector<Vertex> joint_outlines, joint_main;
    for (const auto& j : joints_) {
        joint_outlines.push_back({ j.pos, outline_color });
        joint_main.push_back({ j.pos, main_color });
    }
    draw_batch(joint_outlines, GL_POINTS, 16.0f);

    // Highlight selected joint (drawn after outlines, before main joints)
    if (selected_joint_ >= 0 && selected_joint_ < (int)joint_main.size()) {
        Vertex vtx = joint_main[selected_joint_];

        // Glow (largest, orange)
        vtx.color = glm::vec3(1.0f, 0.4f, 0.2f);
        draw_batch({vtx}, GL_POINTS, 22.0f);

        // Main highlight (medium, red)
        vtx.color = glm::vec3(1.0f, 0.2f, 0.2f);
        draw_batch({vtx}, GL_POINTS, 18.0f);
    }

    // Main joints (drawn on top, smaller)
    draw_batch(joint_main, GL_POINTS, 12.0f);

    // Batch axes
    std::vector<Vertex> axes;
    float axis_len = 25.0f;
    for (size_t i = 0; i < joints_.size(); ++i) {
        const glm::vec3& p = joints_[i].pos;
        glm::mat3 R = glm::mat3_cast(joints_[i].rot);
        axes.push_back({ p, glm::vec3(0.75f, 0.15f, 0.20f) });
        axes.push_back({ p + axis_len * (R * glm::vec3(1, 0, 0)), glm::vec3(0.75f, 0.15f, 0.20f) });
        axes.push_back({ p, glm::vec3(0.10f, 0.50f, 0.20f) });
        axes.push_back({ p + axis_len * (R * glm::vec3(0, 1, 0)), glm::vec3(0.10f, 0.50f, 0.20f) });
        axes.push_back({ p, glm::vec3(0.22f, 0.40f, 0.90f) });
        axes.push_back({ p + axis_len * (R * glm::vec3(0, 0, 1)), glm::vec3(0.22f, 0.40f, 0.90f) });
    }
    draw_batch(axes, GL_LINES, 2.0f);

    buffer_.unbind();
    shader_.unuse();
}
