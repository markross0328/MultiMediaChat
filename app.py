from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO
from flask_cors import CORS
from collections import deque

app = Flask(__name__)
CORS(app)
app.config['SECRET_KEY'] = 'your_secret_key'
socketio = SocketIO(app, cors_allowed_origins="*")

# Store last 50 messages (public)
chat_history = deque(maxlen=50)
active_users = set()
private_messages = {}  # key: username, value: list of private messages
user_sid_map = {}      # Map from username to socket ID
sid_user_map = {}       # Map from socket ID to username

@app.route('/')
def index():
    return render_template('index.html', messages=list(chat_history), active_users=list(active_users))

@app.route('/api/disconnect', methods=['POST'])
def handle_disconnect():
    try:
        data = request.get_json(force=True)
        username = data.get('username')
        if username and username in active_users:
            active_users.remove(username)
            # Remove from maps if present
            sid_to_remove = None
            for sid, user in sid_user_map.items():
                if user == username:
                    sid_to_remove = sid
                    break
            if sid_to_remove:
                sid_user_map.pop(sid_to_remove, None)
            user_sid_map.pop(username, None)
            
            socketio.emit('user_disconnected', {'username': username})
            # Clean up private messages for disconnected user
            if username in private_messages:
                del private_messages[username]
        return jsonify({'status': 'success'}), 200
    except Exception as e:
        print(f"Error in disconnect: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 400

@app.route('/api/message', methods=['POST'])
def handle_message():
    try:
        data = request.get_json(force=True)
        message = data.get('message')
        username = data.get('username')
        header = data.get('header', 'TEXT')
        dest = data.get('dest', None)
        
        if message and username:
            # Add user to active users
            active_users.add(username)

            if header == 'PRIVATE' and dest:
                # Private message: send only to the intended recipient
                formatted_message = f"(Private) {username} to {dest}: {message}"
                chat_history.append(formatted_message)

                # Store private message for recipient
                if dest not in private_messages:
                    private_messages[dest] = []
                private_messages[dest].append(formatted_message)

                # If the recipient is online and identified, send directly
                recipient_sid = user_sid_map.get(dest)
                if recipient_sid:
                    socketio.emit('private_message', {
                        'message': formatted_message,
                        'username': username,
                    }, room=recipient_sid)
            else:
                # Public message: broadcast to all
                formatted_message = f"{username}: {message}"
                chat_history.append(formatted_message)
                socketio.emit('chat_message', {
                    'message': formatted_message,
                    'username': username,
                    'isPrivate': False
                })
            return jsonify({'status': 'success'}), 200
        return jsonify({'status': 'error', 'message': 'Missing username or message'}), 400
    except Exception as e:
        print(f"Error processing message: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 400

@socketio.on('connect')
def handle_connect():
    # Send chat history to newly connected clients
    for message in chat_history:
        socketio.emit('chat_message', {'message': message}, room=request.sid)
    # Send active users list
    socketio.emit('active_users', {'users': list(active_users)}, room=request.sid)

@socketio.on('identify')
def handle_identify(data):
    # Expect data to have 'username'
    username = data.get('username')
    if username:
        user_sid_map[username] = request.sid
        sid_user_map[request.sid] = username
        # If user had queued private messages, send them now
        if username in private_messages:
            for pm in private_messages[username]:
                socketio.emit('private_message', {
                    'message': pm,
                    'username': username
                }, room=request.sid)

@socketio.on('disconnect')
def handle_socket_disconnect():
    sid = request.sid
    if sid in sid_user_map:
        username = sid_user_map.pop(sid)
        user_sid_map.pop(username, None)
    print("A web client disconnected.")

if __name__ == '__main__':
    print("Flask server starting...")
    print("Access the web interface at: http://localhost:5000")
    socketio.run(app, host='127.0.0.1', port=5000, debug=True)
