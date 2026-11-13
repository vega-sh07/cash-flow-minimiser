#pragma once

#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>
using namespace std;

inline bool isValidDate(const string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-')
        return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(static_cast<unsigned char>(d[i])))
            return false;
    }
    int month = stoi(d.substr(5,2));
    int day = stoi(d.substr(8,2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

inline void clearBadInput() {
    if (cin.eof()) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    cin.clear();
    cin.ignore(10000, '\n');
}

inline string readDate(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        if (!(cin >> s)) {
            clearBadInput();
            continue;
        }
        if (isValidDate(s)) return s;
        cout << "Invalid date. Use the format YYYY-MM-DD.\n";
    }
}

// inline string readLine(const string& prompt) {
//     string s;
//     cout << prompt;
//     if (!getline(cin, s)) {
//         cout << "Input closed. Exiting.\n";
//         exit(0);
//     }
//     size_t a = s.find_first_not_of(" \t\r\n");
//     if (a == string::npos) return "";
//     size_t b = s.find_last_not_of(" \t\r\n");
//     return s.substr(a, b - a + 1);
// }
 
// inline long long readLong(const string& prompt, long long lo, long long hi) {
//     while (true) {
//         string s = readLine(prompt);
//         try {
//             size_t pos;
//             long long val = stoll(s, &pos);
//             if (pos == s.size() && val >= lo && val <= hi)
//                 return val;
//         } catch (...) {}
//         cout << "Invalid input. Enter a whole number between " << lo
//              << " and " << hi << ".\n";
//     }
// }
 
// inline double readDouble(const string& prompt, double lo, double hi) {
//     while (true) {
//         string s = readLine(prompt);
//         try {
//             size_t pos;
//             double val = stod(s, &pos);
//             if (pos == s.size() && val >= lo && val <= hi)
//                 return val;
//         } catch (...) {}
//         cout << "Invalid input. Enter a number between " << lo
//              << " and " << hi << ".\n";
//     }
// }
 
// inline string readDate(const string& prompt) {
//     while (true) {
//         string s = readLine(prompt);
//         if (isValidDate(s)) return s;
//         cout << "Invalid date. Use the format YYYY-MM-DD.\n";
//     }
// }