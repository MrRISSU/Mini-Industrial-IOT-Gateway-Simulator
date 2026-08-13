---
trigger: glob
globs: *.cpp, *.hpp
---

# CPP Embedded Practice

* Activation: Glob (*.cpp, *.hpp)
* Trigger: Apply when designing architecture, generating logic, or working with FreeRTOS/ESP-IDF.
* This document outlines the strict C++ practices required to write efficient, safe, and robust firmware for resource-constrained microcontrollers like the ESP32.

## 1. Object-Oriented Programming (OOP) Guidelines

* **Composition Over Inheritance:** Prefer composing small, focused objects over building deep inheritance hierarchies.
* **Interface Abstraction:** Limit inheritance to genuine interface abstractions (e.g., implementing an `IModule` base class) and use composition for implementing sensor simulators and modular components.
* **Dependency Injection:** Avoid singleton abuse; inject dependencies via constructors to keep object graphs clear and testable rather than relying on global `Instance()` calls.
* **Compile-Time Polymorphism:** When designing generic components, such as a thread-safe Tags Registry, utilize C++ templates and `std::type_traits` to enforce type safety at compile time without incurring virtual dispatch overhead.

## 2. Memory Management and Heap Safety

* **Static Allocation:** Default to static allocation or pre-sized memory pools at startup. Minimal dynamic allocation after the initialization phase is critical to prevent heap fragmentation.
* **Standard Library Limits:** Strictly avoid using `std::string` concatenation or `std::vector` in hot code paths, such as download loops, MQTT payload generation, or high-frequency polling tasks.
* **Fixed-Capacity Containers:** Replace dynamic standard library containers with `std::array` or other fixed-capacity buffers.
* **JSON Handling:** Employ memory-safe JSON allocation strategies (e.g., pre-calculated capacities in ArduinoJson) to parse configurations or payloads without fragmenting the heap.

## 3. Resource Acquisition Is Initialization (RAII)

* **Resource Wrapping:** Every resource that requires an acquire/release pair (e.g., FreeRTOS mutexes, file handles, or peripheral bus claims) must be wrapped in an RAII type.
* **Lifecycle Guarantee:** The constructor must acquire the resource and the destructor must release it, guaranteeing that resources are not leaked on early return paths or error conditions.

## 4. Strong Type Safety

* **Enumerations:** Use `enum class` rather than plain `enum` or integer status codes to prevent implicit conversions and mismatched comparisons.
* **Constants:** Use `constexpr` for compile-time constants instead of `#define` macros, giving constants a defined type and scope.
* **Casting:** Avoid C-style casts entirely. Use `static_cast`, `reinterpret_cast`, or `const_cast` explicitly to make type system overrides searchable and intentional.

## 5. Interrupt Service Routine (ISR) Safety

* **No Heap Operations:** Calling `malloc`, `new`, or utilizing heap-allocating classes (like `std::string`) inside an ISR is strictly prohibited, as it can corrupt the FreeRTOS heap allocator.
* **Shared State:** Any variable shared between task-context code and an ISR must be declared `volatile`.
* **Critical Sections:** Multi-byte or complex variables shared with an ISR must be protected by a `portMUX_TYPE` critical section on both sides of the read/write operation.
