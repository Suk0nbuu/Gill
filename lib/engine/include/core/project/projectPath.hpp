#pragma once
#include <filesystem>
#include <string>

class ProjectPaths {
public:
    void SetRoot(const std::filesystem::path& root) { m_root = root; }
    bool HasRoot() const { return !m_root.empty(); }
    const std::filesystem::path& Root() const { return m_root; }

    [[nodiscard]]std::filesystem::path Resolve(const std::string& rel) const { return m_root / rel; }
    [[nodiscard]]std::filesystem::path TexturesDir() const { return m_root / "assets" / "textures"; }
    [[nodiscard]]std::filesystem::path ModelsDir()   const { return m_root / "assets" / "models"; }

    static bool IsSafeRelative(const std::string& s);              // moved out of the serializer's anonymous namespace
    std::string ImportFile(const std::filesystem::path& src, const std::string& subfolder, std::string& err) const;  // your ImportAsset
private:
    std::filesystem::path m_root;
};