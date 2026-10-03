#pragma once
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>
#include "project/project.hpp"

struct ProjectEntry {
    std::string name;
    std::filesystem::path path;
    std::time_t lastOpened = 0;
    bool missing = false;   // folder or manifest no longer on disk
};

// The launcher's list of known projects (like Godot's project manager list).
// Stored per-user, outside any project. "Remove" only forgets an entry; Project::Delete erases disk.
class ProjectList {
public:
    explicit ProjectList(std::filesystem::path file) : m_file(std::move(file)) {}

    static std::filesystem::path DefaultFile();   // %APPDATA%/Gill/projects.txt (or ~/.config/Gill)

    void Load();
    bool Save() const;
    void Refresh();                       // recompute 'missing' flags
    void Add(const Project& project);     // add, or bump to front + update lastOpened
    void Remove(size_t index);            // forget only
    const std::vector<ProjectEntry>& GetEntries() const { return v_entries; }

private:
    std::filesystem::path m_file;
    std::vector<ProjectEntry> v_entries;
};