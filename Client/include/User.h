
#ifndef CLIENT_USER_H
#define CLIENT_USER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <map>

class User {
private:
    std::string userName;
    std::unordered_map<std::string, std::vector<std::string>> inventory; //<genre, books>
    std::unordered_map<std::string, std::string> subscriptions; //<subscriptionId, genre>
    std::unordered_map<std::string, std::vector<std::string>> wishToBorrow; //<genre, booksName>
    std::unordered_map<std::string, std::pair<std::string, std::string>> borrowBooks; //<owner, <genre, bookName>>
    std::unordered_map<std::string, std::pair<std::string, std::string>> receipts; //<receiptId, <subscriptionId, genre>>
    int counterSubID;
    int counterJoin;
    int counterExit;
    std::mutex &_mutexInventory;

public:
    User(std::mutex &mutexInventory);

    ~User();

    void setUserName(std::string name);

    void joinReceipt(std::string genre);

    std::string exitReceipt(std::string genre);

    std::string getCounterSubID();

    std::string getCounterJoin();

    std::string getCounterExit();

    void addBook(std::string bookName, std::string genre);

    void addWishToBorrow(std::string genre, std::string bookName);

    std::string
    removeBook(std::string genre, std::string bookName, bool flag); // flag -> true when returning, false when taking

    bool checkReceiptId(std::string receipt);

    void subscribeToGenre(std::string receiptId);

    void unsubscribeFromGenre(std::string receiptId);

    bool ifIHaveTheBook(std::string book, std::string genre);

    std::string getUserName();

    std::vector<std::string> booksOfGenre(std::string genre);

    bool ifILookingFor(std::string genre, std::string book, std::string owner);
};


#endif //CLIENT_USER_H
