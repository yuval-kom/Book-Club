
#ifndef CLIENT_READFROMSERVER_H
#define CLIENT_READFROMSERVER_H

#include "connectionHandler.h"

using std::string;

class readFromServer {
private:
    string getInfo(string header, string frame);

public:
    void run(ConnectionHandler *connectionHandler, User *user);
};


#endif //CLIENT_READFROMSERVER_H
