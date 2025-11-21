# QWebChat

Simple Qt-based web server for local network chat

## Description

QWebChat is a lightweight C++/Qt application that runs a local web server for organizing chat in a local network. Users can connect to the server through a web browser and communicate in real-time.

## Features

- 🌐 Web-based chat interface
- 🔄 Real-time message updates
- 🏠 Local network operation
- ⚡ Lightweight Qt server
- 📱 Support for any device with a browser

## Requirements

- Qt 5.x or higher
- C++ compiler with C++11 support
- CMake or qmake

## Running

```bash
# Run with default ports (HTTP: 8080, WebSocket: 8081)
./ChatServerQT

# Run with custom ports
./ChatServerQT --http 8080 --ws 8081

# Examples with different port configurations
./ChatServerQT --http 9000 --ws 9001
./ChatServerQT --ws 9090 --http 9091
```

**Parameters:**
-`--http` - HTTP server port (default: 8080)
-`--ws` - WebSocket server port (default: 8081)

### Connecting to the chat

1. Start the server on a computer in your local network
2. Open a browser on any device in the same network
3. Go to:`http://[server-ip-address]:[http-port]` (e.g.,`http://192.168.1.100:8080`)
4. Start chatting!
