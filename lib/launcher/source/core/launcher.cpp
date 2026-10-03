#include "core/launcher.hpp"
#include "glad/gl.h"
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "../../engine/include/core/window/window.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "nfd.h"

namespace fs = std::filesystem;

Launcher::Launcher() : m_list(ProjectList::DefaultFile()) {}

std::optional<Project> Launcher::Run(Window& window) {
    m_list.Load();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;                                   // launcher layout is fixed, don't write imgui.ini
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!window.ShouldClose() && !m_result) {
        window.PollEvents();
        glClearColor(0.10f, 0.10f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        Draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.SwapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                       // engine never sets it; restore the GL default
    return m_result;
}

void Launcher::Draw() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##launcher", nullptr, flags)) {
        ImGui::TextUnformatted("Gill - Project Manager");
        ImGui::Separator();
        DrawToolbar();
        ImGui::Separator();
        DrawProjectTable();

        // Popups must be opened and drawn in the same window scope, so they live here.
        DrawNewProjectPopup();
        DrawDeletePopup();
        if (m_showError) { ImGui::OpenPopup("Error"); m_showError = false; }
        DrawErrorPopup();
    }
    ImGui::End();
}

void Launcher::DrawToolbar() {
    if (ImGui::Button("New Project")) {
        m_createError.clear();
        std::snprintf(m_locationBuf, sizeof(m_locationBuf), "%s", PathUtil::ToUtf8(DefaultLocation()).c_str());
        ImGui::OpenPopup("New Project");
    }
    ImGui::SameLine();
    if (ImGui::Button("Import")) ImportProject();
    ImGui::SameLine();

    ImGui::BeginDisabled(!SelectionUsable());
    if (ImGui::Button("Open")) OpenEntry(static_cast<size_t>(m_selected));
    ImGui::EndDisabled();
    ImGui::SameLine();

    ImGui::BeginDisabled(!HasSelection());
    if (ImGui::Button("Remove")) {                              // forget only, files stay on disk
        m_list.Remove(static_cast<size_t>(m_selected));
        m_list.Save();
        m_selected = -1;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();

    ImGui::BeginDisabled(!SelectionUsable());
    if (ImGui::Button("Delete")) ImGui::OpenPopup("Delete Project");
    ImGui::EndDisabled();
}

void Launcher::DrawProjectTable() {
    const auto& entries = m_list.GetEntries();
    if (entries.empty()) {
        ImGui::TextDisabled("No projects yet. Create one, or import an existing project folder.");
        return;
    }

    int openIndex = -1;   // opening reorders the list, so do it after we stop iterating
    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                                 ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("projects", 3, tableFlags, ImVec2(0.0f, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Last opened", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
            const ProjectEntry& entry = entries[static_cast<size_t>(i)];
            ImGui::TableNextRow();
            ImGui::PushID(i);
            if (entry.missing) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));

            ImGui::TableSetColumnIndex(0);
            std::string label = entry.name + (entry.missing ? "  (missing)" : "");
            if (ImGui::Selectable(label.c_str(), m_selected == i,
                                  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                m_selected = i;
                if (ImGui::IsMouseDoubleClicked(0) && !entry.missing) openIndex = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(PathUtil::ToUtf8(entry.path).c_str());

            ImGui::TableSetColumnIndex(2);
            char timeBuf[32] = "-";
            if (entry.lastOpened != 0) {
                if (const std::tm* t = std::localtime(&entry.lastOpened))
                    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", t);
            }
            ImGui::TextUnformatted(timeBuf);

            if (entry.missing) ImGui::PopStyleColor();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (openIndex >= 0) OpenEntry(static_cast<size_t>(openIndex));
}

void Launcher::DrawNewProjectPopup() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    ImGui::InputText("Name", m_nameBuf, sizeof(m_nameBuf));
    ImGui::InputText("Location", m_locationBuf, sizeof(m_locationBuf));
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        if (auto folder = PickFolder(PathUtil::FromUtf8(m_locationBuf)))
            std::snprintf(m_locationBuf, sizeof(m_locationBuf), "%s", PathUtil::ToUtf8(*folder).c_str());
    }
    ImGui::TextDisabled("Will create: %s", PathUtil::ToUtf8(PathUtil::FromUtf8(m_locationBuf) / m_nameBuf).c_str());
    if (!m_createError.empty()) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_createError.c_str());

    ImGui::BeginDisabled(!Project::IsValidName(m_nameBuf));
    if (ImGui::Button("Create & Open")) {
        std::string error;
        auto project = Project::Create(PathUtil::FromUtf8(m_locationBuf), m_nameBuf, error);
        if (!project) {
            m_createError = error;
        } else {
            m_list.Add(*project);
            m_list.Save();
            m_result = std::move(project);
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

void Launcher::DrawDeletePopup() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("Delete Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    if (!HasSelection()) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
    const fs::path path = m_list.GetEntries()[static_cast<size_t>(m_selected)].path;   // copy: list changes below

    ImGui::TextUnformatted("Permanently delete this project folder from disk?");
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", PathUtil::ToUtf8(path).c_str());
    ImGui::TextDisabled("This cannot be undone. Use Remove to only forget it.");

    if (ImGui::Button("Delete")) {
        std::string error;
        if (Project::Delete(path, error)) {
            m_list.Remove(static_cast<size_t>(m_selected));
            m_list.Save();
            m_selected = -1;
        } else {
            ShowError(error);
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

void Launcher::DrawErrorPopup() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::TextWrapped("%s", m_error.c_str());
    if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

void Launcher::OpenEntry(size_t index) {
    std::string error;
    auto project = Project::Open(m_list.GetEntries()[index].path, error);
    if (!project) { ShowError(error); return; }
    m_list.Add(*project);    // bumps to the front + updates lastOpened
    m_list.Save();
    m_result = std::move(project);
}

void Launcher::ImportProject() {
    auto folder = PickFolder(DefaultLocation());
    if (!folder) return;
    std::string error;
    auto project = Project::Open(*folder, error);
    if (!project) { ShowError(error); return; }
    m_list.Add(*project);
    m_list.Save();
    m_selected = 0;          // Add() puts it first; like Godot, import adds it without opening
}

void Launcher::ShowError(const std::string& message) {
    m_error = message;
    m_showError = true;
}

bool Launcher::HasSelection() const {
    return m_selected >= 0 && m_selected < static_cast<int>(m_list.GetEntries().size());
}

bool Launcher::SelectionUsable() const {
    return HasSelection() && !m_list.GetEntries()[static_cast<size_t>(m_selected)].missing;
}

std::optional<fs::path> Launcher::PickFolder(const fs::path& startIn) {
    std::error_code ec;
    const fs::path previousCwd = fs::current_path(ec);          // engine loads "asset/..." relative to the cwd

    std::string start = PathUtil::ToUtf8(startIn);
    const nfdu8char_t* startPtr = (!startIn.empty() && fs::is_directory(startIn, ec)) ? start.c_str() : nullptr;
    nfdu8char_t* picked = nullptr;
    nfdresult_t result = NFD_PickFolderU8(&picked, startPtr);

    if (!previousCwd.empty()) fs::current_path(previousCwd, ec);   // a dialog may have moved it
    if (result != NFD_OKAY) return std::nullopt;
    fs::path folder = PathUtil::FromUtf8(picked);
    NFD_FreePathU8(picked);
    return folder;
}

fs::path Launcher::DefaultLocation() const {
    std::error_code ec;
    const auto& entries = m_list.GetEntries();
    if (!entries.empty()) {
        fs::path parent = entries.front().path.parent_path();
        if (fs::is_directory(parent, ec)) return parent;
    }
    for (const char* name : {"USERPROFILE", "HOME"})
        if (const char* value = std::getenv(name)) return fs::path(value);
    return fs::current_path(ec);
}