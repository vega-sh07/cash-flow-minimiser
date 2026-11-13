#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "person.h"
#include "settlement.h"
#include "graph.h"
#include "inputhelpers.h"
#include "transaction_manager.h"
#include "cashflowminimizer.h"

using namespace std;

inline void showPeople(const vector<Person>& people) {
    for (const Person& p : people)
        cout << "  [" << p.getId() << "] " << p.getName() << "\n";
}

inline void addPerson(vector<Person>& people) {
    string name;
    while (true) {
        cout << "Enter name: ";
        if (!(cin >> name)) {
            clearBadInput();
            continue;
        }
        if (name.empty()) {
            cout << "Name cannot be empty.\n";
            continue;
        }
        break;
    }
    people.push_back(Person((int)people.size(), name));
    cout << "Added " << name << " with id " << people.size() - 1 << ".\n";
}

inline void splitExpense(vector<Person>& people, TransactionManager& manager) {
    if (people.size() < 2) {
        cout << "Add at least two people first.\n";
        return;
    }
    int n = (int)people.size();
    cout << "People:\n";
    showPeople(people);

    int payer;
    cout << "Who paid the bill (id): ";
    while (!(cin >> payer) || payer < 0 || payer >= n) {
        clearBadInput();
        cout << "Invalid id. Try again: ";
    }

    long long total;
    cout << "Total bill: ";
    while (!(cin >> total) || total < 1 || total > 1000000000000LL) {
        clearBadInput();
        cout << "Invalid amount. Try again: ";
    }

    vector<int> sharers;
    while (true) {
        cout << "Ids of everyone sharing the cost, separated by spaces:\n";
        string line;
        if (!getline(cin >> ws, line)) {
            clearBadInput();
            continue;
        }
        sharers.clear();
        bool ok = true;
        stringstream ss(line);
        string tok;
        while (ss >> tok) {
            try {
                size_t pos;
                int id = stoi(tok, &pos);
                if (pos != tok.size() || id < 0 || id >= n ||
                    find(sharers.begin(), sharers.end(), id) != sharers.end())
                    ok = false;
                else
                    sharers.push_back(id);
            } catch (...) {
                ok = false;
            }
        }
        bool hasOther = false;
        for (int id : sharers)
            if (id != payer) hasOther = true;

        if (ok && hasOther) break;
        cout << "Invalid list. Use valid, non-repeated ids, with at least one person other than the payer.\n";
    }

    string date = readDate("Date of the expense (YYYY-MM-DD): ");
    string dueDate = readDate("Due date for repayment (YYYY-MM-DD): ");
        while (dueDate < date) {
        cout << "Due date cannot be before the expense date. Try again.\n";
        dueDate = readDate("Due date for repayment (YYYY-MM-DD): ");
    }
    double rate;
    cout << "Interest rate % (0 for none): ";
    while (!(cin >> rate) || rate < 0 || rate > 100) {
        clearBadInput();
        cout << "Invalid rate (0 to 100). Try again: ";
    }

    long long penalty;
    cout << "Late penalty per person (0 for none): ";
    while (!(cin >> penalty) || penalty < 0 || penalty > 1000000000000LL) {
        clearBadInput();
        cout << "Invalid penalty. Try again: ";
    }

    long long count = (long long)sharers.size();
    long long base = total / count;
    long long extra = total % count;

    cout << "\nSplit of Rs. " << total << " paid by "
         << people[payer].getName() << ":\n";

    int queued = 0;
    for (int i = 0; i < (int)sharers.size(); i++) {
        int id = sharers[i];
        long long share = base + (i < extra ? 1 : 0);

        if (id == payer) {
            cout << "  " << people[id].getName() << " (payer) keeps their own share of Rs. "
                 << share << "\n";
            continue;
        }

        string error;
        if (manager.submit(payer, id, share, date, dueDate, rate, penalty, n, error)) {
            cout << "  " << people[id].getName() << " owes "
                 << people[payer].getName() << " Rs. " << share << "\n";
            queued++;
        } else {
            cout << "  " << people[id].getName() << ": rejected (" << error << ")\n";
        }
    }
    cout << queued << " transaction(s) queued. Use option 3 to process them.\n";
}

inline void processPending(TransactionManager& manager) {
    int count = manager.processPending();
    if (count == 0) cout << "No pending transactions.\n";
    else cout << "Processed " << count << " transaction(s) into the ledger.\n";
}

