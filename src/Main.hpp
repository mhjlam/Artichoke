#pragma once

#include <vector>
#include <cstddef>
#include <array>
#include <string>

#include <glm/glm.hpp>

#include "Math.hpp"


/* Defines */

#define _____if if


/* Constants */

constexpr const size_t WINDOW_WIDTH = 1280;
constexpr const size_t WINDOW_HEIGHT = 720;


/* Enums */

enum class ViewPlane 
{ 
    XY,                     // Front view (looking down -Z, orthographic)
    YZ,                     // Side view (looking down -X, orthographic)
    XZ,                     // Top view (looking down -Y, orthographic)
    XYZ                     // 3D view (looking down -Z, with perspective)
};


/* Struct */

struct Joint
{
    glm::vec3 pos;          // World position (computed by FK)
    glm::quat rot;          // World rotation (computed by FK)
    glm::quat local_rot;    // Local rotation (user-controlled, relative to parent)
    float length;           // Length of the bone segment to the next joint (0 if last joint)
};

struct Bone {
    glm::vec3 a, b, ab, ab_dir;
    
    Bone(const std::vector<Joint>& joints, size_t idx) {
        a = joints[idx].pos;
        b = joints[idx + 1].pos;
        ab = b - a;
        ab_dir = glm::normalize(ab);
    }
    glm::vec3 binormal(const glm::vec3& up) const {
        return glm::normalize(glm::cross(up, ab_dir));
    }
    glm::vec3 normal(const glm::vec3& up) const {
        return glm::normalize(glm::cross(ab_dir, binormal(up)));
    }
    glm::vec3 point_at(float t) const {
        return a + t * ab;
    }
};

struct Tendon
{
    size_t bone_idx;        // Index of the bone this tendon is attached to
    float t;                // Parameter t in [0, 1] indicating the position along the bone
    glm::vec2 local_offset; // (normal, binormal) in the segment's local frame
    glm::vec3 up;           // Up vector used at creation
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct PlaneAxisInfo {
    const char* h_label;
    const char* v_label;
    glm::vec3 h_color;
    glm::vec3 v_color;
};

inline PlaneAxisInfo get_plane_axis_info(ViewPlane plane) {
    switch (plane) {
        case ViewPlane::XY:
            return { "X", "Y", {0.75f, 0.15f, 0.20f}, {0.10f, 0.50f, 0.20f} };
        case ViewPlane::YZ:
            return { "Z", "Y", {0.22f, 0.40f, 0.90f}, {0.10f, 0.50f, 0.20f} };
        case ViewPlane::XZ:
            return { "X", "Z", {0.75f, 0.15f, 0.20f}, {0.22f, 0.40f, 0.90f} };
        default:
            return { "", "", {1, 1, 1}, {1, 1, 1} };
    }
}
