#include "interpolatingSplineC2.hpp"
#include "vec.hpp"

InterpolatingSplineC2::InterpolatingSplineC2(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  _id = kClassId++;
  _name = "InterpolatingSplineC2_" + std::to_string(_id);
  for (const auto &p : points) {
    _points.emplace_back(p);
    subscribe(p);
  }
  _mesh = generateMesh();
}

std::vector<float> InterpolatingSplineC2::calculateChordLengthKnotsDists() {
  std::vector<float> dists(_points.size() - 1);
  for (std::size_t i = 1; i < _points.size(); ++i) {
    auto deltaP =
        _points[i].get().getPosition() - _points[i - 1].get().getPosition();
    dists[i - 1] = deltaP.length() == 0.f ? 10e-6f : deltaP.length();
  }
  return dists;
}

std::vector<algebra::Vec3f> InterpolatingSplineC2::calculateBezierPoints() {
  auto dists = calculateChordLengthKnotsDists();
  std::vector<float> alpha;
  std::vector<float> beta;
  std::vector<algebra::Vec3f> r;

  for (int i = 1; i < dists.size(); ++i) {
    float d0 = dists[i - 1];
    float d1 = dists[i];

    if (i != 1)
      alpha.push_back(d0 / (d0 + d1));
    if (i != dists.size() - 1)
      beta.push_back(d1 / (d0 + d1));

    auto p0 =
        (_points[i].get().getPosition() - _points[i - 1].get().getPosition()) /
        d0;
    auto p1 =
        (_points[i + 1].get().getPosition() - _points[i].get().getPosition()) /
        d1;
    r.push_back(((p1 - p0) * 3.f) / (d0 + d1));
  }
  auto c = solveTridiagonalMatrix(alpha, beta, r);
  auto bezierPoints = convertPowertoBezier(c, dists);
  return bezierPoints;
}

std::vector<algebra::Vec3f>
InterpolatingSplineC2::convertPowertoBezier(std::vector<algebra::Vec3f> &c,
                                            const std::vector<float> &dists) {

  c.insert(c.begin(), algebra::Vec3f());
  c.emplace_back();

  std::vector<algebra::Vec3f> a(c.size());
  std::vector<algebra::Vec3f> b(c.size());
  std::vector<algebra::Vec3f> d(c.size());

  for (int i = 0; i < d.size(); ++i)
    d[i] = (c[i + 1] - c[i]) / dists[i] / 3.f;

  for (int i = 0; i < a.size(); ++i)
    a[i] = _points[i].get().getPosition();

  for (int i = 0; i < b.size() - 1; i++) {
    b[i] = (a[i + 1] - a[i]) / dists[i] - c[i] * dists[i] -
           d[i] * dists[i] * dists[i];
  }

  std::vector<algebra::Vec3f> bezierPoints;

  bezierPoints.reserve(4 * dists.size());
  for (int i = 0; i < dists.size(); ++i) {
    auto ai = a[i];
    auto bi = b[i] * dists[i];
    auto ci = c[i] * dists[i] * dists[i];
    auto di = d[i] * dists[i] * dists[i] * dists[i];

    bezierPoints.emplace_back(ai);
    bezierPoints.emplace_back(ai + bi / 3.f);
    bezierPoints.emplace_back(ai + bi * 2.f / 3.f + ci / 3.f);
    bezierPoints.emplace_back(ai + bi + ci + di);
  }
  return bezierPoints;
}

std::vector<algebra::Vec3f>
InterpolatingSplineC2::solveTridiagonalMatrix(std::vector<float> &alpha,
                                              std::vector<float> &beta,
                                              std::vector<algebra::Vec3f> &r) {
  const std::size_t m = r.size();
  std::vector<float> gamma(m);
  std::vector<algebra::Vec3f> delta(m);
  std::vector<algebra::Vec3f> c(m);

  if (r.size() == 1) {
    c[0] = r[0] / 2.f;
    return c;
  }

  float denom = 2.0f;
  gamma[0] = beta[0] / denom;
  delta[0] = r[0] / denom;

  for (std::size_t i = 1; i < m; ++i) {
    denom = 2.0f - alpha[i - 1] * gamma[i - 1];
    if (i < m - 1) {
      gamma[i] = beta[i] / denom;
    }
    delta[i] = (r[i] - delta[i - 1] * alpha[i - 1]) / denom;
  }

  c[m - 1] = delta[m - 1];
  for (int i = static_cast<int>(m) - 2; i >= 0; --i) {
    c[i] = delta[i] - c[i + 1] * gamma[i];
  }

  return c;
}

std::unique_ptr<BezierMesh> InterpolatingSplineC2::generateMesh() {
  auto bezierPositions = calculateBezierPoints();
  return BezierMesh::createC2(bezierPositions);
};

bool InterpolatingSplineC2::acceptVisitor(IVisitor &visitor) {
  return visitor.visitInterpolatingSplineCurve(*this);
}