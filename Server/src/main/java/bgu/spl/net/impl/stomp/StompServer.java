package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        int port = Integer.parseInt(args[0]);
        String server = args[1];
        if (server.equals("tpc")) {
            Server.threadPerClient(
                    port,
                    () -> new StompMessagingProtocolImpl<>(), //protocol factory
                    () -> new MessageEncoderDecoderImpl() //message encoder decoder factory
            ).serve();
        } else if (server.equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port,
                    () -> new StompMessagingProtocolImpl<>(), //protocol factory
                    () -> new MessageEncoderDecoderImpl() //message encoder decoder factory
            ).serve();
        }
    }
}
