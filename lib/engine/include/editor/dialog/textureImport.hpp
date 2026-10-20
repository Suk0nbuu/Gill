#pragma once
#include <filesystem>
#include <string>
std::string ImportTexture(const std::filesystem::path& src, const std::filesystem::path& root, std::string& err);