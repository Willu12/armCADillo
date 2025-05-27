#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "vec.hpp"
#include <vector>
class InterpolatingSplineC2 : public BezierCurve {
public:
  explicit InterpolatingSplineC2(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  bool acceptVisitor(IVisitor &visitor) override;

private:
  inline static int _classId;

  std::vector<algebra::Vec3f>
  solveTridiagonalMatrix(std::vector<float> &alpha, std::vector<float> &beta,
                         std::vector<algebra::Vec3f> &r);
  std::vector<float> calculateChordLengthKnotsDists();
  std::unique_ptr<BezierMesh> generateMesh() override;
  std::vector<algebra::Vec3f> calculateBezierPoints();
  std::vector<algebra::Vec3f>
  convertPowertoBezier(std::vector<algebra::Vec3f> &c,
                       const std::vector<float> &dists);
};
