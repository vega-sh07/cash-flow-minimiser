#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>

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
    float getNetBalance() const { return net_balance; }

    void updateBalance(float amount) {
        net_balance += amount;
    }

    void resetBalance() {
        net_balance = 0;
    }
};

/// lender(fromId) gave money to borrower (toId)
class Transaction {
private:
    int id;
    int fromId;
    int toId;
    long long amount;
    string date;
    string dueDate;
    double interestRate;
    double penalty;

public:
    Transaction(int i, int from, int to, long long amt, string d, string dd,
                double rate, long long p)
        : id(i), fromId(from), toId(to), amount(amt), date(d), dueDate(dd),
          interestRate(rate), penalty(p) {}

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

    // principal + interest (+ penalty if overdue on currentDate)
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

class Settlement {
private:
    int from;
    int to;
    long long amount;
    string date;

public:
    Settlement(int f, int t, long long amt, string d = "")
        : from(f), to(t), amount(amt), date(d) {}

    int getFrom() const { return from; }
    int getTo() const { return to; }
    long long getAmount() const { return amount; }
    string getDate() const { return date; }
};

class Graph {
private:
    int v;
    vector<vector<long long>> adjMatrix;

public:
    Graph(int vertices) {
        v = vertices;
        adjMatrix.assign(v, vector<long long> (v, 0));
    }

    void addEdge(int u, int v, long long amount) {
        adjMatrix[u][v] += amount;
    }

    void buildFromBalances (const vector<Person>& balances) {
        v = balances.size();
        adjMatrix.assign(v, vector<long long> (v, 0));

        vector<long long> credit(v, 0);
        vector<long long> debt(v, 0);

        for (int i = 0; i < v; i++) {
            if(balances[i].getNetBalance() > 0)
                credit[i] = balances[i].getNetBalance();
            else if (balances[i].getNetBalance() < 0)
                debt[i] = -balances[i].getNetBalance();
        }

        for (int i = 0; i < v; i++) {
            if(debt[i] <= 0)
                continue;

            for (int j = 0; j < v; j++) {
                if (credit[j] <= 0)
                    continue;

                float amount = (debt[i] < credit[j]) ? debt[i] : credit[j];

                addEdge(i, j, amount);
                debt[i] -= amount;
                credit[j] -= amount;

                if (debt[i] <= 0)
                    break;
            }
        }
    }

    void display(const vector<Person>& people) const {
        if (v == 0) {
            cout << "No people available." << endl;
            return;
        }

        cout << "Graph\n";
        cout << setw(12) << "";

        for(int i = 0; i < v; i++) {
            cout << setw(12) << people[i].getName();
        }
        cout << endl;


    }
};

bool isValidDate(const string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-')
        return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(static_cast<unsigned char>(d[i]))) return false;
    }
    int month = stoi(d.substr(5, 2));
    int day = stoi(d.substr(8, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

// queue - pending transactions waiting to be processed (FIFO)
// vector - the committed ledger, kept sorted by date (merge sort)
// stack - history of committed transactions for undo (LIFO)
class TransactionManager {
private:
    vector<Transaction> log;
    queue<Transaction> pending;
    stack<int> history;
    vector<Transaction> settledLog;
    int nextId;

    static bool before(const Transaction& a, const Transaction& b) {
        if (a.getDate() != b.getDate())
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

    // puts it into pending queue
    bool submit(int from, int to, long long amt, const string& date, const string& dueDate, double rate, long long penalty, int numPeople, string& error) {
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

    int processPending() {
        int count = 0;
        while (!pending.empty()) {
            Transaction t = pending.front();
            pending.pop();
            log.push_back(t);
            history.push(t.getId());
            count++;
        }
        if(count > 0 && !log.empty())
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

    // moves every ledger transaction into the settled archive.
    int markAllSettled() {
        int count = (int)log.size();
        for (const Transaction& t : log)
            settledLog.push_back(t);
        log.clear();
        while (!history.empty())
            history.pop();
        return count;
    }

    int settledCount() const { return (int)settledLog.size(); }

    void computeBalances(vector<Person>& people, const string& currentDate, Graph& graph) const {
        for (Person& p : people)
            p.resetBalance();

        for (const Transaction& t : log) {
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