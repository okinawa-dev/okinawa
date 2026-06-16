#ifndef OK_INPUT_HPP
#define OK_INPUT_HPP

#include "../core/gl_config.hpp"  // IWYU pragma: keep
#include "keys.hpp"

/**
 * @brief Input state structure to hold the current state of input.
 *        Contains boolean flags for movement and rotation controls.
 */
class OkInputState {
public:
  OkInputState()  = default;
  ~OkInputState() = default;

  // Movement state
  bool forward     = false;
  bool backward    = false;
  bool strafeLeft  = false;
  bool strafeRight = false;

  // Rotation state
  bool turnLeft  = false;
  bool turnRight = false;
  bool turnUp    = false;
  bool turnDown  = false;

  // Camera selection (-1 if no camera key was pressed)
  int changeCamera = -1;

  // Action buttons - will be true only on the frame when key is first pressed
  bool action1 = false;
  bool action2 = false;
  bool action3 = false;
  bool action4 = false;

  // Exit state
  bool exit = false;
};

/**
 * @brief Input class to handle user input for the application.
 *        It processes input events and provides the current state of input.
 */
class OkInput {
public:
  using MouseCallback = void (*)(GLFWwindow *, double, double);
  explicit OkInput(GLFWwindow *window, MouseCallback mouseCallback = nullptr);

  ~OkInput() = default;
  // Prevent copying
  OkInput(const OkInput &)            = delete;
  OkInput &operator=(const OkInput &) = delete;

  // Process input and update states
  void process();

  // Input state retrieval methods
  // True only on the frame when key is first pressed
  bool isKeyJustPressed(OkKey key) const;
  // True while key is being held down
  bool isKeyHeld(OkKey key) const;
  // True only on the frame when key is released
  bool isKeyJustReleased(OkKey key) const;

  // Get complete input state (for compatibility)
  OkInputState getState() const;

  // Synthetic input: mark a key as held for the next durationSeconds, as if
  // it were physically pressed. Used to drive the app programmatically (e.g.
  // from the MCP server). The injected state is OR-ed into the polled state
  // in process(), so it behaves exactly like a real key, including the
  // edge-triggered actions. Call from the engine loop thread.
  void injectKey(OkKey key, double durationSeconds);

  // Enable/disable physical (keyboard/mouse) input. When disabled, process()
  // ignores glfwGetKey polling (injected keys still apply) and the cursor is
  // released (GLFW_CURSOR_NORMAL); useful to drive an instance only via the
  // MCP server without the user's input interfering.
  void setPhysicalInputEnabled(bool enabled);
  bool isPhysicalInputEnabled() const { return _physicalEnabled; }

  // Constants
  static constexpr float MOVE_SPEED     = 5.0f;
  static constexpr float ROTATION_SPEED = 2.0f;

private:
  GLFWwindow   *_window;
  MouseCallback _mouseCallback;
  OkInputState  _currentState;               // Current frame's input state
  OkInputState  _prevState;                  // Previous frame's input state
  bool          _currentKeys[OK_KEY_COUNT];  // Current key states
  bool          _prevKeys[OK_KEY_COUNT];     // Previous key states
  // Per-key "injected until" timestamps (glfwGetTime seconds). A key counts as
  // pressed while glfwGetTime() < _injectedUntil[key].
  double        _injectedUntil[OK_KEY_COUNT];
  // When false, physical keyboard/mouse input is ignored (MCP-only control).
  bool          _physicalEnabled;
};

#endif
