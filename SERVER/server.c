<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Live Chat</title>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/socket.io/4.0.1/socket.io.js"></script>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
            background-color: #f0f2f5;
            color: #1c1e21;
        }

        h1 {
            margin-bottom: 20px;
            color: #1877f2;
            text-align: center;
        }

        .container {
            display: grid;
            grid-template-columns: 3fr 1fr 1fr;
            gap: 20px;
            max-width: 1400px;
            margin: 0 auto;
        }

        .chat-section,
        .users-section,
        .private-section {
            background: white;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            padding: 20px;
            height: 80vh;
        }

        #messages,
        #active-users,
        #private-messages {
            height: calc(100% - 40px);
            overflow-y: auto;
            padding: 10px;
        }

        .section-title {
            font-size: 1.2em;
            margin-bottom: 15px;
            color: #1877f2;
            font-weight: 600;
            border-bottom: 2px solid #e4e6eb;
            padding-bottom: 10px;
        }

        .message {
            margin: 8px 0;
            padding: 10px;
            background: #f0f2f5;
            border-radius: 15px;
            line-height: 1.4;
        }

        .private-message {
            background-color: #e7f3ff;
            border-left: 4px solid #1877f2;
        }

        .user-item {
            padding: 8px 12px;
            margin: 5px 0;
            background-color: #e7f3ff;
            border-radius: 20px;
            font-size: 0.9em;
            color: #1877f2;
        }

        .welcome-message {
            color: #65676b;
            text-align: center;
            padding: 20px;
            background: #e7f3ff;
            border-radius: 10px;
            margin-bottom: 20px;
        }

        .system-message {
            color: #65676b;
            font-style: italic;
            text-align: center;
            padding: 5px;
        }

        /* Scrollbar styling */
        ::-webkit-scrollbar {
            width: 8px;
        }

        ::-webkit-scrollbar-track {
            background: #f1f1f1;
            border-radius: 4px;
        }

        ::-webkit-scrollbar-thumb {
            background: #bcc0c4;
            border-radius: 4px;
        }

        ::-webkit-scrollbar-thumb:hover {
            background: #888;
        }
    </style>
</head>

<body>
    <h1>Live Chat</h1>
    <div class="container">
        <div class="chat-section">
            <div class="section-title">Chat Messages</div>
            <div id="messages">
                <div class="message welcome-message">Welcome to the chat!</div>
                {% for message in messages %}
                <div class="message">{{ message }}</div>
                {% endfor %}
            </div>
        </div>

        <div class="users-section">
            <div class="section-title">Active Users</div>
            <div id="active-users"></div>
        </div>

        <div class="private-section">
            <div class="section-title">Private Messages</div>
            <div id="private-messages">
                {% for message in private_messages %}
                <div class="message private-message">{{ message }}</div>
                {% endfor %}
            </div>
        </div>
    </div>

    <script>
        const socket = io();
        const messagesDiv = document.getElementById('messages');
        const privateMessagesDiv = document.getElementById('private-messages');
        const activeUsersDiv = document.getElementById('active-users');
        const activeUsers = new Set();

        // Add logging for connection
        socket.on('connect', () => {
            console.log('Connected to server with socket ID:', socket.id);
        });

        // Prompt user for their username upon loading the page
        const username = prompt("Enter your username:");
        if (username && username.trim() !== "") {
            console.log('Attempting to identify as:', username.trim());
            socket.emit('identify', { username: username.trim() });
        }

        socket.on('chat_message', (data) => {
            console.log('Received chat message:', data);
            // Only add to main chat if it's not a private message
            if (!data.isPrivate) {
                const messageDiv = document.createElement('div');
                messageDiv.className = 'message';
                messageDiv.textContent = data.message;
                messagesDiv.appendChild(messageDiv);
                messagesDiv.scrollTop = messagesDiv.scrollHeight;
            }

            if (data.username) {
                activeUsers.add(data.username);
                updateActiveUsersList();
            }
        });

        socket.on('private_message', (data) => {
            console.log('Received private message:', data);

            // Only show if user is sender or recipient
            if (data.from === username || data.to === username) {
                const privateDiv = document.createElement('div');
                privateDiv.className = 'message private-message';
                privateDiv.textContent = data.message;
                privateMessagesDiv.appendChild(privateDiv);
                privateMessagesDiv.scrollTop = privateMessagesDiv.scrollHeight;
            }
        });

        socket.on('user_disconnected', (data) => {
            console.log('User disconnected:', data);
            if (data.username && activeUsers.has(data.username)) {
                activeUsers.delete(data.username);
                updateActiveUsersList();

                const messageDiv = document.createElement('div');
                messageDiv.className = 'message system-message';
                messageDiv.textContent = `${data.username} has disconnected`;
                messagesDiv.appendChild(messageDiv);
                messagesDiv.scrollTop = messagesDiv.scrollHeight;
            }
        });

        socket.on('active_users', (data) => {
            console.log('Received active users:', data);
            data.users.forEach(user => activeUsers.add(user));
            updateActiveUsersList();
        });

        function updateActiveUsersList() {
            activeUsersDiv.innerHTML = '';
            activeUsers.forEach(username => {
                const userDiv = document.createElement('div');
                userDiv.className = 'user-item';
                userDiv.textContent = username;
                activeUsersDiv.appendChild(userDiv);
            });
        }
         
        // Add logging for disconnection
        socket.on('disconnect', () => {
            console.log('Disconnected from server');
        });
    </script>
</body>

</html>