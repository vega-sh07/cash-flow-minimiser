#pragma once

#include <string>
using namespace std;

class Person {
private:
    int id;
    string name;
    long long net_balance;

public:
    Person(int ID, string n) : id(ID), name(n), net_balance(0) {}

    int getId() const { return id; }
    string getName() const { return name; }
    long long getNetBalance() const { return net_balance; }

    void updateBalance(long long amount) {
        net_balance += amount;
    }
    void resetBalance() {
        net_balance = 0;
    }
};