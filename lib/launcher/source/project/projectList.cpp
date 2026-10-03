#include "project/projectList.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

fs::path ProjectList::DefaultFile() {
    fs::path base;
    if (const char* appdata = std::getenv("APPDATA")) base = appdata;
    else if (const char* home = std::getenv("HOME")) base = fs::path(home) / ".config";
    else base = fs::current_path();
    return base / "Gill" / "projects.txt";
}

void ProjectList::Load() {
    v_entries.clear();
    std::ifstream in(m_file);
    std::string line;
    while (std::getline(in, line)) {   // format: path \t lastOpened \t name
        std::stringstream ss(line);
        std::string path, time, name;
        if (!std::getline(ss, path, '\t') || !std::getline(ss, time, '\t')) continue;
        std::getline(ss, name);
        ProjectEntry entry;
        entry.path = PathUtil::FromUtf8(path);
        entry.name = name;
        try { entry.lastOpened = static_cast<std::time_t>(std::stoll(time)); } catch (...) { continue; }
        v_entries.push_back(std::move(entry));
    }
    std::sort(v_entries.begin(), v_entries.end(),
              [](const ProjectEntry& a, const ProjectEntry& b) { return a.lastOpened > b.lastOpened; });
    Refresh();
}

bool ProjectList::Save() const {
    std::error_code ec;
    fs::create_directories(m_file.parent_path(), ec);
    std::ofstream out(m_file, std::ios::trunc);
    if (!out) return false;
    for (const auto& e : v_entries)
        out << PathUtil::ToUtf8(e.path) << '\t' << static_cast<long long>(e.lastOpened) << '\t' << e.name << '\n';
    return static_cast<bool>(out);
}

void ProjectList::Refresh() {
    std::error_code ec;
    for (auto& e : v_entries)
        e.missing = !fs::exists(e.path / Project::kManifestName, ec);
}

void ProjectList::Add(const Project& project) {
    v_entries.erase(std::remove_if(v_entries.begin(), v_entries.end(),
        [&](const ProjectEntry& e) {
            std::error_code ec;
            return fs::equivalent(e.path, project.GetRoot(), ec);
        }), v_entries.end());
    ProjectEntry entry;
    entry.name = project.GetName();
    entry.path = project.GetRoot();
    entry.lastOpened = std::time(nullptr);
    v_entries.insert(v_entries.begin(), std::move(entry));
}

void ProjectList::Remove(size_t index) {
    if (index < v_entries.size()) v_entries.erase(v_entries.begin() + static_cast<long>(index));
}