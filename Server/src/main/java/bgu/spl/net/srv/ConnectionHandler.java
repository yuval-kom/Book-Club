/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package bgu.spl.net.srv;

import java.io.Closeable;
import java.util.Map;

/**
 * The ConnectionHandler interface for Message of type T
 */
public interface ConnectionHandler<T> extends Closeable {

    /**
     * Comment the following lines (both send methods) for the existing implentations to work.
     */

    void send(T msg);

    void addSubscription(String genre, String subscription);

    String removeSubscription(String subscription);

    String getSubscription(String genre);

    boolean isClosed();

}
