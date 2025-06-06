#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

#include "Main.hpp"
#include "Input.hpp"


class Camera
{
public:
    Camera();

    void update(const Input& input, int active_joint);

    glm::mat4 get_view() const;
    glm::mat4 get_proj(float aspect) const;
    glm::vec3 get_eye() const;

public:
    bool perspective_{ false };

    glm::vec3 target = glm::vec3(0);
    glm::vec3 pan_offset = glm::vec3(0);
    float distance = 400.0f;
    float angle = 0.0f;
    float pitch = 0.0f;

    // Plane constraint for view (xy, yz, xz, or free)
    ViewPlane view_plane = ViewPlane::XYZ;
    glm::vec2 pan_offset_2d; // Used for constrained views

    // Animation state
    float target_angle = 0.0f, target_pitch = 0.0f;
    bool animating = false;
    float animation_speed = 12.0f; // Higher = faster
    ViewPlane last_plane = ViewPlane::XYZ;
};