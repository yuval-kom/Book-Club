
#include <iostream>
#include "../include/User.h"

User::User(std::mutex &mutexInventory)
        : userName(), inventory(), subscriptions(), wishToBorrow(), borrowBooks(), receipts(),
          counterSubID(0), counterJoin(2), counterExit(1), _mutexInventory(mutexInventory) {}

User::~User() {
    for (auto &g: inventory) {
        g.second.clear();
    }
    inventory.clear();
    subscriptions.clear();
    for (auto &g: wishToBorrow) {
        g.second.clear();
    }
    wishToBorrow.clear();
    borrowBooks.clear();
    receipts.clear();
}

void User::setUserName(std::string name) {
    this->userName = name;
}

void User::joinReceipt(std::string genre) {
    counterJoin++;
    counterJoin++;
    counterSubID++;
    std::pair<std::string, std::pair<std::string, std::string>> toAdd = {std::to_string(counterJoin),
                                                                         {std::to_string(counterSubID), genre}};
    receipts.insert(toAdd);
}

std::string User::exitReceipt(std::string genre) {
    counterExit++;
    counterExit++;
    std::string sub = "";
    for (auto &s: subscriptions) {
        if (s.second == genre) {
            sub = s.first;
        }
    }
    std::pair<std::string, std::pair<std::string, std::string>> toAdd = {std::to_string(counterExit), {sub, genre}};
    receipts.insert(toAdd);
    return sub;
}

std::string User::getCounterSubID() {
    return std::to_string(counterSubID);
}

std::string User::getCounterJoin() {
    return std::to_string(counterJoin);
}

std::string User::getCounterExit() {
    return std::to_string(counterExit);
}

void User::addBook(std::string bookName, std::string genre) {
    std::lock_guard<std::mutex> lock(_mutexInventory);
    if (inventory.count(genre) == 0) {
        inventory.insert({genre, {bookName}});
    } else {
        for (auto &i: inventory) {
            if (i.first == genre) {
                i.second.push_back(bookName);
            }
        }
    }
}

void User::addWishToBorrow(std::string genre, std::string bookName) {
    if (wishToBorrow.count(genre) == 0) {
        std::pair<std::string, std::vector<std::string>> toInsert = {genre, {bookName}};
        wishToBorrow.insert(toInsert);
    } else {
        for (auto &w: wishToBorrow) {
            if (w.first == genre) {
                w.second.push_back(bookName);
            }
        }
    }
}

std::string User::removeBook(std::string genre, std::string book, bool flag) {
    std::lock_guard<std::mutex> lock(_mutexInventory);
    for (auto &b: inventory) {
        if (b.first == genre) {
            int i = 0;
            for (auto &v: b.second) {
                if (v == book) {
                    b.second.erase(b.second.begin() + i, b.second.begin() + i + 1);
                }
                i++;
            }
        }
    }
    if (flag) {
        int i = 0;
        std::string owner;
        for (auto &b: borrowBooks) {
            if ((b.second.second == book) & (b.second.first == genre)) {
                owner = b.first;
                borrowBooks.erase(borrowBooks.find(owner));
                return owner;
            }
            i++;
        }
    }
    return "";
}

bool User::checkReceiptId(std::string receipt) {
    for (auto &r: receipts) {
        if (r.first == receipt) {
            return true;
        }
    }
    return false;
}

void User::subscribeToGenre(std::string receiptId) {
    std::pair<std::string, std::string> toInsert;
    for (auto &r: receipts) {
        if (r.first == receiptId) {
            toInsert = r.second;
            receipts.erase(receiptId);
            break;
        }
    }
    subscriptions.insert(toInsert);
}

void User::unsubscribeFromGenre(std::string receiptId) {
    std::string subId;
    for (auto &r: receipts) {
        if (r.first == receiptId) {
            subId = r.second.first;
            receipts.erase(receiptId);
            break;
        }
    }
    subscriptions.erase(subId);
}

bool User::ifIHaveTheBook(std::string book, std::string genre) {
    for (auto &b: inventory) {
        if (b.first == genre) {
            for (auto &v: b.second) {
                if (v == book) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::string User::getUserName() {
    return userName;
}

std::vector<std::string> User::booksOfGenre(std::string genre) {
    for (auto &b: inventory) {
        if (b.first == genre) {
            return b.second;
        }
    }
    return std::vector<std::string>();
}

bool User::ifILookingFor(std::string genre, std::string book, std::string owner) {
    bool found = false;
    for (auto &b: wishToBorrow) {
        if (b.first == genre) {
            int i = 0;
            for (auto &v: b.second) {
                if (v == book) {
                    b.second.erase(b.second.begin() + i, b.second.begin() + i + 1);
                    std::pair<std::string, std::pair<std::string, std::string>> toAdd = {owner, {genre, book}};
                    borrowBooks.insert(toAdd);
                    found = true;
                }
                i++;
            }
        }
    }
    if (found) {
        addBook(book, genre);
    }
    return found;
}
