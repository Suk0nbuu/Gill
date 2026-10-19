#include "core/project/projectPath.hpp"
#include <system_error>

bool ProjectPaths::IsSafeRelative(const std::string& s) {
    if (s.empty() || s.find('\\') != std::string::npos) return false;   // callers handle "empty = none" themselves
    const std::filesystem::path p(s);
    if (p.is_absolute() || p.has_root_name() || p.has_root_directory()) return false;
    for (const auto& part : p) if (part == "..") return false;
    return true;
}

std::string ProjectPaths::ImportFile(const std::filesystem::path& src, const std::string& subfolder, std::string& err) const {
    namespace fs = std::filesystem;
    if (!HasRoot())                  { err = "no project open"; return {}; }
    if (!IsSafeRelative(subfolder))  { err = "bad import folder"; return {}; }

    std::error_code ec;
    if (!fs::is_regular_file(src, ec)) { err = "file not found: " + src.string(); return {}; }

    const fs::path dir = m_root / "assets" / subfolder;
    fs::create_directories(dir, ec);
    if (ec) { err = "cannot create " + dir.string(); return {}; }

    fs::path dst = dir / src.filename();
    for (int i = 1; fs::exists(dst, ec); ++i) {
        if (fs::equivalent(src, dst, ec)) break;                                   // picked from inside the project
        if (fs::file_size(dst, ec) == fs::file_size(src, ec)) break;               // same name and size: reuse, no duplicate
        dst = dir / (src.stem().string() + "_" + std::to_string(i) + src.extension().string());
    }
    if (!fs::exists(dst, ec)) {
        fs::copy_file(src, dst, ec);
        if (ec) { err = "copy failed: " + ec.message(); return {}; }
    }
    return fs::relative(dst, m_root, ec).generic_string();
}