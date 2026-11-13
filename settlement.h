#pragma once

#include <string>
using namespace std;

class Settlement {
private:
    int from;
    int to;
    long long amount;
    string date;
public:
    Settlement(int f, int t, long long amt, string d = "") : from(f), to(t), amount(amt), date(d) {}
    
    int getFrom() const { return from; }
    int getTo() const { return to; }
    long long getAmount() const { return amount; }
    string getDate() const { return date; }
};
