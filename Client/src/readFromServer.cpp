
#include <boost/algorithm/string.hpp>
#include <User.h>
#include "../include/readFromServer.h"

void readFromServer::run(ConnectionHandler *connectionHandler, User *user) {
    while (connectionHandler->isOpen()) {
        std::string frame;
        if (connectionHandler->getLine(frame)) { //read from socket & encode
            std::cout << frame << std::endl;
            std::vector<std::string> linesOfReceivedFrame = std::vector<std::string>();
            boost::split(linesOfReceivedFrame, frame, boost::is_any_of("\n"));
            std::string command = linesOfReceivedFrame.at(0);

            if (command == "MESSAGE") {
                std::string subscriptionId = getInfo("subscription", frame);
                std::string genre = getInfo("destination", frame);
                std::string messageBody = getInfo("\n\n", frame);
                std::vector<std::string> words;
                boost::split(words, messageBody, boost::is_any_of(" "));

                if (words.size() > 3 && (words.at(1) == "wish") & (words.at(2) == "to") & (words.at(3) == "borrow")) {
                    std::string bookName = getInfo("borrow", messageBody);
                    if (user->ifIHaveTheBook(bookName, genre)) {
                        std::string response =
                                "SEND\ndestination:" + genre + "\n\n" + user->getUserName() + " has " +
                                bookName;
                        connectionHandler->sendLine(response);
                    }
                } else if ((words.at(0) == "book") | (words.at(0) == "Book")) { //book status
                    std::vector<std::string> books = user->booksOfGenre(genre);
                    std::string booksToSend = user->getUserName() + ":";
                    if (!books.empty()) {
                        bool first = true;
                        for (auto &b: books) {
                            if (first) {
                                booksToSend += b;
                                first = false;
                            } else {
                                booksToSend += "," + b;
                            }
                        }
                    }
                    std::string response = "SEND\ndestination:" + genre + "\n\n" + booksToSend + "\n";
                    connectionHandler->sendLine(response);
                } else if (words.size() > 2 && (words.at(1) == "has") & (words.at(2) != "added")) { //"john has Dune
                    std::string bookName = getInfo("has", messageBody);
                    std::size_t pos2 = messageBody.find(' ');
                    std::string theOwner = messageBody.substr(0, pos2);
                    if (user->ifILookingFor(genre, bookName, theOwner)) {
                        std::string response =
                                "SEND\ndestination:" + genre + "\n\n" + "Taking " + bookName + " from " + theOwner +
                                "\n";
                        connectionHandler->sendLine(response);
                    }
                } else if (words.at(0) == "Taking") {
                    std::size_t pos1 = messageBody.find("Taking");
                    std::string msg = messageBody.substr(pos1 + 7);
                    std::size_t pos2 = msg.find("from");
                    std::string bookName = msg.substr(0, pos2 - 1);
                    std::string theOwner = msg.substr(pos2 + 5);
                    if (theOwner == user->getUserName()) {
                        user->removeBook(genre, bookName, false);
                    }
                } else if (words.at(0) == "Returning") {
                    std::size_t pos1 = messageBody.find("Returning");
                    std::string msg = messageBody.substr(pos1 + 10);
                    std::size_t pos2 = msg.find("to");
                    std::string bookName = msg.substr(0, pos2 - 1);
                    std::string userName = msg.substr(pos2 + 3);
                    if (userName == user->getUserName()) {
                        std::string owner = user->removeBook(genre, bookName, true);
                        if (!owner.empty()) {
                            std::string response =
                                    "SEND\ndestination:" + genre + "\n\nReturning " + bookName + " to " + owner + "\n";
                            connectionHandler->sendLine(response);
                        } else {
                            user->addBook(bookName, genre);
                        }
                    }
                }
            } else if (command == "RECEIPT") {
                std::string receipt = linesOfReceivedFrame.at(1);
                std::size_t pos1 = receipt.find(':');
                std::string receiptId = receipt.substr(pos1 + 1);
                if (!user->checkReceiptId(receiptId)) {
                    //logout
                    connectionHandler->close();
                } else {
                    int commandReceipt = stoi(receiptId);
                    if (commandReceipt % 2 == 0) {
                        //join
                        user->subscribeToGenre(receiptId);
                    } else {
                        //exit
                        user->unsubscribeFromGenre(receiptId);
                    }
                }
            } else if (command == "ERROR") {
                connectionHandler->close();
                std::cout << "Connection error, please write 'bye' and login again" << std::endl;
            }
        } else {
            connectionHandler->close();
        }
    }
}

std::string readFromServer::getInfo(std::string header, std::string frame) {
    std::size_t pos = frame.find(header);
    std::string str;
    if (header == "\n\n") {
        str = frame.substr(pos + header.size());
    } else {
        str = frame.substr(pos + header.size() + 1);
    }
    std::size_t pos2 = str.find('\n');
    return str.substr(0, pos2);
}