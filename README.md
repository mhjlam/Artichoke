# Artichoke: Articulated Chain Viewer

Artichoke is a modern OpenGL-based application for visualizing and interactively manipulating 2D and 3D articulated chains. It features efficient rendering, intuitive controls, and a flexible UI powered by ImGui.

## Features

- 2D and 3D articulated chain visualization.
- Interactive joint and point manipulation.
- Modern OpenGL rendering (GL 3.3+, shaders, VAOs, VBOs).
- Grid and background gradient.
- ImGui-based menu and overlays.
- Efficient, batched rendering for all geometry.

## Getting Started

### Prerequisites

- C++17+
- CMake 3.10+
- OpenGL 3.3+
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [ImGui](https://github.com/ocornut/imgui)

### Build Instructions

```sh
# Clone the repository
$ git clone https://github.com/yourusername/Artichoke.git
$ cd Artichoke

# Create build directory and compile
$ mkdir build && cd build
$ cmake ..
$ cmake --build .

# Run the application
$ ./Artichoke
```

## Usage

- **Left Click**: Select or drag joints (in 2D views).
- **Right Click**: Deselect joint.
- **Middle Click**: Pan view.
- **Scroll Wheel**: Rotate selected joint (hold X/Y/Z in 3D).
- Use the ImGui menu to switch views, adjust bone lengths, add points, and toggle visibility.

## Implementation Overview

### Data Structures

- **Joint**: Stores position, rotation (world and local), and bone length.
- **Tendon**: Represents an attached point on a link, with local offset and up vector.
- **Chain**: Manages a vector of joints and tendons, supports forward kinematics and interactive manipulation.
- **Camera**: Handles 2D/3D view transforms and user navigation.
- **Grid**: Renders background grid and gradient in 2D/3D using OpenGL buffers and shaders.

### Kinematics and Manipulation

- **Forward Kinematics**: Computes world positions and rotations for all joints based on local rotations and bone lengths.
- **2D Manipulation**: Drag joints in the plane, rotate by mouse wheel.
- **3D Manipulation**: Rotate joints about X/Y/Z axes (quaternion-based).
- **Point Attachment**: Attach points to links in 2D, stored with local offset and up vector for stability.

### Rendering

- Uses modern OpenGL (GL 3.3+) with VAOs, VBOs, and GLSL shaders (`res/color.vert`, `res/color.frag`).
- All geometry (bones, joints, axes, points) is batched and rendered efficiently.
- Grid and background gradient are drawn using the `Grid` class.
- Global axes are rendered in 3D view.
- ImGui provides an interactive overlay for all controls.

### File Structure

- `src/Chain.cpp`, `Chain.hpp`: Articulated chain logic and rendering.
- `src/Camera.cpp`, `Camera.hpp`: Camera/view logic.
- `src/Grid.cpp`, `Grid.hpp`: Grid and gradient rendering.
- `src/Renderer.cpp`, `Renderer.hpp`: Main application loop and rendering orchestration.
- `src/Shader.cpp`, `Shader.hpp`: GLSL shader management.
- `src/Buffer.cpp`, `Buffer.hpp`: OpenGL buffer/VAO abstraction.
- `src/Overlay.cpp`, `Overlay.hpp`: ImGui overlay and menu logic.
- `res/color.vert`, `res/color.frag`: GLSL shaders.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
