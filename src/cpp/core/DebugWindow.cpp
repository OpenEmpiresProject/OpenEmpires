#include "DebugWindow.h"

#include "ImGuiHelper.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"

using namespace core;

DebugWindow::DebugWindow()
{
    registerCallback(Event::Type::TICK, this, &DebugWindow::onTick);
    registerCallback(Event::Type::MOUSE_MOVE, this, &DebugWindow::onMouseMove);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &DebugWindow::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_BTN_DOWN, this, &DebugWindow::onMouseButtonDown);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &DebugWindow::onUnitSelection);
}

DebugWindow::~DebugWindow()
{
    // destructor
}

bool DebugWindow::onMouseMove(const Event& e)
{
    if (m_stateManager->isRendererReady())
    {
        auto& data = e.getData<MouseMoveData>();
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent((float) data.screenPos.x, (float) data.screenPos.y);

        // If the imgui is in focus, do not let EventLoop to propagate this event to others
        return ImGui::GetIO().WantCaptureMouse;
    }
    return false;
}

bool DebugWindow::onMouseButtonUp(const Event& e)
{
    if (m_stateManager->isRendererReady())
    {
        auto& data = e.getData<MouseClickData>();
        ImGuiIO& io = ImGui::GetIO();
        // Note: MouseClickData::Button enum values are assumed to align with imgui values
        io.AddMouseButtonEvent((int) data.button, false);

        // If the imgui is in focus, do not let EventLoop to propagate this event to others
        return ImGui::GetIO().WantCaptureMouse;
    }
    return false;
}

bool DebugWindow::onMouseButtonDown(const Event& e)
{
    if (m_stateManager->isRendererReady())
    {
        auto& data = e.getData<MouseClickData>();
        ImGuiIO& io = ImGui::GetIO();
        // Note: MouseClickData::Button enum values are assumed to align with imgui values
        io.AddMouseButtonEvent((int) data.button, true);

        // If the imgui is in focus, do not let EventLoop to propagate this event to others
        return ImGui::GetIO().WantCaptureMouse;
    }
    return false;
}

bool DebugWindow::onTick(const Event& e)
{
    if (m_stateManager->isRendererReady())
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        showDebugWindow();
        ImGui::Render();
    }
    return false;
}

template <typename... Args> inline void tableKVFmt(const char* key, const char* fmt, Args&&... args)
{
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(key);

    ImGui::TableNextColumn();
    ImGui::Text(fmt, std::forward<Args>(args)...);
}

void DebugWindow::showDebugWindow()
{
    const int windowWidth = 400;

    ImGui::SetNextWindowSize(ImVec2(windowWidth, 500));
    ImGui::Begin("Debug Window");

    ImGui::Text("Selected Entities %d", m_currentEntitySelection.selection.selectedEntities.size());
    ImGui::Separator();

    ImGui::BeginChild("Entity Selection", ImVec2(windowWidth, 200));
    auto singleEntity = m_currentEntitySelection.selection.selectedEntities.size() == 1;
    int selectedEntityTemp = -1;

    for (auto e : m_currentEntitySelection.selection.selectedEntities)
    {
        bool isSelected = (m_selectedEntity == e) or singleEntity;

        std::string label = fmt::format("{}", e);

        if (ImGui::Selectable(label.c_str(), isSelected) or isSelected)
        {
            m_selectedEntity = e;
            selectedEntityTemp = e;
        }
    }

    ImGui::EndChild();

    ImGui::Text("Properties");

    ImGui::Separator();
    if (selectedEntityTemp != -1 and ImGui::BeginTable("Properties", 2))
    {
        auto& transform = m_stateManager->getComponent<CompTransform>(selectedEntityTemp);

        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        tableKVFmt("Position", "%s", transform.position.toString().c_str());
        tableKVFmt("Tile", "%s", transform.position.toTile().toString().c_str());
        tableKVFmt("Max Speed", "%d", transform.speed.value());

        if (auto unit = m_stateManager->tryGetComponent<CompUnit>(selectedEntityTemp))
        {
            tableKVFmt("Command", "%s", unit->commandQueue.top()->toString().c_str());
            tableKVFmt("In Formation", "%s", unit->formationSlot.isValid() ? "true" : "false");
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

bool DebugWindow::onUnitSelection(const Event& e)
{
    m_currentEntitySelection = e.getData<EntitySelectionData>();
    return false;
}
