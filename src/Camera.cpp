#include "Camera.hpp"

#include <cmath>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Main.hpp"
#include "Chain.hpp"
#include "Math.hpp"

namespace {
    glm::vec3 calc_pan3d(const Camera& cam) {
        switch (cam.view_plane) {
            case ViewPlane::XY: return glm::vec3(cam.pan_offset_2d.x, cam.pan_offset_2d.y, 0.0f);
            case ViewPlane::YZ: return glm::vec3(0.0f, cam.pan_offset_2d.y, cam.pan_offset_2d.x);
            case ViewPlane::XZ: return glm::vec3(cam.pan_offset_2d.x, 0.0f, cam.pan_offset_2d.y);
            default: return glm::vec3(0.0f);
        }
    }
    glm::vec3 calc_offset(float ortho_half_width, float ortho_half_height, float margin_world_x, float margin_world_y, ViewPlane plane) {
        switch (plane) {
            case ViewPlane::XY: return glm::vec3(ortho_half_width - margin_world_x, ortho_half_height - margin_world_y, 0.0f);
            case ViewPlane::YZ: return glm::vec3(0.0f, ortho_half_height - margin_world_y, ortho_half_width - margin_world_x);
            case ViewPlane::XZ: return glm::vec3(ortho_half_width - margin_world_x, 0.0f, ortho_half_height - margin_world_y);
            default: return glm::vec3(0.0f);
        }
    }
}

Camera::Camera() : view_plane(ViewPlane::XY), last_plane(ViewPlane::XY), animating(false), distance(600.0f), pan_offset_2d(0.0f), pan_offset(0.0f), target(0.0f) {
    auto [ta, tp] = Math::plane_angles(ViewPlane::XY);
    angle = target_angle = ta;
    pitch = target_pitch = tp;
}

void Camera::update(const Input& input, int active_joint)
{
    // Detect plane change
    if (view_plane != last_plane) {
        auto [ta, tp] = Math::plane_angles(view_plane);

        if (view_plane != ViewPlane::XYZ) {
            angle = target_angle = ta;
            pitch = target_pitch = tp;
            animating = false;
            distance = 600.0f;
            pan_offset_2d = glm::vec2(0.0f);
        }
        else {
            angle = target_angle = glm::quarter_pi<float>();
            pitch = target_pitch = glm::asin(1.0f / std::sqrt(3.0f));
            animating = false;
            distance = 600.0f;
            pan_offset = glm::vec3(0.0f);
        }
        last_plane = view_plane;
    }

    // Animate if needed
    if (animating) {
        float delta = animation_speed * input.delta_time();
        float da = target_angle - angle;
        if (da > glm::pi<float>()) da -= glm::two_pi<float>();
        if (da < -glm::pi<float>()) da += glm::two_pi<float>();
        angle += da * delta;
        pitch += (target_pitch - pitch) * delta;
        if (std::abs(da) < 0.01f && std::abs(target_pitch - pitch) < 0.01f) {
            angle = target_angle;
            pitch = target_pitch;
            animating = false;
        }
    }

    // User input (disable during animation)
    if (!animating) {
        // Free 3D rotation with left mouse button when unconstrained
        if (input.mouse_down(0) && view_plane == ViewPlane::XYZ) {
            glm::vec2 delta = input.mouse_delta();
            angle -= delta.x * 0.005f;
            pitch += delta.y * 0.005f;
            pitch = glm::clamp(pitch, -glm::half_pi<float>() + 0.1f, glm::half_pi<float>() - 0.1f);
        }
        // Zoom with right mouse button (all views) -- DISABLE in 2D views
        if (input.mouse_down(1) && view_plane == ViewPlane::XYZ) {
            glm::vec2 delta = input.mouse_delta();
            distance += delta.y * 2.0f;
            distance = glm::clamp(distance, 100.0f, 2000.0f);
        }
        // Pan with middle mouse button (all views)
        if (input.mouse_down(2)) {
            glm::vec2 delta = input.mouse_delta();
            float panSpeed = distance / 600.0f;

            if (view_plane == ViewPlane::XYZ) {
                float angle_offset = angle - glm::radians(90.0f);
                float pitch_offset = pitch - glm::radians(25.0f);

                glm::vec3 forward = glm::normalize(glm::vec3(
                    sin(angle_offset) * cos(pitch_offset),
                    sin(pitch_offset),
                    cos(angle_offset) * cos(pitch_offset)
                ));
                glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
                glm::vec3 up_vec = glm::normalize(glm::cross(forward, right));

                pan_offset += -right * (float)delta.x * panSpeed + up_vec * (float)delta.y * panSpeed;
            }
        }
        // Zoom with mouse wheel (disable in 2D views and when a joint is selected)
        if (input.mouse_wheel() != 0.0f && view_plane == ViewPlane::XYZ && active_joint < 0) {
            distance -= input.mouse_wheel() * 40.0f;
            distance = glm::clamp(distance, 100.0f, 2000.0f);
        }
    }
}

