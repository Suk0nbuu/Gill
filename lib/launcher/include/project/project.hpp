#pragma once
#include <filesystem>
#include <optional>
#include <string>

namespace PathUtil {
    // Paths cross the ImGui/NFD/file boundary as UTF-8; std::filesystem needs them converted on Windows.
    inline std::string ToUtf8(const std::filesystem::path& p) {
        auto s = p.u8string();
        return std::string(s.begin(), s.end());
    }
    inline std::filesystem::path FromUtf8(const std::string& s) {
        return std::filesystem::path(std::u8string(s.begin(), s.end()));
    }
}

// A Gill project = a folder containing a manifest (project.gproj) plus Assets/ Scenes/ Cache/.
// Everything stored *inside* a project is a path relative to the project root.
class Project {
public:
    static constexpr const char* kManifestName = "project.gproj";
    static constexpr int kVersion = 1;

    static bool IsValidName(const std::string& name);

    // Create the base project at parent/name. On failure returns nullopt and fills error.
    static std::optional<Project> Create(const std::filesystem::path& parent, const std::string& name, std::string& error);
    // Open an existing project folder (the one that holds project.gproj).
    static std::optional<Project> Open(const std::filesystem::path& root, std::string& error);
    // Erase a project folder from disk. Refuses anything that doesn't look like a project.
    static bool Delete(const std::filesystem::path& root, std::string& error);

    bool Save() const;   // rewrites the manifest (atomic: temp file, then rename)

    const std::string& GetName() const { return m_name; }
    const std::filesystem::path& GetRoot() const { return m_root; }
    std::filesystem::path ScenePath() const { return m_root / "Scenes" / "Main.json"; }
    std::filesystem::path ResolveAsset(const std::string& rel) const;                 // rel -> absolute
    std::optional<std::string> MakeRelative(const std::filesystem::path& abs) const;  // nullopt if outside project

private:
    std::string m_name;
    std::filesystem::path m_root;

};