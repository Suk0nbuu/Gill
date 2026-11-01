#pragma once
#include <string>

#include "mathpp.hpp"
#include <filesystem>



class Shader {
public:
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    Shader(const std::string& vertPath, const std::string& fragPath, const std::string& defines = "");
    static Shader FromGeneratedFragment(const std::string& vertPath, const std::string& fragSource,
                                        const std::string& includeDir, const std::string& defines = "");
    bool IsValid() const { return m_valid; }
    const std::string& Error() const { return m_error; }




    void Use() const;
    void SetBool(const std::string& name, bool value) const;
    void SetUInt(const std::string& name, unsigned int value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetMat4f(const std::string& name, const mathpp::mat4f& matrix) const;
    void SetVec2f(const std::string& name, const mathpp::vec2f& vector) const;
    void SetVec3f(const std::string& name, const mathpp::vec3f& vector) const;
    void SetMat3f(const std::string& name, const mathpp::mat3f& matrix) const;

private:
    static std::string LoadShaderSourceWithIncludes(const std::string& path);
    static std::string InjectDefines(const std::string& source, const std::string& defines);
    unsigned int m_ID = 0;
    void Build(std::string vertStr, std::string fragStr, const std::string& defines, const std::string& label);
    static std::string ExpandIncludes(std::istream& in, const std::filesystem::path& baseDir, const std::string& label);
    Shader() = default;
    bool m_valid = false;
    std::string m_error;

};

