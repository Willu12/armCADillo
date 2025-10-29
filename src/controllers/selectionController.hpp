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
  SelectionController(GLFWwindow *window, const Scene *scene,
                      std::vector<IEntity *> &selectedEntities)
      : _window(window), _scene(scene), _selectedEntities(selectedEntities) {

    _pickingTexture.init(GLFWHelper::getWidth(_window),
                         GLFWHelper::getHeight(_window));
  }

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void process(const Mouse & /*mouse*/) override;

  PickingTexture &getPickingTexture() { return _pickingTexture; }

private:
  GLFWwindow *_window = nullptr;
  const Scene *_scene = nullptr;
  std::vector<IEntity *> &_selectedEntities;
  PickingTexture _pickingTexture;
  bool _selectionBoxActive = false;

  std::optional<IEntity *> getEntity(float x, float y);
  std::vector<IEntity *> getEntities(const algebra::Vec2f &startPos,
                                     const algebra::Vec2f &endPos,
                                     int stride = 1);
};