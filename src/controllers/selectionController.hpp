#pragma once
#include "IController.hpp"
#include "IEntity.hpp"
#include "imgui.h"
#include "pickingTexture.hpp"
#include "scene.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "glfwHelper.hpp"

class SelectionController : public IController {
public:
  SelectionController(GLFWwindow *window, std::shared_ptr<Scene> scene,
                      std::vector<std::shared_ptr<IEntity>> &selectedEntities)
      : _window(window), _scene(std::move(scene)),
        _selectedEntities(selectedEntities) {

    _pickingTexture.init(GLFWHelper::getWidth(_window),
                         GLFWHelper::getHeight(_window));
  }

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void process(const Mouse & /*mouse*/) override;

  PickingTexture &getPickingTexture() { return _pickingTexture; }

private:
  GLFWwindow *_window;
  std::shared_ptr<Scene> _scene;
  std::vector<std::shared_ptr<IEntity>> &_selectedEntities;
  PickingTexture _pickingTexture;
  bool _selectionBoxActive = false;

  std::optional<std::shared_ptr<IEntity>> getEntity(float x, float y);
  std::vector<std::shared_ptr<IEntity>>
  getEntities(const algebra::Vec2f &startPos, const algebra::Vec2f &endPos,
              int stride = 1);
};