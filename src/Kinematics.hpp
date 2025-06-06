#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Main.hpp"
#include "Math.hpp"
#include "Camera.hpp"



class Kinematics {
public:
    static void forward_kinematics(std::vector<Joint>& joints, const glm::vec3& root_pos, const glm::quat& root_quat);
    static void rotate_joints(std::vector<Joint>& joints, const glm::quat& root_quat);
    static void update_segment_lengths(std::vector<Joint>& joints);
};