inline void undoLast(TransactionManager& manager) {
    int id;
    if (manager.undoLast(id)) cout << "Undid transaction #" << id << ".\n";
    else cout << "Nothing to undo.\n";
}

inline void showGraph(vector<Person>& people, TransactionManager& manager) {
    if (manager.logSize() == 0) {
        cout << "The ledger is empty.\n";
        return;
    }
    string today = readDate("Current date (YYYY-MM-DD): ");
    int future = manager.futureCount(today);
    if (future > 0)
        cout << "Note: " << future << " transaction(s) are dated after " << today << " and are not counted yet.\n";
    Graph g((int)people.size());
    manager.computeBalances(people, today, g);
    cout << "\nDebt graph before settlement:\n";
    g.display(people);
}

inline void settle(vector<Person>& people, TransactionManager& manager,
            const CashFlowMinimizer& minimizer,
            vector<Settlement>& paymentHistory) {
    if (manager.logSize() == 0) {
        cout << "The ledger is empty. Nothing to settle.\n";
        return;
    }
    if (manager.pendingCount() > 0)
        cout << "Note: " << manager.pendingCount() << " pending transaction(s) are NOT included. Process them first.\n";

    string today = readDate("Current date (YYYY-MM-DD): ");

    int future = manager.futureCount(today);
    if (future > 0)
        cout << "Note: " << future << " transaction(s) are dated after " << today << " and are not counted yet.\n";
    if (future == manager.logSize()) {
        cout << "No transactions have happened as of " << today << ". Nothing to settle.\n";
        return;
    }

    Graph rawGraph((int)people.size());
    manager.computeBalances(people, today, rawGraph);

    cout << "\nNet balances:\n";
    for (const Person& p : people) {
        long long b = p.getNetBalance();
        cout << "  " << setw(12) << left << p.getName();
        if (b > 0) cout << "is owed Rs. " << b << "\n";
        else if (b < 0) cout << "owes Rs. " << -b << "\n";
        else cout << "is settled\n";
    }

    cout << "\nDebt graph before settlement:\n";
    rawGraph.display(people);

    vector<Settlement> plan = minimizer.minimize(people);

    cout << "\nOptimised settlement plan:\n";
    if (plan.empty()) {
        cout << "  No payments needed - all debts cancel out.\n";
    } else {
        Graph settledGraph((int)people.size());
        for (const Settlement& s : plan) {
            cout << "  " << people[s.getFrom()].getName() << " pays " << people[s.getTo()].getName() << " Rs. " << s.getAmount() << "\n";
            settledGraph.addEdge(s.getFrom(), s.getTo(), s.getAmount());
        }

        cout << "\nPayments to be made:\n";
        settledGraph.display(people);
    }

    string answer;
    cout << "\nSettle now? This marks the payments as made and clears these debts (y/n): ";
    if (!(cin >> answer))
        clearBadInput();
    if (answer != "y" && answer != "Y") {
        cout << "Settlement cancelled. Nothing was changed.\n";
        return;
    }

    for (const Settlement& s : plan)
        paymentHistory.push_back(Settlement(s.getFrom(), s.getTo(),
                                            s.getAmount(), today));

    int cleared = manager.markAllSettled(today);
    for (Person& p : people)
        p.resetBalance();

    cout << "Settled! " << plan.size() << " payment(s) recorded and " << cleared << " transaction(s) cleared from the ledger.\n";
}

inline void showSettlementHistory(const vector<Person>& people,
                           const vector<Settlement>& paymentHistory) {
    if (paymentHistory.empty()) {
        cout << "No settlements have been made yet.\n";
        return;
    }
    cout << "Settlement history:\n";
    for (const Settlement& s : paymentHistory) {
        cout << "  " << s.getDate() << "  " << people[s.getFrom()].getName() << " paid " << people[s.getTo()].getName() << " Rs. " << s.getAmount() << "\n";
    }
}

inline void printMenu() {
    cout << "\n===== Cash Flow Minimizer =====\n"
         << "1. Add person\n"
         << "2. Add expense\n"
         << "3. Process pending transactions\n"
         << "4. View transaction log\n"
         << "5. Show debt graph\n"
         << "6. Compute optimised settlement and settle\n"
         << "7. Undo last processed transaction\n"
         << "8. View settlement history\n"
         << "9. Exit\n";
}