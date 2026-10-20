#include "render/shader/shader.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <mathpp.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "render/data/bindings.hpp"





void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()),static_cast<int> (value));
}
void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()),value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()),value);
}

void Shader::setMat4f(const std::string &name, const mathpp::mat4f& matrix ) const {
    const float* p = &matrix.col[0][0];
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()),1,GL_FALSE,p);
}

void Shader::setVec2f(const std::string &name, const mathpp::vec2f& vector) const {
    const float* p = &vector.x;
    glUniform2fv(glGetUniformLocation(m_ID, name.c_str()),1,p);
}

void Shader::setVec3f(const std::string &name, const mathpp::vec3f &vector) const {
    const float*p = &vector.x;
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()),1,p);
}
void Shader::Use() const {
    glUseProgram(m_ID);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

Shader::Shader(Shader&& other) noexcept
    : m_ID(other.m_ID), m_valid(other.m_valid), m_error(std::move(other.m_error)) {
    other.m_ID = 0;
    other.m_valid = false;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        glDeleteProgram(m_ID);
        m_ID = other.m_ID;
        m_valid = other.m_valid;
        m_error = std::move(other.m_error);
        other.m_ID = 0;
        other.m_valid = false;
    }
    return *this;
}

void Shader::setMat3f(const std::string &name, const mathpp::mat3f &matrix) const {
    const float* p = &matrix.col[0][0];
    glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()),1,GL_FALSE,p);
}




Shader::Shader(const std::string& vertPath, const std::string& fragPath, const std::string& defines) {
    Build(LoadShaderSourceWithIncludes(vertPath), LoadShaderSourceWithIncludes(fragPath),
          defines, vertPath + " | " + fragPath);
}

Shader Shader::FromGeneratedFragment(const std::string& vertPath, const std::string& fragSource,const std::string& includeDir, const std::string& defines) {
    Shader s;
    std::istringstream in(fragSource);
    s.Build(LoadShaderSourceWithIncludes(vertPath),
            ExpandIncludes(in, includeDir, "<generated>"),
            defines, "generated fragment");
    return s;
}

void Shader::Build(std::string vertStr, std::string fragStr, const std::string& defines, const std::string& label) {
    if (!defines.empty()) {
        vertStr = InjectDefines(vertStr, defines);
        fragStr = InjectDefines(fragStr, defines);
    }
    if (vertStr.empty()) m_error += "Vertex source is empty (" + label + ")\n";
    if (fragStr.empty()) m_error += "Fragment source is empty (" + label + ")\n";

    auto compile = [&](GLenum type, const std::string& src, const char* what) -> unsigned int {
        unsigned int id = glCreateShader(type);
        const char* code = src.c_str();
        glShaderSource(id, 1, &code, nullptr);
        glCompileShader(id);
        int ok = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[2048];
            glGetShaderInfoLog(id, sizeof(log), nullptr, log);
            m_error += std::string(what) + " shader compile error (" + label + "):\n" + log + "\n";
        }
        return id;
    };
    unsigned int vs = compile(GL_VERTEX_SHADER, vertStr, "Vertex");
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragStr, "Fragment");

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vs);
    glAttachShader(m_ID, fs);
    glLinkProgram(m_ID);
    glValidateProgram(m_ID);
    int linked = 0;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[2048];
        glGetProgramInfoLog(m_ID, sizeof(log), nullptr, log);
        m_error += "Shader program link error (" + label + "):\n" + log + "\n";
    }
    m_valid = (linked != 0);

    auto bindBlock = [&](const char* name, unsigned binding) {
        GLuint idx = glGetUniformBlockIndex(m_ID, name);
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(m_ID, idx, binding);
    };
    bindBlock("Camera", kCameraBinding);
    bindBlock("Skin",   kSkinBinding);
    bindBlock("Lights", kLightBinding);

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!m_error.empty()) std::cerr << m_error;
}

std::string Shader::InjectDefines(const std::string& source, const std::string& defines) {
    size_t versionEnd = source.find('\n');
    if (versionEnd == std::string::npos) return defines + source;
    return source.substr(0, versionEnd + 1) + defines + source.substr(versionEnd + 1);
}

std::string Shader::ExpandIncludes(std::istream& in, const std::filesystem::path& baseDir, const std::string& label) {
    std::stringstream result;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("#include", 0) == 0) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            if (firstQuote == std::string::npos || lastQuote == firstQuote) {
                std::cerr << "Malformed #include in " << label << ": " << line << std::endl;
                continue;
            }
            std::string includeName = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            result << LoadShaderSourceWithIncludes((baseDir / includeName).string());   // rekursif
        } else {
            result << line << "\n";
        }
    }
    return result.str();
}

std::string Shader::LoadShaderSourceWithIncludes(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    return ExpandIncludes(file, std::filesystem::path(path).parent_path(), path);
}