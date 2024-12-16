# Multimedia Group Chat Application

## Overview

The **Multimedia Group Chat Application** is a client-server system that supports real-time group text chat and private messaging. The application consists of a **C++ backend server** and a **Flask-based Python frontend**, providing both a web-based user interface and a command-line client for seamless communication between multiple users. This design leverages key operating system concepts such as multithreading, synchronization, and custom protocol design to ensure efficient and scalable performance.

## Features

* **Group Text Chat:** Real-time broadcasting of messages to all connected clients.
* **Private Messaging:** Direct messages between individual users, visible only to the sender and recipient.
* **User Tracking:** Real-time display of active users in the chat.
* **Modular Architecture:** Separation of frontend and backend for scalability and maintainability.
* **Custom Protocol:** Efficient handling of different message types using a lightweight custom protocol.

*Note: Audio and video features are outlined for future enhancements.*

## Directory Structure

```
MultiMediaApp/
├── CLIENT/
│   ├── client              # Compiled C++ client binary (ignored by Git)
│   └── client.c            # C++ client source code
├── SERVER/
│   ├── server              # Compiled C++ server binary (ignored by Git)
│   └── server.c            # C++ server source code
├── protocol/
│   ├── protocol.c          # Protocol handling source code
│   └── protocol.h          # Protocol header file
├── app.py                  # Flask frontend server
├── templates/
│   └── index.html          # Web client HTML template
├── docs/
│   ├── README.md           # Project documentation
│   └── requirements.txt    # Python dependencies
```

## Prerequisites

### Operating System
* **Linux** (Ubuntu 20.04+ recommended)
* **Windows** (with [WSL2](https://docs.microsoft.com/en-us/windows/wsl/install) for compatibility)
* **macOS**

### Software Requirements
* **C++ Compiler:** `g++` (supporting C++17 or later)
* **Python:** Python 3.7+
* **Git:** Version control system
* **Make or CMake:** Build automation tools (optional, if using Makefiles or CMakeLists)

### Libraries and Tools
* **Networking Libraries:** Included in standard C++ libraries (`<sys/socket.h>`, `<netinet/in.h>`, etc.)
* **Flask:** Python web framework
* **Flask-SocketIO:** Real-time communication for Flask
* **Flask-CORS:** Cross-Origin Resource Sharing support for Flask
* **OpenCV:** (Optional) For multimedia handling (audio/video)
* **FFmpeg:** (Optional) For audio/video encoding and decoding
* **PortAudio:** (Optional) For audio capture/playback (future enhancement)

### Additional Tools
* **Wireshark:** For packet capturing and debugging
* **Postman or curl:** For testing HTTP endpoints

## Installation

### 1. Clone the Repository

Open your terminal and clone the GitHub repository:

```bash
git clone https://github.com/markross0328/MultiMediaChat.git
cd MultiMediaChat
```

### 2. Set Up the Python Environment

Create and activate a virtual environment:

```bash
python3 -m venv venv
source venv/bin/activate
```

On Windows:
```bash
python -m venv venv
venv\Scripts\activate
```

### 3. Install Python Dependencies

Install required packages:

```bash
pip install -r requirements.txt
```

### 4. Configure and Build the C++ Server

Navigate to the SERVER directory and build the server:

```bash
cd SERVER
g++ -std=c++17 -pthread -o server server.c protocol/protocol.c video/videochat.c -lavcodec -lavformat -lavutil -lswscale -lopencv_core -lopencv_highgui
```

### 5. Build the C++ Client

Navigate to the CLIENT directory and build the client:

```bash
cd ../CLIENT
g++ -std=c++17 -pthread -o client client.c protocol/protocol.c
```

## Running the Application

### 1. Start the Flask Frontend
```bash
# From the root directory (MultiMediaChat)
python3 app.py
```

### 2. Start the C++ Server
```bash
cd SERVER
./server
```

### 3. Run the C++ Client
```bash
cd CLIENT
./client
```

### 4. Access the Web Client
* Open your web browser
* Navigate to http://localhost:5000
* Enter your username when prompted

## Usage

### Logging In

**Web Client:**
* Access http://localhost:5000
* Enter your username when prompted

**C++ Client:**
* Run the client executable
* Enter your username when prompted

### Sending Public Messages

**C++ Client:**
* Type your message and press Enter to broadcast

### Sending Private Messages

**C++ Client:**
```
/msg <recipient_username> <your_message>
```
Example:
```
/msg Alice Hey Alice, how are you?
```

## Troubleshooting

### Flask Server Not Accessible
* Verify app.py is running without errors
* Check port 5000 is not blocked by firewall
* Ensure virtual environment is activated

### C++ Server Not Running
* Check for compilation errors
* Verify all required libraries are installed
* Run in debug mode for detailed error messages

### Client Connection Issues
* Verify server is running
* Check IP and port configurations
* Confirm network connectivity
* Review firewall settings

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Commit changes: `git commit -m "Add feature: Your Feature Name"`
4. Push to branch: `git push origin feature/your-feature-name`
5. Submit a pull request

## License

This project is licensed under the MIT License.

## Contact

* **Name:** Mark Ross
* **Email:** mark.ross@morehouse.edu
* **GitHub:** markross0328