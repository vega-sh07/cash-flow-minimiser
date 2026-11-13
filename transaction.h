#pragma once

#include <string>
#include <cmath>
using namespace std;

class Transaction {
private:
    int id;
    int fromId;
    int toId;
    long long amount;
    string date;
    string dueDate;
    double interestRate;
    long long penalty;
public:
    Transaction(int i, int from, int to, long long amt, string d, string dd, double rate, long long p) : id(i), fromId(from), toId(to), amount(amt), date(d), dueDate(dd), interestRate(rate), penalty(p) {}

    int getId() const { return id; }
    int getFromId() const { return fromId; }
    int getToId() const { return toId; }
    long long getAmount() const { return amount; }
    string getDate() const { return date; }
    string getDueDate() const { return dueDate; }
    double getInterestRate() const { return interestRate; }
    long long getPenalty() const { return penalty; }
 
    bool isOverdue(const string& currentDate) const {
        return currentDate > dueDate;
    }
 
    long long interestAmount() const {
        return llround(amount * interestRate / 100.0);
    }

    long long totalDue(const string& currentDate) const {
        long long total = amount + interestAmount();
        if (isOverdue(currentDate))
            total += penalty;
        return total;
    }
 
    bool sameAs(const Transaction& o) const {
        return fromId == o.fromId && toId == o.toId && amount == o.amount &&
               date == o.date && dueDate == o.dueDate;
    }
};