# Coding Convention for Embedded Systems

## 1. File Structure & Naming
- Use `.h` for headers, `.cpp` for implementations.
- File names: CamelCase, matching the main class/module (e.g., `SensorManager.h`).
- One class per file.

## 2. Naming Conventions
- Classes: `CamelCase` (e.g., `SensorManager`)
- Methods/functions: `camelCase` (e.g., `readValue()`)
- Variables: `camelCase` (e.g., `sensorValue`)
- Constants/macros: `ALL_CAPS_WITH_UNDERSCORES`

## 3. Formatting
- Indent with 2 or 4 spaces (be consistent).
- Braces on the same line:
  ```cpp
  if (condition) {
    // code
  }
  ```
- One statement per line.

## 4. Functions & Classes
- Keep functions short and focused.
- Use `const` where possible.
- Prefer references over pointers unless nullability is required.
- Document public functions with brief comments.

## 5. Arduino Specifics
- Avoid blocking code in `loop()`; use non-blocking patterns or timers.
- Use `yield()` or `delay(1)` in long loops to avoid watchdog resets.

## 6. Error Handling
- Return status codes or `bool` for success/failure.
- Use `Serial.print` for debugging, but remove or guard with `#ifdef DEBUG` in production. Custom debugging can also be used.

## 7. Comments & Documentation
- Use `//` for single-line, `/* ... */` for multi-line comments.
- Document complex logic and all public APIs.
- Use Doxygen-style comments for functions if possible:
  ```cpp
  /**
   * Reads the sensor value.
   * @return int Sensor value.
   */
  int readValue();
  ```

## 8. Version Control
- Commit early, commit often.
- Write clear commit messages.
- No generated files or binaries in version control.

## 9. Miscellaneous
- Avoid using `String` class for memory-constrained systems; prefer C-strings or fixed-size buffers.
- Use enums for states, not numbers.
- Prefer `constexpr` or `const` over `#define` for constants.
