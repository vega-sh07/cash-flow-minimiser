#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include "person.h"
#include "transaction.h"
#include "graph.h"
#include "inputhelpers.h"

using namespace std;

class TransactionManager {
private:
    vector<Transaction> log;
    queue<Transaction> pending;
    stack<int> history;
    vector<Transaction> settledLog;
    int nextId;

    // merge sort
    static bool before(const Transaction& a, const Transaction& b) {
        if(a.getDate() != b.getDate())
            return a.getDate() < b.getDate();
        return a.getId() < b.getId();
    }

    static void mergeHalves(vector<Transaction>& v, int l, int m, int r) {
        vector<Transaction> temp;
        temp.reserve(r - l + 1);
        int i = l, j = m + 1;
 
        while (i <= m && j <= r) {
            if (before(v[j], v[i])) temp.push_back(v[j++]);
            else temp.push_back(v[i++]);
        }
        while (i <= m) temp.push_back(v[i++]);
        while (j <= r) temp.push_back(v[j++]);
 
        for (int k = 0; k < (int)temp.size(); k++)
            v[l + k] = temp[k];
    }

    static void mergeSort(vector<Transaction>& v, int l, int r) {
        if (l >= r) return;
        int m = l + (r - l) / 2;
        mergeSort(v, l, m);
        mergeSort(v, m + 1, r);
        mergeHalves(v, l, m, r);
    }

public:
    TransactionManager() : nextId(1) {}

    int pendingCount() const { return (int)pending.size(); }
    int logSize() const { return (int)log.size(); }

    // Validates a new transaction and puts it in the pending queue.
    bool submit(int from, int to, long long amt, const string& date,
                const string& dueDate, double rate, long long penalty,
                int numPeople, string& error) {
        if (from < 0 || from >= numPeople || to < 0 || to >= numPeople) {
            error = "Unknown person id.";
            return false;
        }
        if (from == to) {
            error = "Lender and borrower cannot be the same person.";
            return false;
        }
        if (amt <= 0) {
            error = "Amount must be positive.";
            return false;
        }
        if (!isValidDate(date) || !isValidDate(dueDate)) {
            error = "Dates must be valid and in YYYY-MM-DD format.";
            return false;
        }
        if (dueDate < date) {
            error = "Due date cannot be before the transaction date.";
            return false;
        }
        if (rate < 0 || penalty < 0) {
            error = "Interest rate and penalty cannot be negative.";
            return false;
        }

        Transaction t(nextId, from, to, amt, date, dueDate, rate, penalty);

        // duplicate check against committed ledger
        for (const Transaction& x : log) {
            if (x.sameAs(t)) {
                error = "Duplicate transaction (already in the ledger).";
                return false;
            }
        }
        // duplicate check against the pending queue
        queue<Transaction> tmp = pending;
        while (!tmp.empty()) {
            if (tmp.front().sameAs(t)) {
                error = "Duplicate transaction (already pending).";
                return false;
            }
            tmp.pop();
        }

        pending.push(t);
        nextId++;
        return true;
    }

    // moves every pending transaction into the ledger
    int processPending() {
        int count = 0;
        while (!pending.empty()) {
            Transaction t = pending.front();
            pending.pop();
            log.push_back(t);
            history.push(t.getId());
            count++;
        }
        if (count > 0 && !log.empty())
            mergeSort(log, 0, (int)log.size() - 1);
        return count;
    }

    bool undoLast(int& removedId) {
        if (history.empty()) return false;
        removedId = history.top();
        history.pop();
        for (size_t i = 0; i < log.size(); i++) {
            if (log[i].getId() == removedId) {
                log.erase(log.begin() + i);
                break;
            }
        }
        return true;
    }

    // Moves every ledger transaction dated on or before currentDate into the settled archive. Future-dated transactions stay in the ledger.
    // Settled transactions are also removed from the undo history.
    int markAllSettled(const string& currentDate) {
        vector<Transaction> remaining;
        int count = 0;
        for (const Transaction& t : log) {
            if (t.getDate() <= currentDate) {
                settledLog.push_back(t);
                count++;
            } else {
                remaining.push_back(t);
            }
        }
        log = remaining;

        vector<int> kept;
        while (!history.empty()) {
            int id = history.top();
            history.pop();
            for (const Transaction& t : log)
                if (t.getId() == id) { kept.push_back(id); break; }
        }
        for (int i = (int)kept.size() - 1; i >= 0; i--)
            history.push(kept[i]);
        return count;
    }

    int futureCount(const string& currentDate) const {
        int count = 0;
        for (const Transaction& t : log)
            if (t.getDate() > currentDate) count++;
        return count;
    }

    int settledCount() const { return (int)settledLog.size(); }

    void computeBalances(vector<Person>& people, const string& currentDate,
                         Graph& graph) const {
        for (Person& p : people) p.resetBalance();

        for (const Transaction& t : log) {
            if (t.getDate() > currentDate)
                continue;
            long long total = t.totalDue(currentDate);
            people[t.getFromId()].updateBalance(total);
            people[t.getToId()].updateBalance(-total);
            graph.addEdge(t.getToId(), t.getFromId(), total);
        }
    }

    void displayLog(const vector<Person>& people) const {
        if (log.empty()) {
            cout << "The ledger is empty.\n";
            return;
        }
        cout << left << setw(5) << "ID" << setw(12) << "Date"
             << setw(12) << "Lender" << setw(12) << "Borrower"
             << setw(10) << "Amount" << setw(12) << "Due"
             << setw(8) << "Rate%" << setw(8) << "Penalty" << "\n";
        cout << string(79, '-') << "\n";
        for (const Transaction& t : log) {
            cout << left << setw(5) << t.getId() << setw(12) << t.getDate()
                 << setw(12) << people[t.getFromId()].getName()
                 << setw(12) << people[t.getToId()].getName()
                 << setw(10) << t.getAmount() << setw(12) << t.getDueDate()
                 << setw(8) << t.getInterestRate()
                 << setw(8) << t.getPenalty() << "\n";
        }
    }
};