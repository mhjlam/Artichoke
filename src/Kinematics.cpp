#include "Kinematics.hpp"
#include "Math.hpp"

// Forward kinematics: propagate positions and rotations
void Kinematics::forward_kinematics(std::vector<Joint>& joints, const glm::vec3& root_pos, const glm::quat& root_quat)
{
    if (joints.empty()) return;
    joints[0].rot = root_quat;
    joints[0].pos = root_pos;
    for (size_t i = 1; i < joints.size(); ++i) {
        joints[i].rot = joints[i - 1].rot * joints[i].local_rot;
        joints[i].pos = joints[i - 1].pos + joints[i - 1].rot * glm::vec3(0, 0, joints[i - 1].length);
    }
}

void Kinematics::rotate_joints(std::vector<Joint>& joints, const glm::quat& root_quat)
{
    if (joints.empty()) return;
    for (size_t i = 1; i < joints.size(); ++i) {
        glm::vec3 dir = glm::normalize(joints[i].pos - joints[i - 1].pos);
        float len = glm::length(joints[i].pos - joints[i - 1].pos);

        glm::quat new_world_rot = Math::compute_frame_quat_from_dir(dir);

        glm::quat parent_world_rot = (i == 1) ? root_quat : joints[i - 1].rot;
        glm::quat new_local_rot = glm::inverse(parent_world_rot) * new_world_rot;

        joints[i - 1].local_rot = new_local_rot;
        joints[i - 1].length = len;
    }
}

void Kinematics::update_segment_lengths(std::vector<Joint>& joints)
{
    for (size_t i = 0; i + 1 < joints.size(); ++i) {
        joints[i].length = glm::length(joints[i + 1].pos - joints[i].pos);
    }
}
