# API Reference

This document provides the complete API reference for the Transport library.

---

## Transport Class

The **Transport** class is an abstract base interface for stream-based communication. It defines the common API for establishing connections, sending, receiving, and disconnecting. Any custom transport (such as future WebSocket or Serial transports) must implement these virtual methods.

### `Initialise()`

Allocates and prepares the underlying resources for the transport.

**Returns:**

* `true`: If the transport initializes successfully.
* `false`: If initialization fails.

### `Destroy()`

Cleans up and frees all resources and handles associated with the transport.

**Returns:**

* `true`: If resources were successfully destroyed.
* `false`: If there were no active resources to destroy.

### `Connect(const char* host, uint16_t port, int timeout_ms)`

Establishes a connection to the specified remote host.

| Parameter | Type | Description |
| --- | --- | --- |
| `host` | `const char*` | The remote server's hostname or IP address. |
| `port` | `uint16_t` | The target port number. |
| `timeout_ms` | `int` | Maximum time to wait for the connection to establish, in milliseconds. |

**Returns:**

* `true`: Connection established successfully.
* `false`: Connection failed.

### `Disconnect()`

Safely closes the active connection without destroying the transport's memory context.

### `Read(uint8_t* buffer, std::size_t len, int timeout_ms)`

Reads incoming bytes from the active connection.

| Parameter | Type | Description |
| --- | --- | --- |
| `buffer` | `uint8_t*` | Pointer to the array where received data will be stored. |
| `len` | `std::size_t` | The maximum number of bytes to read. |
| `timeout_ms` | `int` | Maximum time to wait for incoming data, in milliseconds. |

**Returns:**

* `> 0`: The number of bytes successfully read.
* `0`: Connection closed by the peer.
* `< 0`: Read error or timeout.

### `Write(const uint8_t* buffer, std::size_t len, int timeout_ms)`

Sends a byte buffer over the active connection.

| Parameter | Type | Description |
| --- | --- | --- |
| `buffer` | `const uint8_t*` | Pointer to the data to be transmitted. |
| `len` | `std::size_t` | The number of bytes to send. |
| `timeout_ms` | `int` | Maximum time to wait if the transmission blocks, in milliseconds. |

**Returns:**

* `> 0`: The number of bytes successfully written.
* `< 0`: Write error or timeout.

---

## TcpTransport Class

The **TcpTransport** class implements the **Transport** interface for unsecure TCP communication using the ESP-IDF transport layer.

### `Initialise()`

Initializes the specific TCP transport handle at object creation. The underlying handle is assigned to a protected pointer.

**Returns:**

* `true`: If the TCP transport handle initializes successfully.
* `false`: If initialization fails and the handle is null.

### `Destroy()`

Cleans up the memory allocated by `esp_transport_tcp_init`.

**Returns:**

* `true`: If the handle was valid and successfully destroyed.
* `false`: If the handle was already null.

### `Connect(const char* host, std::uint16_t port, int timeout_ms)`

Handles DNS resolution, socket creation, and the TCP 3-way handshake entirely under the hood using `esp_transport_connect`.

| Parameter | Type | Description |
| --- | --- | --- |
| `host` | `const char*` | The remote server's hostname or IP address. |
| `port` | `std::uint16_t` | The target port number. |
| `timeout_ms` | `int` | Maximum time to wait for the connection to establish, in |

**Returns:**

* `true`: TCP connection established successfully.
* `false`: TCP connection failed or transport handle is null.

### `Disconnect()`

Securely terminates the connection by closing the socket. Crucially for zero-fragmentation, it does not destroy the handle's memory context, allowing `Connect()` to be called again safely.

### `Read(std::uint8_t* buffer, std::size_t len, int timeout_ms)`

Reads incoming data using `esp_transport_read`, which automatically handles the poll/select logic to enforce the FreeRTOS timeout.

| Parameter | Type | Description |
| --- | --- | --- |
| `buffer` | `std::uint8_t*` | Pointer to the array where received data will be stored. |
| `len` | `std::size_t` | The maximum number of bytes to read. |
| `timeout_ms` | `int` | Maximum time to wait for incoming data, in milliseconds. |

**Returns:**

* `> 0`: The number of bytes successfully read.
* `-1`: If the handle is uninitialized, or a read error/timeout occurs.

### `Write(const std::uint8_t* buffer, std::size_t len, int timeout_ms)`

Sends the payload over the active socket using `esp_transport_write`, managing the timeout if the TCP window is full and blocking.

| Parameter | Type | Description |
| --- | --- | --- |
| `buffer` | `const std::uint8_t*` | Pointer to the data to be transmitted. |
| `len` | `std::size_t` | The number of bytes to send. |
| `timeout_ms` | `int` | Maximum time to wait if the transmission blocks, in milliseconds. |

**Returns:**

* `> 0`: The number of bytes successfully written.
* `-1`: If the handle is uninitialized, or a write error/timeout occurs.

**Example:**

```cpp
TcpTransport tcp_client;
tcp_client.Initialise();

if (tcp_client.Connect("192.168.1.10", 80, 5000))
{
    const uint8_t payload[] = "PING";
    tcp_client.Write(payload, sizeof(payload), 2000);
    tcp_client.Disconnect();
}
tcp_client.Destroy();

```

---

## TlsTransport Class

The **TlsTransport** class extends **TcpTransport** to establish encrypted TCP connections using TLS/SSL via the ESP-IDF transport layer. It inherits all read, write, and connection logic directly from **TcpTransport**.

### `SetCert(const char* certPem)`

Sets the PEM certificate data required for TLS verification. This must be called before `Initialise()` if strict server verification is required.

| Parameter | Type | Description |
| --- | --- | --- |
| `certPem` | `const char*` | Null-terminated string containing the PEM-formatted certificate. |

**Returns:**

* `true`: If the certificate pointer is successfully assigned.
* `false`: If the provided certificate pointer is null.

### `Initialise()`

Initializes the secure TLS transport handle using `esp_transport_ssl_init`. If a certificate was provided via `SetCert()`, it applies the PEM data to the transport context.

**Returns:**

* `true`: If the SSL transport handle initializes successfully.
* `false`: If initialization fails and the handle is null.

### `Destroy()`

Cleans up the SSL handle and context when the object is destroyed or when called manually. It also resets the certificate pointer to null.

**Returns:**

* `true`: If the SSL handle was successfully destroyed.
* `false`: If the handle was already null.
