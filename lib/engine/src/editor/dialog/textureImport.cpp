#include "editor/dialog/textureImport.hpp"
#include "stb_image.h"

std::string ImportTexture(const std::filesystem::path& src, const std::filesystem::path& root, std::string& err) {
    int w, h, c;
    if (!stbi_info(src.string().c_str(), &w, &h, &c)) { err = "not a readable image"; return {}; }  // checks content, not extension
    std::error_code ec;
    const auto dir = root / "assets" / "textures";
    std::filesystem::create_directories(dir, ec);
    auto dst = dir / src.filename();
    for (int i = 1; std::filesystem::exists(dst, ec) && std::filesystem::file_size(dst, ec) != std::filesystem::file_size(src, ec); ++i)
        dst = dir / (src.stem().string() + "_" + std::to_string(i) + src.extension().string());
    if (!std::filesystem::exists(dst, ec)) {
        std::filesystem::copy_file(src, dst, ec);
        if (ec) { err = "copy failed: " + ec.message(); return {}; }
    }
    return std::filesystem::relative(dst, root, ec).generic_string();
}