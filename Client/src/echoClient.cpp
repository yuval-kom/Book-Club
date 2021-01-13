#include "../include/echoClient.h"
#include <connectionHandler.h>
#include <readFromServer.h>
#include <thread>
#include <mutex>
#include <boost/algorithm/string.hpp>

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/

int main() {
    std::mutex mutexWrite;
    std::mutex mutexInventory;
    ConnectionHandler *connectionHandler;
    User *user;
    readFromServer *task = new readFromServer();

    const short bufsize = 1024;
    char buf[bufsize];
    std::cin.getline(buf, bufsize);
    std::string line(buf);
    std::string serverHost;
    short port;

    if (line.find("login") != std::string::npos) {
        std::string userName;
        std::string toSend = echoClient::login(line, serverHost, port, userName);
        connectionHandler = new ConnectionHandler(serverHost, port, mutexWrite);
        user = new User(mutexInventory);
        user->setUserName(userName);
        if (!connectionHandler->connect()) {
            std::cerr << "Cannot connect to " << serverHost << ":" << port << std::endl;
            return 1;
        }
        if (!connectionHandler->sendLine(toSend)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            connectionHandler->close();
        }
    } else {
        std::cerr << "Need to login first" << std::endl;
    }
    std::thread threadReadFromServer(&readFromServer::run, task, connectionHandler, user);

    //From here we will see the rest of the echo client implementation:
    while (connectionHandler->isOpen()) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        if (line == "bye") {
            threadReadFromServer.join();
            connectionHandler->close();
        } else {
            std::string toSend = echoClient::lineToFrame(line, user);
            if (!connectionHandler->sendLine(toSend)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                connectionHandler->close();
            }
            if (line.find("logout") != std::string::npos) {
                threadReadFromServer.join();
                connectionHandler->close();
            }
        }
    }
    delete task;
    delete user;
    delete connectionHandler;
    task = nullptr;
    user = nullptr;
    connectionHandler = nullptr;
    return 0;
}

std::string echoClient::lineToFrame(std::string line, User *user) {
    std::vector<std::string> words;
    boost::split(words, line, boost::is_any_of(" "));
    std::string command = words.at(0);
    if (command == "login") {
        std::string host = words.at(1);
        std::string userName = words.at(2);
        std::string password = words.at(3);
        return "CONNECT\naccept-version:1.2\nhost:" + host + "\nlogin:" + userName + "\npasscode:" + password +
               "\n\n";
    } else if (command == "join") {
        std::string destination = words.at(1);
        user->joinReceipt(destination);
        return "SUBSCRIBE\ndestination:" + destination + "\nid:" + user->getCounterSubID() + "\nreceipt:" +
               user->getCounterJoin()
               + "\n\n";
    } else if (command == "exit") {
        std::string destination = words.at(1);
        std::string subscription = user->exitReceipt(destination);
        return "UNSUBSCRIBE\nid:" + subscription + "\n" + "receipt:" + user->getCounterExit() + "\n\n";
    } else if (command == "add") {
        std::string destination = words.at(1);
        std::string bookName = findBookName(words);
        user->addBook(bookName, destination);
        return "SEND\ndestination:" + destination + "\n\n" + user->getUserName() + " has added the book " + bookName +
               "\n";
    } else if (command == "borrow") {
        std::string destination = words.at(1);
        std::string bookName = findBookName(words);
        user->addWishToBorrow(destination, bookName);
        return "SEND\ndestination:" + destination + "\n\n" + user->getUserName() + " wish to borrow " + bookName + "\n";
    } else if (command == "return") {
        std::string destination = words.at(1);
        std::string bookName = findBookName(words);
        std::string owner = user->removeBook(destination, bookName, true);
        return "SEND\ndestination:" + destination + "\n\n" + "Returning " + bookName + " to " + owner + "\n";
    } else if (command == "status") {
        std::string destination = words.at(1);
        return "SEND\ndestination:" + destination + "\n\n" + "book status" + "\n";
    } else if (command == "logout") {
        return "DISCONNECT\nreceipt:1\n\n";
    } else {
        return "";
    }

}

std::string echoClient::login(std::string line, std::string &serverHost, short &port, std::string &userName) {
    std::vector<std::string> words;
    boost::split(words, line, boost::is_any_of(" "));
    std::string host = words.at(1);
    userName = words.at(2);
    std::string password = words.at(3);
    std::vector<std::string> connectionParameters;
    boost::split(connectionParameters, host, boost::is_any_of(":"));
    serverHost = connectionParameters.at(0);
    port = stoi(connectionParameters.at(1));
    return "CONNECT\naccept-version:1.2\nhost:" + host + "\nlogin:" + userName + "\npasscode:" + password +
           "\n\n";
}

std::string echoClient::findBookName(std::vector<std::string> words) {
    std::string output;
    unsigned int i = 2;
    while (i < words.size()) {
        output += words.at(i);
        output += " ";
        i++;
    }
    return output.substr(0, output.size() - 1);
}

