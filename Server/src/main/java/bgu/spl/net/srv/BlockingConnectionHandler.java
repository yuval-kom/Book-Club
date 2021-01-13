package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final StompMessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private Connections<String> connections;
    private volatile boolean connected = true;
    private Map<String, String> subscriptionsNumber; //<subscription, genre>

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, StompMessagingProtocol<T> protocol, Connections<String> connections) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        this.connections = connections;
        subscriptionsNumber = new HashMap<>();
    }

    @Override
    public void run() {
        protocol.start(sock.getPort(), connections, this);
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    protocol.process(nextMessage);
                }
            }

            if (connected) {
                close();
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg) {
        try {
            out.write(encdec.encode(msg));
            out.flush();
        } catch (IOException ex) {
        }
    }

    public void addSubscription(String genre, String subscription) {
        subscriptionsNumber.putIfAbsent(subscription, genre);
    }

    public String removeSubscription(String subscription) {
        String genre = subscriptionsNumber.get(subscription);
        subscriptionsNumber.remove(subscription);
        return genre;
    }

    public String getSubscription(String genre) {
        for (Map.Entry<String, String> unit : subscriptionsNumber.entrySet()) {
            if (unit.getValue().equals(genre)) {
                return unit.getKey();
            }
        }
        return "-1";
    }

    @Override
    public boolean isClosed() {
        return sock.isClosed();
    }
}
