# QWebChat

Simple Qt-based web server for local network chat

![alt text](https://img.shields.io/badge/Qt6-6.7.3-blue?style=flat-square&logo=qt) 
![alt text](https://img.shields.io/badge/compatible%20-linux-blue?style=flat-square&logo=linux)
![alt text](https://img.shields.io/badge/compatible%20-windows-blue?style=flat-square)



QWebChat is a lightweight C++/Qt application that runs a local web server for organizing chat in a local network. Users can connect to the server through a web browser and communicate in real-time.



<img width="1918" height="914" alt="image" src="https://github.com/user-attachments/assets/a32e3c8d-b691-4129-90df-fe22d7b25bac" />


---------------------------------------


## Features

- 🌐 Web-based chat interface
- 🔄 Real-time message updates
- 🏠 Local network operation
- ⚡ Lightweight Qt server
- 📱 Support for any device with a browser
- 💾 Saving chat history to .csv file and loading for new chat clients

## Requirements

- Qt 6.x or higher
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
