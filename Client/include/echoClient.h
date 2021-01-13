
#ifndef CLIENT_ECHOCLIENT_H
#define CLIENT_ECHOCLIENT_H

#include <string>
#include "User.h"
#include "connectionHandler.h"

class echoClient{
public:
    static std::string lineToFrame(std::string line,User* user);
    static std::string login(std::string line, std::string &serverHost, short &port,std::string &userName);
    static std::string findBookName(std::vector<std::string> words);
};
#endif //CLIENT_ECHOCLIENT_H
