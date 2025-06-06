#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <imgui.h>


class Camera;
class Chain;
class Input;


class Overlay {
public:
    Overlay(std::shared_ptr<Camera> camera, std::shared_ptr<Chain> chain);

    // Draws all ImGui UI and overlays, returns true if view/camera changed
    bool draw_menu(Input& input);

    // Draws 2D/3D overlays (axes, grid, etc.)
    void draw_overlays();

    // Retrieves a font by name
    ImFont* get_font(const std::string& name) const;

    // Add getter for chain visibility in 3D
    bool hide_chain() const { return hide_chain_; }

private:
    // Loads all fonts
    void load_fonts();

private:
    std::shared_ptr<Chain> chain_;
    std::shared_ptr<Camera> camera_;
    
    std::unordered_map<std::string, ImFont*> fonts_;

    bool hide_chain_{false};
};