glm::mat4 Camera::get_view() const
{
    glm::vec3 up(0, 1, 0);
    glm::vec3 camPos, center;
    glm::vec3 pan3d(0.0f);

    const float margin_px = 16.0f;
    float win_w = static_cast<float>(WINDOW_WIDTH);
    float win_h = static_cast<float>(WINDOW_HEIGHT);
    float aspect = win_w / win_h;
    float ortho_half_height = distance * 0.5f;
    float ortho_half_width = ortho_half_height * aspect;
    float margin_world_x = margin_px * (2.0f * ortho_half_width) / win_w;
    float margin_world_y = margin_px * (2.0f * ortho_half_height) / win_h;

    switch (view_plane) {
        case ViewPlane::XY:
        case ViewPlane::YZ:
        case ViewPlane::XZ:
        {
            pan3d = calc_pan3d(*this);
            glm::vec3 offset = calc_offset(ortho_half_width, ortho_half_height, margin_world_x, margin_world_y, view_plane);
            glm::vec3 cam_dir;
            if (view_plane == ViewPlane::XY)      cam_dir = glm::vec3(0, 0, distance);
            else if (view_plane == ViewPlane::YZ) cam_dir = glm::vec3(distance, 0, 0);
            else                                  cam_dir = glm::vec3(0, distance, 0);
            camPos = target + pan3d + offset + cam_dir;
            center = target + pan3d + offset;
            up = (view_plane == ViewPlane::XZ) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
            break;
        }
        case ViewPlane::XYZ:
        default:
        {
            constexpr float fov = glm::radians(45.0f);
            float tan_half_fov = std::tan(fov * 0.5f);
            float view_height = 2.0f * distance * tan_half_fov;
            float view_width = view_height * aspect;
            float angle_offset = angle - glm::radians(90.0f);
            float pitch_offset = pitch - glm::radians(25.0f);

            glm::vec3 forward = glm::normalize(glm::vec3(
                sin(angle_offset) * cos(pitch_offset),
                sin(pitch_offset),
                cos(angle_offset) * cos(pitch_offset)
            ));
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
            glm::vec3 up_vec = glm::normalize(glm::cross(forward, right));
            glm::vec3 offset = 0.25f * view_width * right + 0.25f * view_height * up_vec;

            camPos = target + pan_offset +
                glm::vec3(
                    distance * cos(pitch_offset) * sin(angle_offset),
                    distance * sin(pitch_offset),
                    distance * cos(pitch_offset) * cos(angle_offset)
                );
            center = target + pan_offset + offset;
            camPos += offset;
            up = up_vec;
            break;
        }
    }

    return glm::lookAt(camPos, center, up);
}

glm::mat4 Camera::get_proj(float aspect) const
{
    float orthoHalfSize = distance * 0.5f;
    switch (view_plane) {
        case ViewPlane::XY:
        case ViewPlane::YZ:
        case ViewPlane::XZ:
            return glm::ortho(
                -orthoHalfSize * aspect, orthoHalfSize * aspect,
                -orthoHalfSize, orthoHalfSize,
                -4000.0f, 4000.0f
            );
        case ViewPlane::XYZ:
        default:
            if (perspective_) {
                return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 4000.0f);
            }
            return glm::ortho(
                -orthoHalfSize * aspect, orthoHalfSize * aspect,
                -orthoHalfSize, orthoHalfSize,
                -4000.0f, 4000.0f
            );
    }
}

glm::vec3 Camera::get_eye() const {
    return target + pan_offset +
        glm::vec3(
            distance * cos(pitch) * sin(angle),
            distance * sin(pitch),
            distance * cos(pitch) * cos(angle)
        );
}
