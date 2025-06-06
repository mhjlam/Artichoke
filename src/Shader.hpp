#pragma once

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>


class Shader
{
public:
    Shader() = default;
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    bool load(const char* vertexPath, const char* fragmentPath);
    void use() const { glUseProgram(program_); }
    void unuse() const { glUseProgram(0); }

    void setUniform(const char* name, const glm::mat4& value) const;
    void setUniform(const char* name, const glm::vec3& value) const;
    void setUniform(const char* name, const glm::vec2& value) const;
    void setUniform(const char* name, float value) const;
    void setUniform(const char* name, int value) const;
    void set_mvp(const glm::mat4& mvp) const { setUniform("uMVP", mvp); }

    GLuint id() const { return program_; }
    void destroy();

private:
    GLuint program_ = 0;
    void cleanup();
    GLuint compile(GLenum type, const char* src);
    std::string load_file(const char* path);
};
