#include "project/project.hpp"
#include <cctype>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {
    fs::path ManifestPath(const fs::path& root) { return root / Project::kManifestName; }
}

bool Project::IsValidName(const std::string& name) {
    if (name.empty() || name.size() > 64) return false;
    if (name.front() == '.' || name.back() == '.' || name.back() == ' ') return false;
    for (char c : name) {
        if (static_cast<unsigned char>(c) < 32) return false;
        if (std::string("<>:\"/\\|?*").find(c) != std::string::npos) return false;
    }
    // Windows reserved device names (CON, NUL, COM1, ... even with an extension)
    std::string stem;
    for (char c : name.substr(0, name.find('.'))) stem += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL") return false;
    if (stem.size() == 4 && (stem.rfind("COM", 0) == 0 || stem.rfind("LPT", 0) == 0) && stem[3] >= '1' && stem[3] <= '9') return false;
    return true;
}

std::optional<Project> Project::Create(const fs::path& parent, const std::string& name, std::string& error) {
    if (!IsValidName(name)) { error = "Invalid project name."; return std::nullopt; }
    std::error_code ec;
    if (!fs::is_directory(parent, ec)) { error = "Location does not exist."; return std::nullopt; }

    fs::path root = parent / name;
    bool existed = fs::exists(root, ec);
    if (existed && !fs::is_empty(root, ec)) { error = "Folder already exists and is not empty."; return std::nullopt; }

    fs::create_directories(root / "Assets", ec);
    fs::create_directories(root / "Scenes", ec);
    fs::create_directories(root / "Cache", ec);

    Project project;
    project.m_name = name;
    project.m_root = fs::weakly_canonical(root, ec);


    std::ofstream scene(root / "Scenes" / "Main.json");
    scene << "{\"version\":1,\"entities\":[]}\n";
    scene.close();

    if (ec || !scene || !project.Save()) {
        error = "Failed to write project files.";
        if (!existed) fs::remove_all(root, ec);   // only clean up what we created
        return std::nullopt;
    }
    return project;
}

std::optional<Project> Project::Open(const fs::path& root, std::string& error) {
    std::ifstream file(ManifestPath(root));
    if (!file) { error = "No project.gproj found in that folder."; return std::nullopt; }

    int version = 0;
    std::string name;
    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq), value = line.substr(eq + 1);
        if (key == "gill_project") { try { version = std::stoi(value); } catch (...) {} }
        else if (key == "name") name = value;
    }
    if (version <= 0) { error = "project.gproj is corrupt."; return std::nullopt; }
    if (version > kVersion) { error = "Project was made with a newer version of Gill."; return std::nullopt; }

    std::error_code ec;
    Project project;
    project.m_root = fs::weakly_canonical(root, ec);
    project.m_name = name.empty() ? project.m_root.filename().string() : name;

    fs::create_directories(project.m_root / "Assets", ec);   // heal missing folders
    fs::create_directories(project.m_root / "Scenes", ec);
    fs::create_directories(project.m_root / "Cache", ec);
    return project;
}

bool Project::Delete(const fs::path& root, std::string& error) {
    std::error_code ec;
    fs::path canonical = fs::weakly_canonical(root, ec);
    if (!fs::exists(ManifestPath(canonical), ec)) { error = "Not a Gill project folder."; return false; }
    // Never delete something shallow like C:\ or C:\Users
    size_t depth = 0;
    for (const auto& part : canonical.relative_path()) { (void)part; ++depth; }
    if (depth < 2) { error = "Refusing to delete a top-level folder."; return false; }
    fs::remove_all(canonical, ec);
    if (ec) { error = "Could not delete folder: " + ec.message(); return false; }
    return true;
}

bool Project::Save() const {
    fs::path finalPath = ManifestPath(m_root);
    fs::path tmpPath = finalPath;
    tmpPath += ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::trunc);
        if (!out) return false;
        out << "gill_project=" << kVersion << "\n";
        out << "name=" << m_name << "\n";
        if (!out) return false;
    }
    std::error_code ec;
    fs::rename(tmpPath, finalPath, ec);
    return !ec;
}

fs::path Project::ResolveAsset(const std::string& rel) const {
    return m_root / fs::path(rel);
}

std::optional<std::string> Project::MakeRelative(const fs::path& abs) const {
    std::error_code ec;
    fs::path rel = fs::relative(fs::weakly_canonical(abs, ec), m_root, ec);
    if (ec || rel.empty() || *rel.begin() == "..") return std::nullopt;
    return rel.generic_string();
}