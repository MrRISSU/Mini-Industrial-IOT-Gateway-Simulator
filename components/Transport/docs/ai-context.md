# AI Integration Context

This document provides the context required for an AI assistant to correctly use and integrate this library into a broader ESP-IDF application.

## 1. Library Identity

* **Library Name:** Transport
* **Purpose:** Object-oriented abstraction layer for the ESP-IDF network transport API.


* **Primary Language:** C++
* **Target Platforms:** ESP32 series microcontrollers.
* **Framework:** ESP-IDF v6.0.2.
* **Dependencies:** `tcp_transport`, `esp-tls`, `log`.

## 2. What the Library Provides

This library provides:

* Plain TCP connection management.


* TLS-secured TCP connection management.


* Built-in FreeRTOS timeout handling for blocking operations.


* A standard interface for creating custom transport streams (e.g., future WebSockets or Serial).



## 3. When to Use It

Use this library when:

* The application needs a reusable, clean C++ interface for socket communication.
* You need to seamlessly swap between unsecure and secure networking layers in a custom MQTT, HTTP, or custom protocol client.

## 4. Integration Requirements

Required:

* The ESP-IDF project must register this component in `CMakeLists.txt` with `REQUIRES tcp_transport log esp-tls`.
* The system network layer (Wi-Fi/Ethernet station) must be fully initialized and connected before calling `Connect()`.

## 5. Integration Flow

1. Create a `TcpTransport` or `TlsTransport` object.


2. For TLS, call `SetCert(cert_pem)` with a valid string.


3. Call `Initialise()` to allocate handles.


4. Call `Connect()` with the host, port, and timeout.


5. Call `Write()` and `Read()` for data transfer.


6. Call `Disconnect()` to close the socket.


7. Call `Destroy()` to free memory.



## 6. Primary APIs

* `bool Initialise()`: Allocates the underlying ESP-IDF handle.


* `bool SetCert(const char* certPem)`: Sets the certificate for TLS endpoints.


* `bool Connect(const char* host, uint16_t port, int timeout_ms)`: Establishes the socket.


* `int Read(uint8_t* buffer, std::size_t len, int timeout_ms)`: Reads incoming data with a timeout.


* `int Write(const uint8_t* buffer, std::size_t len, int timeout_ms)`: Transmits data with a timeout.


* `void Disconnect()`: Closes the active socket without destroying the handle context.



## 7. Application Constraints

### Memory

* The `certPem` string passed to `SetCert()` must remain valid in memory for the lifetime of the `TlsTransport` object or until `Destroy()` is called. The library does not copy the certificate string; it only stores a pointer.



### Blocking Behavior

* `Connect()`, `Read()`, and `Write()` are blocking calls. They will yield to other FreeRTOS tasks until the operation completes or the specified `timeout_ms` expires.



### Error Handling

* The application is responsible for its own retry logic. If `Read()` or `Write()` returns `< 0`, the application should consider the connection dead, call `Disconnect()`, and attempt to reconnect.