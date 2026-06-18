#ifndef TEST_OPENGL_HPP
#define TEST_OPENGL_HPP

#include "okinawa/core/gl_config.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>

/**
 * @brief Test GLFW context for OpenGL rendering.
 */
class TestGLFWContext {
public:
  TestGLFWContext() {
    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    // Create a hidden window for OpenGL context
    glfwWindowHint(GLFW_SAMPLES, 4);  // 4x antialiasing
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Forward-compatible core context, the same as the engine does (required
    // on macOS, harmless elsewhere).
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(1, 1, "Test", nullptr, nullptr);
    if (!window) {
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);

    // Initialize the OpenGL loader the same way the engine does, so the shader
    // tests get valid GL entry points (on Linux glCreateShader is null until
    // the loader runs). The platform specifics live in okInitGlLoader().
    if (!okInitGlLoader()) {
      glfwDestroyWindow(window);
      glfwTerminate();
      throw std::runtime_error("Failed to initialize the OpenGL loader");
    }
  }

  ~TestGLFWContext() {
    if (window) {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
  }

private:
  GLFWwindow *window;
};

#endif  // TEST_OPENGL_HPP
