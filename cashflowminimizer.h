#pragma once

#include <vector>
#include <queue>
#include <utility>
#include <algorithm>
#include "person.h"
#include "settlement.h"

using namespace std;

class CashFlowMinimizer {
public:
    vector<Settlement> minimize(const vector<Person>& people) const {
        priority_queue<pair<long long, int>> creditors, debtors;

        for (const Person& p : people) {
            long long bal = p.getNetBalance();
            if (bal > 0) creditors.push({bal, p.getId()});
            else if (bal < 0) debtors.push({-bal, p.getId()});
        }

        vector<Settlement> result;

        while (!creditors.empty() && !debtors.empty()) {
            pair<long long, int> c = creditors.top(); creditors.pop();
            pair<long long, int> d = debtors.top();   debtors.pop();

            long long amt = min(c.first, d.first);
            result.push_back(Settlement(d.second, c.second, amt));

            c.first -= amt;
            d.first -= amt;

            if (c.first > 0) creditors.push(c);
            if (d.first > 0) debtors.push(d);
        }
        return result;
    }
};