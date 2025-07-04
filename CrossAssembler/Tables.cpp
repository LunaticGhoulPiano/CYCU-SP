#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace std;

class Tables {
    public:
        vector<vector<string>> tables; // Table1.table ~ Table7.table (insrt., pseudo-instr., reg., delim., symbol, integer/real, string)

        Tables() { // constructor, put in Table1 ~ Table4
            vector<string> empty, table567init(100, "");
            tables.push_back(empty); // Table0 don't put

            for (int i = 1; i < 8; i++) {
                if (i < 5) { // Table1 ~ Table4
                    string filename = "Table" + to_string(i) + ".table";
                    string fileline = "";
                    ifstream file(filename.c_str());

                    vector<string> table;
                    table.push_back(""); // useless Table0

                    for (int j = 1; getline(file, fileline); j++) {
                        fileline.erase(remove_if(fileline.begin(), fileline.end(), ::isspace), fileline.end()); // erase whitespace
                        table.insert(table.begin() + j, fileline);
                    } // end for

                    file.close();
                    tables.push_back(table);
                }
                else tables.push_back(table567init); // Table5~Table7
            }
        }

        bool isDelimiter(string str) {
            if (find(tables[4].begin(), tables[4].end(), str) != tables[4].end()) return true;
            else return false;
        }

        int HashFunction(string value) { // mod = 100
            int sum_ascii = 0;
            if (value[0] == '\'' && value[value.size()-1] == '\'') { // if value is String
                value.erase(value.begin()); // delete the start "\'"
                value.pop_back(); // delete the end "\'"
            }

            for (char character : value) sum_ascii += static_cast<int>(character); // get sum of ASCIIs in string
            int hashed_index = sum_ascii % 100;
            // cout << "_" << value << "_ after hashed is " << hashed_index << endl;
            return hashed_index;
        }
        
        int put_int_hash_tables(string token, int tableNum) { // put in Table5~Table7, return the index ("y" in (x, y))
            int index = HashFunction(token);
            vector<string>::iterator it = find(tables[tableNum].begin(), tables[tableNum].end(), token); // find in Table(tableNum)

            if (it == tables[tableNum].end()) { // not find in table, first put
                if (tables[tableNum][index].size() == 0) tables[tableNum][index] = token;
                else { // collision
                    for (index += 1; (tables[tableNum][index] != "" && index <= 99) || (index > 99); index++) {
                        if (tables[tableNum][index] == "" && index <= 99) {
                            tables[tableNum][index] = token;
                            break;
                        }
                        else if (index > 99) {
                            tables[tableNum].push_back(token);
                            index = tables[tableNum].size()-1;
                            break;
                        }
                        else {
                            // cout << "ERROR" << endl;
                            break;
                        }
                    }
                }
            }

            return index;
        }
        
        vector<int> find_coordinates(string token) {
            vector<int> v; // table(type of the token), line in table

            for (int i = 1; i < tables.size(); i++) { // table
                for (int j = 0; j < tables[i].size(); j++) {
                    string temp = tables[i][j];
                    if (strcmp(token.c_str(), temp.c_str()) == 0) {
                        v.push_back(i);
                        v.push_back(j);
                        return v;
                    }
                }
            }
            
            return v;
        }
};