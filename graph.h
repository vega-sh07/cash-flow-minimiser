#pragma once

#include <iostream>
#include <vector>
#include <iomanip>
#include "person.h"
using namespace std;

class Graph {
private:
    int v;
    vector<vector<long long>> adjMatrix;

public:
    Graph(int vertices = 0) {
        v = vertices;
        adjMatrix.assign(v, vector<long long>(v, 0));
    }

    void addEdge(int from, int to, long long amount) {
        adjMatrix[from][to] += amount;
    }
 
    void display(const vector<Person>& people) const {
        if (v == 0) {
            cout << "No people available" << endl;
            return;
        }
 
        // cout << "(row owes column)\n";
        cout << setw(12) << "";
        for (int i = 0; i < v; i++)
            cout << setw(12) << people[i].getName();
        cout << endl;
 
        for (int i = 0; i < v; i++) {
            cout << setw(12) << people[i].getName();
            for (int j = 0; j < v; j++)
                cout << setw(12) << adjMatrix[i][j];
            cout << endl;
        }
    }
};