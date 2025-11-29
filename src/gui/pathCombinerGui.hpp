#pragma once

#include "namedPath.hpp"
#include "pathCombiner.hpp"
#include "vec.hpp"

class Cursor;

class PathCombinerGUI {
public:
  void displayGUI(Cursor *cursor);
  std::vector<const NamedPath *> getSelectedPaths() const;

private:
  PathCombiner pathCombiner_;
  std::vector<uint32_t> selectedPaths_;
  std::string name_;
  bool showSelectedPaths_ = false;
  void showPathList();
  void createCombinedPaths();
  void removeSelectedPaths();
  void saveSelectedPath();
  void renamePath();
  void reduceSelectedPaths();
  void showNameBox();
  // void loadPaths(std::vector<)

  void addPoint(const algebra::Vec3f &position);
  void loadPathsGUI();
};