#include "jsonSerializer.hpp"
#include "IEntity.hpp"
#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceC2.hpp"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
#include "torusEntity.hpp"
#include "virtualPoint.hpp"
#include <fstream>

using json = nlohmann::json;

void JsonSerializer::serializeScene(const Scene &scene) {
  createSceneJson(scene);
  std::ofstream file(_savePath);
  if (file.is_open()) {
    file << std::setw(4) << _sceneJson << std::endl;
    file.close();
  } else {
    throw std::runtime_error("Unable to open file: " + _savePath);
  }
}

void JsonSerializer::createSceneJson(const Scene &scene) {
  json sceneJson;
  const auto &points = scene.getPoints();
  _pointsJson.clear();
  for (const auto &point : points) {
    point->acceptVisitor(*this);
  }
  _geometryJson.clear();
  const auto &groupedEntites = scene.getGroupedEntities();
  for (const auto &pair : groupedEntites) {
    for (const auto &entity : pair.second) {
      entity->acceptVisitor(*this);
    }
  }

  sceneJson["points"] = _pointsJson;
  sceneJson["geometry"] = _geometryJson;
  _sceneJson = sceneJson;
}

bool JsonSerializer::visitTorus(TorusEntity &torus) {
  json j;
  j["objectType"] = "torus";
  j["id"] = torus.getId();
  j["name"] = torus.getName();
  serializePosition(j, torus);
  serializeRotation(j, torus);
  serializeScale(j, torus);
  const auto &meshDensity = torus.getMeshDensity();
  j["samples"] = {{"u", meshDensity.s}, {"v", meshDensity.t}};
  j["smallRadius"] = {torus.getTubeRadius()};
  j["largeRadius"] = {torus.getInnerRadius()};
  _geometryJson.push_back(j);

  return true;
};

bool JsonSerializer::visitPoint(PointEntity &point) {
  json j;
  j["id"] = point.getId();
  j["name"] = point.getName();
  serializePosition(j, point);
  _pointsJson.push_back(j);
  return true;
}

bool JsonSerializer::visitBezierCurve(BezierCurveC0 &bezierCurve) {
  json j;
  j["objectType"] = "bezierC0";
  j["id"] = bezierCurve.getId();
  j["name"] = bezierCurve.getName();
  serializeControlPoints(j, bezierCurve);
  _geometryJson.push_back(j);
  return true;
}

bool JsonSerializer::visitBSplineCurve(BSplineCurve &bSplineCurve) {
  json j;
  j["objectType"] = "bezierC2";
  j["id"] = bSplineCurve.getId();
  j["name"] = bSplineCurve.getName();
  serializeControlPoints(j, bSplineCurve);
  _geometryJson.push_back(j);
  return true;
}

bool JsonSerializer::visitInterpolatingSplineCurve(
    InterpolatingSplineC2 &interpolatingSpline) {
  json j;
  j["objectType"] = "interpolatedC2";
  j["id"] = interpolatingSpline.getId();
  j["name"] = interpolatingSpline.getName();
  serializeControlPoints(j, interpolatingSpline);
  _geometryJson.push_back(j);
  return true;
}

bool JsonSerializer::visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurfaceC0) {
  json j;
  j["objectType"] = "bezierSurfaceC0";
  j["id"] = bezierSurfaceC0.getId();
  j["name"] = bezierSurfaceC0.getName();
  serializeControlPoints(j, bezierSurfaceC0);
  const auto &meshDensity = bezierSurfaceC0.getMeshDensity();
  j["samples"] = {{"u", meshDensity.s}, {"v", meshDensity.t}};
  j["size"] = {{"u", bezierSurfaceC0.getColCount()},
               {"v", bezierSurfaceC0.getRowCount()}};
  _geometryJson.push_back(j);
  return true;
}

bool JsonSerializer::visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurfaceC2) {
  json j;
  j["objectType"] = "bezierSurfaceC2";
  j["id"] = bezierSurfaceC2.getId();
  j["name"] = bezierSurfaceC2.getName();
  serializeControlPoints(j, bezierSurfaceC2);
  const auto &meshDensity = bezierSurfaceC2.getMeshDensity();
  j["samples"] = {{"u", meshDensity.s}, {"v", meshDensity.t}};
  j["size"] = {{"u", bezierSurfaceC2.getColCount()},
               {"v", bezierSurfaceC2.getRowCount()}};
  _geometryJson.push_back(j);

  return true;
}

void JsonSerializer::serializePosition(json &j, const IEntity &entity) {

  const auto &pos = entity.getPosition();
  j["position"] = {{"x", pos[0]}, {"y", pos[1]}, {"z", pos[2]}};
}
void JsonSerializer::serializeRotation(json &j, const IEntity &entity) {
  const auto &rotation = entity.getRotation();
  j["rotation"] = {{"x", rotation.x()},
                   {"y", rotation.y()},
                   {"z", rotation.z()},
                   {"w", rotation.w()}};
}
void JsonSerializer::serializeScale(json &j, const IEntity &entity) {
  const auto &scale = entity.getScale();
  j["scale"] = {{"x", scale}, {"y", scale}};
}

void JsonSerializer::serializeControlPoints(
    nlohmann::json &j, const IGroupedEntity &groupedEntity) {
  json controlPoints = json::array();
  const auto &pointsRef = groupedEntity.getPointsReferences();

  for (const auto &point : pointsRef) {
    controlPoints.push_back({{"id", point.get().getId()}});
  }
  j["controlPoints"] = controlPoints;
}

bool JsonSerializer::visitVirtualPoint(VirtualPoint & /*point*/) {
  return false;
}
bool JsonSerializer::visitBezierSurface(BezierSurface & /*bezierSurface*/) {
  return false;
}