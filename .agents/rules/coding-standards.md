---
trigger: glob
globs: *.cpp, *.hpp, *.c, *.h
---

# Coding Standards

* Activation: Glob (*.cpp, *.hpp, *.c, *.h)
* Trigger: Apply whenever reading, modifying, or generating C/C++ source code.
* This document defines the absolute rules for code formatting, naming, and file structure that the Engineer must strictly follow to ensure consistency across the codebase.

## 1. Naming Conventions

* **Classes, Structs, Enums, Type Aliases:** Use `PascalCase` (e.g., `TagRegistry`, `ModbusConfig`).
* **Functions and Methods:** Use `PascalCase` (e.g., `ReadHoldingRegisters()`).
* **Local Variables and Parameters:** Use `camelCase` (e.g., `pollIntervalMs`).
* **Private/Protected Members:** Use `camelCase` with a trailing underscore (e.g., `pollTaskHandle_`).
* **Compile-time Constants (`constexpr`):** Prefix with `k` followed by `PascalCase` (e.g., `kMaxRetryCount`).
* **Macros:** Use `UPPER_SNAKE_CASE` (e.g., `CONFIG_GATEWAY_ENABLE_CAN`).
* **Namespaces and Files:** Use `snake_case` (e.g., `gateway::modbus`, `modbus_master.cpp`).
* **Booleans:** Must include an `is`, `has`, or `needs` prefix (e.g., `isConnected`, `hasPendingWrite`).
* **Time Units:** Any time or interval-valued variable or constant must have an explicit unit suffix such as `_Ms`, `_Sec`, or `_Min` (or `_MS` for macros).

## 2. Formatting and Control Flow

* **Brace Style:** The opening brace must be placed on its own new line.
* **Mandatory Braces:** Braces must always be used on `if`, `else`, `for`, and `while` statements, even for single-line bodies.
* **Conditional Logic:** The `if` branch must hold the expected "happy path," while the `else` branch handles the error or absence case.
* **Line Length and Statements:** Limit lines to 100 columns and write strictly one statement per line.

## 3. File Headers and Layout

* **Headers:** Every source file (`.hpp`, `.cpp`, `.h`, `.c`)

  ```cpp
  /******************************************************************************
  * @file  : name.hpp
  * @brief : Short consise brief about this file/module
  *
  * Author : Huwairis Ibnu Kabeer
  * License: MIT
  ******************************************************************************/
  ```

* **Structure:** Files must follow a strict internal order:
  * Include:

    ```cpp
    /*==========================================================================*/
    /* Includes                                                                 */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Macros                                                                   */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Enumerations                                                             */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Structures                                                               */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Global Variables                                                         */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Configuration / Lookup Tables                                            */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Classes                                                                  */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Public API                                                               */
    /*==========================================================================*/

    ```

  * src:

    ```cpp

    /*==========================================================================*/
    /* Includes                                                                 */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Macros                                                                   */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Static Variables                                                         */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Static Configuration / Lookup Tables                                     */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Static Helper Functions                                                  */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Public Member Functions                                                  */
    /*==========================================================================*/

    /*==========================================================================*/
    /* Private Member Functions                                                 */
    /*==========================================================================*/

    ```

## 4. Linkage and Scope

* Functions that do not need to be part of a component's public interface must be marked `static` or placed inside an anonymous namespace to default to internal linkage.
* Member variables not intended for external use must be declared `private`.

## 5. Constants vs. Macros

* **Magic Numbers:** Raw literals used for sizes, timeouts, or counts are strictly forbidden; use named `constexpr` variables instead.
* **Macro Minimization:** Prefer `constexpr` or `enum class` over `#define` for constants. Reserve macros exclusively for conditional compilation, header guards, or token-pasting.

## 6. Commenting and Doxygen

* **Public APIs:** Every public API in an `include/` header must have Doxygen comments. This must include a brief description, `@param` for each parameter, `@return` detailing the meaning of the return value, and any thread-safety or calling-context requirements.
* **Internal Helpers:** Private or internal helpers require a one-line purpose comment.
* **Comment Philosophy:** Comments should explain "why" a decision was made rather than "how" the code works, and should be placed beside hardware quirks to explain workarounds.
* **Dead Code:** Unused code must be deleted rather than commented out.
