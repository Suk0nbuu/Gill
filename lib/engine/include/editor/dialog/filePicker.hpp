#pragma once
#include <optional>
#include <filesystem>

namespace FilePicker {
    std::optional<std::filesystem::path> PickImageFile();
    std::optional<std::filesystem::path> PickModelFile();
}