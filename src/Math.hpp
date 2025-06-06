#pragma once

#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class ViewPlane;

class Math {
public:
    static glm::quat axis_angle_quat(const glm::vec3& axis, float angle_deg);
    static glm::quat compute_frame_quat(const glm::vec3& from, const glm::vec3& to, glm::vec3 up = glm::vec3(0, 1, 0));
    static glm::quat compute_frame_quat_from_dir(const glm::vec3& dir, glm::vec3 up = glm::vec3(0, 1, 0));
    static std::pair<float, float> plane_angles(ViewPlane plane);
};
