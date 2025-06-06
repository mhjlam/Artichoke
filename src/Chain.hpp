#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <imgui.h>

#include "Main.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "Kinematics.hpp"


class Chain
{
public:
    explicit Chain(std::shared_ptr<Camera>& camera);

    void update(const Input& input, ViewPlane view_plane, bool allow_add_points, const glm::mat4& proj, const glm::mat4& view);
    void render(const glm::mat4& mvp, bool tendons_only = false);

    int active_joint() const { return selected_joint_; }
    std::vector<Joint>& joints() { return joints_; }
    const std::vector<Joint>& joints() const { return joints_; }
    glm::quat root_quat() const { return root_quat_; }
    void set_root_quat(const glm::quat& q) { root_quat_ = q; }
    glm::quat world_rotation(size_t idx) const { return joints_[idx].rot; }

    void forward_kinematics() { Kinematics::forward_kinematics(joints_, root_pos_, root_quat_); }

public:
    ViewPlane view_plane = ViewPlane::XY;

private:
    void drag_joint(const Input& input, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view);
    void update_dragged_joint_from_mouse(const Input& input, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view);
    void attach_tendon(glm::vec3& pt);
    glm::vec3 project_to_plane(const ImVec2& mouse, ViewPlane view_plane, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& plane_point);

    void draw_batch(const std::vector<Vertex>& verts, GLenum mode, float size_or_width);

private:
    Shader shader_;
    Buffer buffer_;
    std::shared_ptr<Camera> camera_;

    int selected_joint_;
    std::vector<Joint> joints_;
    std::vector<Tendon> tendons_;

    glm::vec3 root_pos_;
    glm::quat root_quat_;

    bool dragging_;
    bool just_selected_;
    glm::vec3 drag_start_world_;
    glm::vec2 select_start_mouse_;
};
