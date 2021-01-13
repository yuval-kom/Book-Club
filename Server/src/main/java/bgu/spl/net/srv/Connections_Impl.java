package bgu.spl.net.srv;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class Connections_Impl<T> implements Connections<T> {

    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connectUsers; //<connectionId, ConnectionHandler> //register ports
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer>> topics; //<topic, users connectionId that subscribed to this topic>
    private ConcurrentHashMap<Integer, String> activeUsersNames; //<connectionId, UserName> //users that logged in
    private ConcurrentHashMap<String, String> users; //<userName, password>

    public Connections_Impl() {
        connectUsers = new ConcurrentHashMap<>();
        topics = new ConcurrentHashMap<>();
        activeUsersNames = new ConcurrentHashMap<>();
        users = new ConcurrentHashMap<>();
    }

    public boolean send(int connectionId, T msg) {
        if (!connectUsers.get(connectionId).isClosed()) {
            connectUsers.get(connectionId).send(msg);
            return true;
        }
        //returns false if the connection fails
        return false;
    }

    public void send(String channel, T msg) {
        if (topics.containsKey(channel)) {
            for (Integer i : topics.get(channel)) {
                connectUsers.get(i).send(msg);
            }
        }
    }

    public void disconnect(int connectionId) {
        connectUsers.remove(connectionId);
        for (ConcurrentHashMap.Entry<String, ConcurrentLinkedQueue<Integer>> unit : topics.entrySet()) {
            unit.getValue().remove(connectionId);
        }
        activeUsersNames.remove(connectionId);
    }

    public void connect(int connectionId, ConnectionHandler<T> connectionHandler) {
        connectUsers.put(connectionId, connectionHandler);
    }

    public boolean checkIfConnected(int connectionId) {
        return connectUsers.containsKey(connectionId);
    }

    public void addTopic(String topic, int connectionId, String subscription) {
        synchronized (topics) {
            topics.putIfAbsent(topic, new ConcurrentLinkedQueue<>());
        }
        topics.get(topic).add(connectionId);
        connectUsers.get(connectionId).addSubscription(topic, subscription);
    }

    public String unsubscribe(String subscription, int connectionId) {
        String topic = connectUsers.get(connectionId).removeSubscription(subscription);
        if (topics.containsKey(topic)) {
            topics.get(topic).remove(connectionId);
        }
        return topic;
    }

    public String getSubscription(String topic, int connectionId) {
        return connectUsers.get(connectionId).getSubscription(topic);
    }

    public void updateActiveUsersNames(int connectionId, String userName) {
        activeUsersNames.put(connectionId, userName);
    }

    public boolean checkIfTheUserIsActive(String userName) {
        return activeUsersNames.containsValue(userName);
    }

    public boolean isPasswordValid(String userName, String password) {
        return users.get(userName).equals(password);
    }

    public boolean checkIfUserExist(String userName, String password) {
        synchronized (users) {
            if (!users.containsKey(userName)) {
                users.put(userName, password);
                return false;
            }
        }
        return true;
    }
}
