#ifndef OK_GL_CONFIG_HPP
#define OK_GL_CONFIG_HPP

#if (__APPLE__)
// macOS: use Apple's OpenGL framework headers directly.
// Silence deprecation warnings on macOS
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#define GLFW_INCLUDE_GLCOREARB
// Next two defined to avoid using gl.h and gl3.h at the same time
// (which causes warnings)
#define __gl_h_
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl3.h>      // IWYU pragma: export
#include <OpenGL/gltypes.h>  // IWYU pragma: export
#else
// Other platforms (e.g. Linux): use GLEW as the OpenGL function loader. GLEW
// must be included before GLFW (or any other OpenGL header), and the loader
// must be initialized once a GL context is current (see okInitGlLoader).
#include <GL/glew.h>  // IWYU pragma: export
#endif

// the pragmas below are used to control include-what-you-use (iwyu) behavior
// and ensure that the necessary OpenGL headers are exported correctly
// and to avoid warnings about missing includes when using strict configuration

#include <GLFW/glfw3.h>  // IWYU pragma: export

// Initialize the OpenGL function loader for the current platform. Call once,
// after a GL context is made current and before any GL call. On Apple the
// system framework provides the entry points directly, so this is a no-op; on
// other platforms it runs glewInit(). Returns true on success. Keeping the
// platform-specific logic here means every GL context (the engine's and the
// tests') initializes the loader the same way.
bool okInitGlLoader();

#endif  // OK_GL_CONFIG_HPP
