#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>


Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    load(vertexPath, fragmentPath);
}

Shader::~Shader()
{
    cleanup();
}

std::string Shader::load_file(const char* path)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Shader file not found: " << path << std::endl;
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool Shader::load(const char* vertexPath, const char* fragmentPath)
{
    cleanup();
    std::string vertSrc = load_file(vertexPath);
    std::string fragSrc = load_file(fragmentPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    GLuint vs = compile(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragSrc.c_str());
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint status = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &status);
    if (!status) {
        char log[512];
        glGetProgramInfoLog(program_, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << std::endl;
        cleanup();
        return false;
    }
    return true;
}

void Shader::cleanup()
{
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

GLuint Shader::compile(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::setUniform(const char* name, const glm::mat4& value) const
{
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const char* name, const glm::vec3& value) const
{
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::setUniform(const char* name, const glm::vec2& value) const
{
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != -1) glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Shader::setUniform(const char* name, float value) const
{
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != -1) glUniform1f(loc, value);
}

void Shader::setUniform(const char* name, int value) const
{
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::destroy()
{
    cleanup();
}
