#include "selectionController.hpp"
#include "IEntity.hpp"
#include "imgui.h"
#include "mouse.hpp"
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
    return;
  ImVec2 currentMousePosition = ImGui::GetMousePos();

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    //
    //
    _selectedEntities.clear();
    //  _selectionBoxActive = true;

    auto entity = getEntity(currentMousePosition.x, currentMousePosition.y);
    if (entity) {
      const std::shared_ptr<IEntity> &entityPointer = entity.value();
      if (ImGui::GetIO().KeyCtrl) {
        auto it = std::ranges::find(_selectedEntities, entityPointer);
        if (it != _selectedEntities.end())
          _selectedEntities.erase(it);
        else
          _selectedEntities.push_back(entityPointer);
      } else {
        _selectedEntities.clear();
        _selectedEntities.push_back(entityPointer);
      }
    } else if (!entity && !ImGui::GetIO().KeyCtrl &&
               !ImGui::GetIO().WantCaptureMouse) {
      _selectedEntities.clear();
    }

  } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    _selectionBoxActive = false;
    const auto &entities =
        getEntities(mouse.getLastClickedPosition(), mouse.getCurrentPosition());
    if (!ImGui::GetIO().KeyCtrl) {
      _selectedEntities = entities;
    }
  }
};

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
  printf("fooound\n");
  return selectedEntities;
}