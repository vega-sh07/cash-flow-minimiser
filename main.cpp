#include <iostream>

#include "person.h"
#include "settlement.h"
#include "transaction_manager.h"
#include "cashflowminimizer.h"
#include "menu.h"

using namespace std;

int main() {
    vector<Person> people;
    TransactionManager manager;
    CashFlowMinimizer minimizer;
    vector<Settlement> paymentHistory;

    while (true) {
        printMenu();
        int choice;
        cout << "Enter choice: ";
        if (!(cin >> choice)) {
            clearBadInput();
            cout << "Please type a number from 1 to 9.\n";
            continue;
        }

        switch (choice) {
            case 1: addPerson(people); break;
            case 2: splitExpense(people, manager); break;
            case 3: processPending(manager); break;
            case 4: manager.displayLog(people); break;
            case 5: showGraph(people, manager); break;
            case 6: settle(people, manager, minimizer, paymentHistory); break;
            case 7: undoLast(manager); break;
            case 8: showSettlementHistory(people, paymentHistory); break;
            case 9: cout << "Goodbye!\n"; return 0;
            default: cout << "Invalid choice. Pick 1 to 9.\n";
        }
    }
}
