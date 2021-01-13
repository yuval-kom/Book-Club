package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Connections_Impl;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<T> {
    private boolean _shouldTerminate = false;
    private Connections_Impl _connections;
    private int _connectionId;
    private ConnectionHandler _connectionH;
    private AtomicInteger messageId;
    private Map<String, String> StompFrame;

    public void start(int connectionId, Connections<String> connections, ConnectionHandler<T> connectionHandler) {
        _connections = (Connections_Impl) connections;
        _connectionId = connectionId;
        _connectionH = connectionHandler;
        messageId = new AtomicInteger(1);
    }

    public void process(T message) {
        StompFrame = new HashMap<>();
        String receivedMsg = message.toString();
        msgToFrame(receivedMsg);
        switch (StompFrame.get("command")) {
            case ("DISCONNECT"):
                disconnect();
                break;

            case ("CONNECT"):
                connect();
                break;

            case ("SUBSCRIBE"):
                subscribe();
                break;

            case ("SEND"):
                sendMessage();
                break;

            case ("UNSUBSCRIBE"):
                unsubscribe();
                break;
        }

    }

    private void connect() {
        String frameToSend;
        String userName = StompFrame.get("login");
        String password = StompFrame.get("passcode");
        String version = StompFrame.get("accept-version");
        if (!_connections.checkIfConnected(_connectionId)) {
            _connections.connect(_connectionId, _connectionH);
            if (!_connections.checkIfUserExist(userName, password)) {
                frameToSend = "CONNECTED\nversion:" + version + "\n\n";
                _connections.updateActiveUsersNames(_connectionId, userName);
            } else {
                if (_connections.checkIfTheUserIsActive(userName)) {
                    frameToSend = "ERROR\nreceipt-id:message" + messageId.getAndIncrement() + "\n\nmessage: User already logged in\n";
                    _shouldTerminate = true;
                } else if (!_connections.isPasswordValid(userName, password)) {
                    frameToSend = "ERROR\nreceipt-id:message" + messageId.getAndIncrement() + "\n\nmessage: Wrong password\n";
                    _shouldTerminate = true;
                } else {
                    frameToSend = "CONNECTED\nversion:" + version + "\n\n";
                    _connections.updateActiveUsersNames(_connectionId, userName);
                }
            }
        } else {
            frameToSend = "ERROR\nreceipt-id: message-" + messageId.getAndIncrement() + "\n\nmessage: the connection is already used\n";
            _shouldTerminate = true;
        }
        if (!_connections.send(_connectionId, frameToSend)) {
            _shouldTerminate = true;
        }
    }

    private void subscribe() {
        String topic = StompFrame.get("destination");
        String subscription = StompFrame.get("id");
        _connections.addTopic(topic, _connectionId, subscription);
        if (StompFrame.containsKey("receipt")) {
            String receipt = StompFrame.get("receipt");
            String frameToSend = "RECEIPT\nreceipt-id:" + receipt + "\n\nJoined club " + topic + "\n";
            if (!_connections.send(_connectionId, frameToSend))
                _shouldTerminate = true;
        }
    }

    private void sendMessage() {
        String topic = StompFrame.get("destination");
        String subscription = _connections.getSubscription(topic, _connectionId);
        String frameToSend = "MESSAGE\nsubscription:" + subscription + "\nMessage-id:" + messageId.getAndIncrement() + "\n" + headersToSend() + "\n" + StompFrame.get("body") + "\n";
        _connections.send(topic, frameToSend);
    }

    private void unsubscribe() {
        if (StompFrame.containsKey("receipt")) {
            String subscription = StompFrame.get("id");
            String topic = _connections.unsubscribe(subscription, _connectionId);
            String receipt = StompFrame.get("receipt");
            String frameToSend = "RECEIPT\nreceipt-id:" + receipt + "\n\nExits club " + topic + "\n";
            if (!_connections.send(_connectionId, frameToSend))
                _shouldTerminate = true;
        }
    }

    private void disconnect() {
        if (StompFrame.containsKey("receipt")) {
            String receipt = StompFrame.get("receipt");
            String frameToSend = "RECEIPT\nreceipt-id:" + receipt + "\n\n";
            _connections.send(_connectionId, frameToSend);
        }
        _connections.disconnect(_connectionId);
        _shouldTerminate = true;
    }

    private void msgToFrame(String receivedMsg) {
        int index = receivedMsg.indexOf('\n');
        String command = receivedMsg.substring(0, index);
        StompFrame.put("command", command);
        String headersAndBody = receivedMsg.substring(index + 1);
        String header;
        while (headersAndBody.charAt(0) != '\n') {
            index = headersAndBody.indexOf('\n');
            header = headersAndBody.substring(0, index);
            int separate = header.indexOf(':');
            StompFrame.put(header.substring(0, separate), header.substring(separate + 1));
            headersAndBody = headersAndBody.substring(index + 1);
        }
        headersAndBody = headersAndBody.substring(1);
        StompFrame.put("body", headersAndBody);
    }

    private String headersToSend() {
        String output = "";
        for (HashMap.Entry<String, String> header : StompFrame.entrySet()) {
            if (header.getKey() != "command" & header.getKey() != "body") {
                output += header.getKey() + ":" + header.getValue() + '\n';
            }
        }
        return output;
    }

    public boolean shouldTerminate() {
        return _shouldTerminate;
    }
}
