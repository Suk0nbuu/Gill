#include "editor/dialog/filePicker.hpp"
#include "nfd.h"

namespace  FilePicker {
    std::optional<std::filesystem::path> PickImageFile() {
        nfdu8char_t* out = nullptr;
        nfdu8filteritem_t filters[1] = {{"Images", "png,jpg,jpeg,tga,bmp"}};
        if (NFD_OpenDialogU8(&out, filters, 1, nullptr) != NFD_OKAY) return std::nullopt;
        std::filesystem::path p = std::filesystem::u8path(reinterpret_cast<const char*>(out));
        NFD_FreePathU8(out);
        return p;
    }
    std::optional<std::filesystem::path> PickModelFile() {
        nfdu8char_t* out = nullptr;
        nfdu8filteritem_t filters[1] = {{"Models","glb,fbx,obj"}};
        if (NFD_OpenDialogU8(&out, filters, 1, nullptr) != NFD_OKAY) return std::nullopt;
        std::filesystem::path p = std::filesystem::u8path(reinterpret_cast<const char*>(out));
        NFD_FreePathU8(out);
        return p;
    }
}