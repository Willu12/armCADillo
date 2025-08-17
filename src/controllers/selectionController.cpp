#include "selectionController.hpp"
#include "IEntity.hpp"
#include "color.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include <algorithm>
#include <print>
#include <vector>

std::optional<std::shared_ptr<IEntity>>
SelectionController::getEntity(float x, float y) {
  PickingTexture::PixelInfo pixel =
      _pickingTexture.ReadPixel(x, GLFWHelper::getHeight(_window) - y - 1);
  if (pixel.ObjectId == 0)
    return std::nullopt;
  auto clickedObjectId = pixel.ObjectId - 1;

  auto sceneEntities = _scene->getPoints();
  if (clickedObjectId > sceneEntities.size())
    return std::nullopt;
  return sceneEntities[clickedObjectId];
}

void SelectionController::process(const Mouse &mouse) {
  if (ImGui::IsAnyItemActive())
    return; // This maybe unnecessary

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    //   if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    // _selectedEntities.clear();

    // check if something was clicked;
    const auto &currentMousePos = ImGui::GetMousePos();
    if (auto entity = getEntity(currentMousePos[0], currentMousePos[1])) {
      mouse._isSelectionBoxActive = false;
      const auto iter = std::ranges::find(_selectedEntities, *entity);
      if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        if (iter == _selectedEntities.end())
          _selectedEntities.emplace_back(*entity);
        else
          _selectedEntities.erase(iter);
      }
    } else {
      mouse._isSelectionBoxActive = true;
      if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        for (const auto &selectedEntity : _selectedEntities) {
          selectedEntity->setColor(Color::White());
        }
        _selectedEntities.clear();
      }
    }

  }

  else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
           mouse._isSelectionBoxActive) {
    mouse._isSelectionBoxActive = false;
    const auto &entities =
        getEntities(mouse.getLastClickedPosition(), mouse.getCurrentPosition());
    if (!ImGui::GetIO().KeyCtrl) {
      if (_selectedEntities.empty())
        _selectedEntities = entities;
    } else {
      for (const auto &entity : entities) {
        if (std::ranges::find(_selectedEntities, entity) ==
            _selectedEntities.end())
          _selectedEntities.emplace_back(entity);
      }
    }
  }
}

std::vector<std::shared_ptr<IEntity>>
SelectionController::getEntities(const algebra::Vec2f &startPos,
                                 const algebra::Vec2f &endPos) {
  const size_t stepCount = 30;
  const float dx = (endPos[0] - startPos[0]) / stepCount;
  const float dy = (endPos[1] - startPos[1]) / stepCount;

  std::vector<std::shared_ptr<IEntity>> selectedEntities;
  float x = startPos[0];
  for (int stepX = 0; stepX < stepCount; ++stepX) {
    float y = startPos[1];
    x += dx;

    for (int stepY = 0; stepY < stepCount; ++stepY) {
      y += dy;
      if (auto entity = getEntity(x, y)) {
        if (std::ranges::find(selectedEntities, *entity) !=
            selectedEntities.end())
          continue;

        selectedEntities.push_back(*entity);
      }
    }
  }
  return selectedEntities;
}