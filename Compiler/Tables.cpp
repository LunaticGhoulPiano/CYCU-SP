#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace std;

struct t5 {
    string identifier;
    int subroutine;
    int type;
    int pointer;
};

class Tables {
    public:
        vector<string> table1 = {"", // index 0 nothing // Delimiter Table
                                 ";",
                                 "(",
                                 ")",
                                 "=",
                                 "+",
                                 "-",
                                 "*",
                                 "/",
                                 "¡ô", // UTF-8: 0x2191
                                 "\'",
                                 ",",
                                 ":"},
                       table2 = {"", // index 0 nothing // Reserved Word Table
                                 "AND",
                                 "BOOLEAN",
                                 "CALL", // statement
                                 "DIMENSION", // statement
                                 "ELSE",
                                 "ENP",
                                 "ENS",
                                 "EQ",
                                 "GE",
                                 "GT",
                                 "GTO", // statement
                                 "IF", // statement
                                 "INPUT", // statement
                                 "INTEGER",
                                 "LABEL", // statement
                                 "LE",
                                 "LT",
                                 "NE",
                                 "OR",
                                 "OUTPUT", // statement
                                 "PROGRAM", // statement
                                 "REAL",
                                 "SUBROUTINE", // statement
                                 "THEN",
                                 "VARIABLE"}; // statement
        vector<int> table3; // Integer table
        vector<float> table4; // Real table
        vector<t5> table5; // Identifier Table
        vector<string> table6; // result

        Tables() {
            table3.push_back(-1); // index put start from 1
            table4.push_back(-1.1); // index put start from 1
        }

        bool isDelimiter(string str) {
            if (str == "¡ô") return true;
            bool temp = false;
            for (int i = 1; i < table1.size(); i++) {
                if (str == table1[i]) temp =  true;
            }

            return temp;
        }

        vector<int> find_coordinates(string token) {
            if (token == "¡ô") return {1, 9};
            vector<int> v;

            for (int i = 1; i < table2.size(); i++) {
                if (i < 13 && table1[i] == token) {
                    v.push_back(1);
                    v.push_back(i);
                    return v;
                }
                else if (table2[i] == token) {
                    v.push_back(2);
                    v.push_back(i);
                    return v;
                }
            }

            bool isInt = false, isReal = false;
            for (int j = 0; j < token.size(); j++) {
                if ('0' <= token[j] && token[j] <= '9') isInt = true;
                else if (token[j] == '.') isReal = true;
                else break;
            }

            if (isInt && ! isReal) {
                for (int i = 1; i < table3.size(); i++) {
                    if (table3[i] == stoi(token)) {
                        v.push_back(3);
                        v.push_back(i);
                        return v;
                    }
                }
            }
            else if (isInt && isReal) {
                for (int i = 1; i < table4.size(); i++) {
                    if (table4[i] == stof(token)) {
                        v.push_back(4);
                        v.push_back(i);
                        return v;
                    }
                }
            }

            // table5
            return v;
        }
};