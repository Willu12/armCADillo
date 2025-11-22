#include "pathCombinerGui.hpp"
#include "cursor.hpp"
#include "imgui.h"
#include "namedPath.hpp"
#include "nfd.hpp"
#include "vec.hpp"
#include <memory>

void PathCombinerGUI::displayGUI(Cursor *cursor) {
  ImGui::Begin("Path Combiner");
  showPathList();
  addPoint(cursor->getPosition());
  loadPathsGUI();
  createCombinedPaths();
  ImGui::End();
};

void PathCombinerGUI::showPathList() {
  if (!ImGui::BeginListBox("Milling Paths")) {
    return;
  }
  const auto &paths = pathCombiner_.millingPaths();

  for (uint32_t i = 0; i < paths.size(); ++i) {
    bool is_selected =
        std::ranges::find(selectedPaths_, i) != selectedPaths_.end();
    const auto &path = paths[i];

    if (ImGui::Selectable(path->name().c_str(), is_selected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (is_selected) {
          auto selected_path_it = std::ranges::find(selectedPaths_, i);
          selectedPaths_.erase(selected_path_it);
        } else {
          selectedPaths_.push_back(i);
        }

      } else {
        selectedPaths_.clear();
        selectedPaths_.push_back(i);
      }
    }
  }

  ImGui::EndListBox();
}

void PathCombinerGUI::createCombinedPaths() {
  if (ImGui::Button("Create combined path")) {
    pathCombiner_.createCombinedPath(selectedPaths_);
  }
}

void PathCombinerGUI::addPoint(const algebra::Vec3f &cursorPosition) {
  if (ImGui::Button("Add point")) {
    pathCombiner_.addPoint(cursorPosition);
  }
}

void PathCombinerGUI::loadPathsGUI() {
  if (!ImGui::Button("Load Paths")) {
    return;
  }

  NFD::Init();
  NFD::UniquePathSet paths;
  nfdresult_t result = NFD::OpenDialogMultiple(paths);
  nfdpathsetsize_t count = 0;
  NFD::PathSet::Count(paths, count);

  if (result == NFD_OKAY) {
    std::vector<std::filesystem::path> filepaths;
    NFD::UniquePathSetPathN path;
    for (int i = 0; i < count; ++i) {
      NFD::PathSet::GetPath(paths, i, path);
      filepaths.emplace_back(path.get());
    }

    pathCombiner_.addPaths(filepaths);
  }
  NFD::Quit();
}

std::vector<const NamedPath *> PathCombinerGUI::getSelectedPaths() const {
  std::vector<const NamedPath *> paths;
  paths.reserve(selectedPaths_.size());
  for (const auto &index : selectedPaths_) {
    paths.push_back(pathCombiner_.millingPaths()[index].get());
  }
  return paths;
}