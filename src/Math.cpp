#include "Math.hpp"

#include <glm/gtc/constants.hpp>

#include "Main.hpp"

glm::quat Math::axis_angle_quat(const glm::vec3& axis, float angle_deg) {
    return glm::angleAxis(glm::radians(angle_deg), glm::normalize(axis));
}

glm::quat Math::compute_frame_quat(const glm::vec3& from, const glm::vec3& to, glm::vec3 up) {
    glm::vec3 dir = glm::normalize(to - from);
    if (std::abs(glm::dot(dir, up)) > 0.99f) { up = glm::vec3(1, 0, 0); }
    glm::vec3 x_axis = glm::normalize(glm::cross(up, dir));
    glm::vec3 y_axis = glm::normalize(glm::cross(dir, x_axis));
    glm::mat3 rot_mtx(x_axis, y_axis, dir);
    return glm::quat_cast(rot_mtx);
}

glm::quat Math::compute_frame_quat_from_dir(const glm::vec3& dir, glm::vec3 up) {
    glm::vec3 z_axis = glm::normalize(dir);
    if (std::abs(glm::dot(z_axis, up)) > 0.99f) { up = glm::vec3(1, 0, 0); }
    glm::vec3 x_axis = glm::normalize(glm::cross(up, z_axis));
    glm::vec3 y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    glm::mat3 rot_mtx(x_axis, y_axis, z_axis);
    return glm::quat_cast(rot_mtx);
}

std::pair<float, float> Math::plane_angles(ViewPlane plane) {
    switch (plane) {
        case ViewPlane::XY: return { glm::half_pi<float>(), 0.0f }; // +Z
        case ViewPlane::YZ: return { 0.0f, 0.0f }; // +X
        case ViewPlane::XZ: return { glm::half_pi<float>(), -glm::half_pi<float>() / 2.0f }; // +Y
        default: return { 0.8f, 0.3f }; // arbitrary for 3D
    }
}
