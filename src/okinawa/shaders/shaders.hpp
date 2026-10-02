#ifndef OK_SHADERS_HPP
#define OK_SHADERS_HPP

#include "../core/gl_config.hpp"
#include <string>

class OkShader {
public:
  // Static class - no instantiation
  OkShader() = delete;

  // Compile shader from source
  static GLuint compile(const std::string &source, GLenum shaderType,
                        const std::string &shaderName);

  // Create shader program from vertex and fragment shaders
  static GLuint createProgram(const std::string &vertexSource,
                              const std::string &fragmentSource);

  /**
   * @brief A number that changes every time a program is linked.
   *
   * For whoever keeps uniform locations by program: the driver hands the
   * same name out again once a program is deleted or its context is
   * gone, so a cache keyed by the name alone can hold another program's
   * locations. Compare this as well, and look them up again when it has
   * moved.
   */
  static unsigned long generation();

private:
};

#endif
