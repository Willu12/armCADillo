#include "selectionController.hpp"
#include "IEntity.hpp"
#include "color.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include <algorithm>
#include <print>
#include <unordered_set>
#include <vector>

std::optional<std::shared_ptr<IEntity>>
SelectionController::getEntity(float x, float y) {
  PickingTexture::PixelInfo pixel =
      _pickingTexture.ReadPixel(x, GLFWHelper::getHeight(_window) - y - 1);
  if (pixel.ObjectId == 0) {
    return std::nullopt;
  }
  auto clickedObjectId = pixel.ObjectId - 1;

  auto sceneEntities = _scene->getPoints();
  if (clickedObjectId > sceneEntities.size()) {
    return std::nullopt;
  }
  return sceneEntities[clickedObjectId];
}

void SelectionController::process(const Mouse &mouse) {
  if (ImGui::IsAnyItemActive()) {
    return; // This maybe unnecessary
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    //   if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    // _selectedEntities.clear();

    // check if something was clicked;
    const auto &currentMousePos = ImGui::GetMousePos();
    if (auto entity = getEntity(currentMousePos[0], currentMousePos[1])) {
      mouse._isSelectionBoxActive = false;
      const auto iter = std::ranges::find(_selectedEntities, *entity);
      if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        if (iter == _selectedEntities.end()) {
          _selectedEntities.emplace_back(*entity);
        } else {
          _selectedEntities.erase(iter);
        }
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
    const auto &entities = getEntities(mouse.getLastClickedPosition(),
                                       mouse.getCurrentPosition(), 4);
    if (!ImGui::GetIO().KeyCtrl) {
      if (_selectedEntities.empty()) {
        _selectedEntities = entities;
      }
    } else {
      for (const auto &entity : entities) {
        if (std::ranges::find(_selectedEntities, entity) ==
            _selectedEntities.end()) {
          _selectedEntities.emplace_back(entity);
        }
      }
    }
  }
}

std::vector<std::shared_ptr<IEntity>>
SelectionController::getEntities(const algebra::Vec2f &startPos,
                                 const algebra::Vec2f &endPos,
                                 int stride /* = 1 */) {
  std::vector<std::shared_ptr<IEntity>> selectedEntities;

  int minX =
      std::min(static_cast<int>(startPos[0]), static_cast<int>(endPos[0]));
  int maxX =
      std::max(static_cast<int>(startPos[0]), static_cast<int>(endPos[0]));
  int minY =
      std::min(static_cast<int>(startPos[1]), static_cast<int>(endPos[1]));
  int maxY =
      std::max(static_cast<int>(startPos[1]), static_cast<int>(endPos[1]));

  auto sceneEntities = _scene->getPoints();
  std::unordered_set<int> uniqueIds;

  for (int y = minY; y <= maxY; y += stride) {
    for (int x = minX; x <= maxX; x += stride) {
      auto pixel =
          _pickingTexture.ReadPixel(x, GLFWHelper::getHeight(_window) - y - 1);

      if (pixel.ObjectId == 0) {
        continue;
      }

      int objId = static_cast<int>(pixel.ObjectId) - 1;
      if (objId < 0 || objId >= static_cast<int>(sceneEntities.size())) {
        continue;
      }

      if (uniqueIds.insert(objId).second) {
        selectedEntities.push_back(sceneEntities[objId]);
      }
    }
  }

  return selectedEntities;
}
