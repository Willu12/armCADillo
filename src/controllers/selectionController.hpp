#pragma once
#include "IController.hpp"
#include "IEntity.hpp"
#include "algorithm"
#include "pickingTexture.hpp"
#include <optional>
#include <vector>

class SelectionController : public IController {
public:
  SelectionController(GLFWwindow *window, std::vector<IEntity *> &entities,
                      std::vector<IEntity *> &selectedEntities)
      : _window(window), _entities(entities),
        _selectedEntities(selectedEntities) {

    _pickingTexture.init(GLFWHelper::getWidth(_window),
                         GLFWHelper::getHeight(_window));
  }

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void process(float x, float y) {
    ImVec2 currentMousePosition = ImGui::GetMousePos();

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      auto entity = getEntity(currentMousePosition.x, currentMousePosition.y);
      if (entity) {
        IEntity *entityPointer = entity.value();
        if (ImGui::GetIO().KeyCtrl) {
          auto it = std::find(_selectedEntities.begin(),
                              _selectedEntities.end(), entityPointer);
          if (it != _selectedEntities.end())
            _selectedEntities.erase(it);
          else
            _selectedEntities.push_back(entityPointer);
        } else {
          _selectedEntities.clear();
          _selectedEntities.push_back(entityPointer);
        }
      }
    }
  };

  PickingTexture &getPickingTexture() { return _pickingTexture; }

private:
  std::vector<IEntity *> &_selectedEntities;
  std::vector<IEntity *> &_entities;
  GLFWwindow *_window;
  PickingTexture _pickingTexture;

  std::optional<IEntity *> getEntity(float x, float y) {
    PickingTexture::PixelInfo pixel =
        _pickingTexture.ReadPixel(x, GLFWHelper::getHeight(_window) - y - 1);
    if (pixel.ObjectId == 0)
      return std::nullopt;
    auto clickedObjectId = pixel.ObjectId - 1;

    if (clickedObjectId > _entities.size())
      return std::nullopt;
    return _entities[clickedObjectId];
  }
};