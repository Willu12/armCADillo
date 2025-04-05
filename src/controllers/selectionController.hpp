#pragma once
#include "IController.hpp"
#include "IEntity.hpp"
#include "algorithm"
#include "pickingTexture.hpp"
#include "scene.hpp"
#include <memory>
#include <optional>
#include <vector>

#include "glfwHelper.hpp"

class SelectionController : public IController {
public:
  SelectionController(GLFWwindow *window, std::shared_ptr<Scene> scene,
                      std::vector<std::shared_ptr<IEntity>> &selectedEntities)
      : _window(window), _scene(scene), _selectedEntities(selectedEntities) {

    _pickingTexture.init(GLFWHelper::getWidth(_window),
                         GLFWHelper::getHeight(_window));
  }

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void process(float x, float y) override {
    ImVec2 currentMousePosition = ImGui::GetMousePos();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      auto entity = getEntity(currentMousePosition.x, currentMousePosition.y);
      if (entity) {
        std::shared_ptr<IEntity> entityPointer = entity.value();
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
  GLFWwindow *_window;
  std::shared_ptr<Scene> _scene;
  std::vector<std::shared_ptr<IEntity>> &_selectedEntities;
  PickingTexture _pickingTexture;

  std::optional<std::shared_ptr<IEntity>> getEntity(float x, float y) {
    PickingTexture::PixelInfo pixel =
        _pickingTexture.ReadPixel(x, GLFWHelper::getHeight(_window) - y - 1);
    if (pixel.ObjectId == 0)
      return std::nullopt;
    auto clickedObjectId = pixel.ObjectId - 1;

    auto sceneEntities = _scene->getEntites();
    if (clickedObjectId > sceneEntities.size())
      return std::nullopt;
    return sceneEntities[clickedObjectId];
  }
};