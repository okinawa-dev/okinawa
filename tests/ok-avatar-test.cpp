#include "okinawa/avatar/cameras/top_down_view.hpp"
#include "okinawa/avatar/controllers/walk_controller.hpp"
#include "okinawa/input/input.hpp"
#include "okinawa/math/rotation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

// These exercise the pure ground-movement maths (no window/GL needed). They
// assert convention-independent invariants, so they don't depend on the exact
// forward direction a given yaw produces.

namespace {
float mag2d(const OkGroundMove &m) {
  return std::sqrt(m.dx * m.dx + m.dz * m.dz);
}
bool near(float a, float b) { return std::fabs(a - b) < 1e-3f; }
}  // namespace

TEST_CASE("walk: no input produces no movement", "[avatar]") {
  OkInputState in;
  OkRotation   cam(0.0f, 0.0f, 0.0f);
  OkGroundMove m = OkWalkController::computeGroundMove(in, cam, 10.0f, 1.0f);
  CHECK_FALSE(m.moved);
  CHECK(m.dx == 0.0f);
  CHECK(m.dz == 0.0f);
}

TEST_CASE("walk: forward moves at speed * dt", "[avatar]") {
  OkInputState in;
  in.forward = true;
  OkRotation   cam(0.0f, 0.0f, 0.0f);
  OkGroundMove m = OkWalkController::computeGroundMove(in, cam, 10.0f, 0.5f);
  CHECK(m.moved);
  CHECK(near(mag2d(m), 5.0f));
}

TEST_CASE("walk: opposing inputs cancel out", "[avatar]") {
  OkInputState in;
  in.forward  = true;
  in.backward = true;
  OkRotation   cam(0.0f, 0.0f, 0.0f);
  OkGroundMove m = OkWalkController::computeGroundMove(in, cam, 10.0f, 1.0f);
  CHECK_FALSE(m.moved);
}

TEST_CASE("walk: strafing left and right are opposite", "[avatar]") {
  OkRotation   cam(0.0f, 0.0f, 0.0f);
  OkInputState r;
  r.strafeRight = true;
  OkInputState l;
  l.strafeLeft = true;
  OkGroundMove mr = OkWalkController::computeGroundMove(r, cam, 10.0f, 1.0f);
  OkGroundMove ml = OkWalkController::computeGroundMove(l, cam, 10.0f, 1.0f);
  CHECK(mr.moved);
  CHECK(ml.moved);
  CHECK(near(mr.dx, -ml.dx));
  CHECK(near(mr.dz, -ml.dz));
}

TEST_CASE("walk: diagonal movement is normalised (not faster)", "[avatar]") {
  OkInputState in;
  in.forward     = true;
  in.strafeRight = true;
  OkRotation   cam(0.0f, 0.0f, 0.0f);
  OkGroundMove m = OkWalkController::computeGroundMove(in, cam, 10.0f, 1.0f);
  CHECK(m.moved);
  CHECK(near(mag2d(m), 10.0f));
}

TEST_CASE("top-down: height frames the rectangle for the FOV", "[avatar]") {
  // 1200 m square, 60 deg vertical FOV, no margin: half / tan(30 deg).
  float h = OkTopDownView::computeHeight(1200.0f, 1200.0f, 60.0f, 1.0f);
  CHECK(std::fabs(h - 600.0f / std::tan(M_PI / 6.0f)) < 1.0f);

  // The larger side drives it (1200, not 400).
  float hWide = OkTopDownView::computeHeight(1200.0f, 400.0f, 60.0f, 1.0f);
  CHECK(std::fabs(hWide - h) < 1e-3f);

  // Margin raises the camera; a wider FOV lowers it.
  CHECK(OkTopDownView::computeHeight(1200.0f, 1200.0f, 60.0f, 1.2f) > h);
  CHECK(OkTopDownView::computeHeight(1200.0f, 1200.0f, 90.0f, 1.0f) < h);
}
