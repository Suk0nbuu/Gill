#include "render/shader/shader.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <mathpp.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "render/data/bindings.hpp"


std::string Shader::LoadShaderSourceWithIncludes(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }

    std::stringstream result;
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("#include", 0) == 0) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            if (firstQuote == std::string::npos || lastQuote == firstQuote) {
                std::cerr << "Malformed #include in " << path << ": " << line << std::endl;
                continue;
            }
            std::string includeName = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            std::string includePath = std::filesystem::path(path).parent_path().string() + "/" + includeName;
            result << LoadShaderSourceWithIncludes(includePath); // recursive — nested includes work too
        } else {
            result << line << "\n";
        }
    }
    return result.str();
}



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

Shader::Shader(Shader&& other) noexcept : m_ID(other.m_ID) {
    other.m_ID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        glDeleteProgram(m_ID);   // clean up whatever this Shader currently owns
        m_ID = other.m_ID;
        other.m_ID = 0;
    }
    return *this;
}

void Shader::setMat3f(const std::string &name, const mathpp::mat3f &matrix) const {
    const float* p = &matrix.col[0][0];
    glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()),1,GL_FALSE,p);
}




Shader::Shader(const std::string& vertPath, const std::string& fragPath, const std::string& defines) {
    std::string vertStr = LoadShaderSourceWithIncludes(vertPath);
    std::string fragStr = LoadShaderSourceWithIncludes(fragPath);

    if (!defines.empty()) {
        vertStr = InjectDefines(vertStr, defines);
        fragStr = InjectDefines(fragStr, defines);
    }

    if (vertStr.empty()) std::cerr << "Vertex shader source is empty: " << vertPath << std::endl;
    if (fragStr.empty()) std::cerr << "Fragment shader source is empty: " << fragPath << std::endl;

    const char* vertCode = vertStr.c_str();
    const char* fragCode = fragStr.c_str();

    unsigned int vertexShader, fragmentShader;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertCode, NULL);
    int successv;
    char infoLogv[512];

    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successv);
    if (!successv) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLogv);
        std::cerr << "Vertex shader compile error:\n" << infoLogv << std::endl;
    }
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragCode, NULL);
    int successf;
    char infoLogf[512];

    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successf);
    if (!successf) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLogf);
        std::cerr << "Fragment shader compile error:\n" << infoLogf << std::endl;
    }





    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    glLinkProgram(m_ID);
    glValidateProgram(m_ID);
    int successLink;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &successLink);
    if (!successLink) {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::cerr << "Shader program link error:\n" << infoLog << std::endl;
    }
    auto bindBlock = [&](const char* name, unsigned binding) {
        GLuint idx = glGetUniformBlockIndex(m_ID, name);
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(m_ID, idx, binding);
    };
    bindBlock("Camera", kCameraBinding);
    bindBlock("Skin",   kSkinBinding);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

std::string Shader::InjectDefines(const std::string& source, const std::string& defines) {
    size_t versionEnd = source.find('\n');
    if (versionEnd == std::string::npos) return defines + source;
    return source.substr(0, versionEnd + 1) + defines + source.substr(versionEnd + 1);
}