// Student: CYCU EECS 112-1 11020107 Po-Hsun Su
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <typeinfo> // for debug
#include "Tables.cpp"
using namespace std;

struct Line { // Tokens_and_Coordinates
    vector<string> tokens; // ex. {"PROGRAM", "MAIN", ";"}
    vector<vector<int>> coordinates; // ex. {{2, 21}, {5, 3}, {1, 1}}
};

/*
int utf8_to_unicode(string utf8_code)
{
    unsigned utf8_size = utf8_code.length();
    int unicode = 0;

    for (unsigned p=0; p<utf8_size; ++p)
    {
        int bit_count = (p? 6: 8 - utf8_size - (utf8_size == 1? 0: 1)),
            shift = (p < utf8_size - 1? (6*(utf8_size - p - 1)): 0);

        for (int k=0; k<bit_count; ++k)
            unicode += ((utf8_code[p] & (1 << k)) << shift);
    }

    return unicode;
}


string unicode_to_utf8(int unicode)
{
    string s;

    if (unicode>=0 && unicode <= 0x7f)  // 7F(16) = 127(10)
    {
        s = static_cast<char>(unicode);

        return s;
    }
    else if (unicode <= 0x7ff)  // 7FF(16) = 2047(10)
    {
        unsigned char c1 = 192, c2 = 128;

        for (int k=0; k<11; ++k)
        {
            if (k < 6)
                c2 |= (unicode % 64) & (1 << k);
            else
                c1 |= (unicode >> 6) & (1 << (k - 6));
        }

        s = c1;
        s += c2;

        return s;
    }
    else if (unicode <= 0xffff)  // FFFF(16) = 65535(10)
    {
        unsigned char c1 = 224, c2 = 128, c3 = 128;

        for (int k=0; k<16; ++k)
        {
            if (k < 6)
                c3 |= (unicode % 64) & (1 << k);
            else if
                (k < 12) c2 |= (unicode >> 6) & (1 << (k - 6));
            else
                c1 |= (unicode >> 12) & (1 << (k - 12));
        }

        s = c1;
        s += c2;
        s += c3;

        return s;
    }
    else if (unicode <= 0x1fffff)  // 1FFFFF(16) = 2097151(10)
    {
        unsigned char c1 = 240, c2 = 128, c3 = 128, c4 = 128;

        for (int k=0; k<21; ++k)
        {
            if (k < 6)
                c4 |= (unicode % 64) & (1 << k);
            else if (k < 12)
                c3 |= (unicode >> 6) & (1 << (k - 6));
            else if (k < 18)
                c2 |= (unicode >> 12) & (1 << (k - 12));
            else
                c1 |= (unicode >> 18) & (1 << (k - 18));
        }

        s = c1;
        s += c2;
        s += c3;
        s += c4;

        return s;
    }
    else if (unicode <= 0x3ffffff)  // 3FFFFFF(16) = 67108863(10)
    {
        ;  // Actually, there are no 5-bytes unicodes
    }
    else if (unicode <= 0x7fffffff)  // 7FFFFFFF(16) = 2147483647(10)
    {
        ;  // Actually, there are no 6-bytes unicodes
    }
    else
        ;  // Incorrect unicode (< 0 or > 2147483647)

    return "";
}
*/

vector<string> get_token(string str, Tables Ts) {
    vector<string> tokens;
    str.push_back('\n');

    for (int i = 0; str[i] != '\n'; i++) {
        string token = "", tempstr = "", arrow = "↑";
        stringstream ss;
        ss << str[i];
        ss >> tempstr;
        cout << tempstr;
        if (strcmp("↑", tempstr) == 0) cout << tempstr << endl;

        if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') continue;
        else if (Ts.isDelimiter(tempstr)) {
            token = tempstr;
            //token = temp; // table1
        }
        else if ('0' <= str[i] && str[i] <= '9') { // table3 or table4
            string temp = "" + str[i];
            token = temp;
            int j= i+1;
            for (; '0' <= str[j] && str[j] <= '9'; j++) token.push_back(str[j]);
            i = j - 1;

            if (str[i+1] != '.') {
                //cout << token << " ";
                tokens.push_back(token);
                token = "";
                continue; // table3 (integer)
            }
            else { // table4 (real)
                token.push_back('.'); // str[i+1]
                int k = i+2;
                if (! ('0' <= str[k] && str[k] <= '9')) token.push_back('0'); // ex. "123." => "123.0"
                for (; '0' <= str[k] && str[k] <= '9'; k++) token.push_back(str[k]);
                i = k - 1;
            }

            //cout << token << " ";
        }
        else if (('a' <= str[i] && str[i] <= 'z') || ('A' <= str[i] && str[i] <= 'Z') || str[i] == '_') { // table2 (reserved word) or table5 (identifier)
            string temp(1, str[i]);
            token = temp;
            if (str[i+1] == ' ' || str[i+1] == '\t' || str[i+1] == '\n') {
                tokens.push_back(token);
                token = "";
                continue;
            }

            int j = i+1;
            for (; ('a' <= str[j] && str[j] <= 'z') || ('A' <= str[j] && str[j] <= 'Z') || ('0' <= str[j] && str[j] <= '9') || str[j] == '_'; j++) token.push_back(str[j]);
            i = j - 1;
            // cout << token << " ";
        }
        else if (! (str[i+1] == ' ' || str[i+1] == '\t' || str[i+1] == '\n')) {
            string temp = "" + str[i];
            token = temp;
            //cout << token << " ";
        }

        tokens.push_back(token);
    }

    //cout << endl;

    return tokens;
}

vector<vector<int>> classify(vector<string> tokens, Tables &Ts) {
    vector<vector<int>> coordinates;

    for (int i = 0; i < tokens.size(); i++) {
        vector<int> v = Ts.find_coordinates(tokens[i]);

        if (v.empty()) {
            // table3 or table4
            bool isInt = false, isReal = false;
            for (int j = 0; j < tokens[i].size(); j++) {
                if ('0' <= tokens[i][j] && tokens[i][j] <= '9') isInt = true;
                else if (tokens[i][j] == '.') isReal = true;
                else break;
            }

            if (isInt && ! isReal) { // integer
                Ts.table3.push_back(stoi(tokens[i]));
                v = Ts.find_coordinates(tokens[i]);
            }
            else if (isInt && isReal) { // real
                Ts.table4.push_back(stof(tokens[i]));
                v = Ts.find_coordinates(tokens[i]);
            }

            // table5
            else v = {5, 5};
        }

        // cout << v[0] << " " << v[1] << endl;
        coordinates.push_back(v);
    }

    return coordinates;
}

void Analysis(string filename, Tables &Ts) {
    ifstream file(filename.c_str());
    string str = "";
    vector<Line> lines;

    // get tokens
    for (int i= 0; getline(file, str); i++) {
        vector<string> tokens;
        vector<vector<int>> coordinates;

        tokens = get_token(str, Ts);
        coordinates = classify(tokens, Ts);
        
        /*
        for (int j = 0; j < tokens.size(); j++) {
            cout << tokens[j] << " (" << coordinates[j][0] << ", " << coordinates[j][1] << ") ";
        }
        cout << endl;
        */
    }
}

/*
vector<Line> Analysis(string filename, Tables &Ts) {
    ifstream file(filename.c_str());
    string str = "";
    vector<Line> lines;

    // Lexical Analysis
    for (int i = 0; getline(file, str); i++) {
        vector<string> tokens; // ex. {"MOV", "AH", ",", "4CH"}
        vector<vector<int>> coordinates; // ex. {{1, 109}, {3, 1}, {4, 3}, {6, -1}} // -1 means index unknown

        tokens = get_token(str, Ts); // return tokens, every token is a string
        coordinates = classify(tokens, Ts); // return coordinates, every coordinate is a vector<int>, indicates (able number, index in table)
        if (check_lexical_errors(tokens, Ts, i+1)) str = "";

        if (str != "") {
            Line line;
            line.tokens = tokens;
            line.coordinates = coordinates;
            lines.push_back(line);
        }

        str = "";
        tokens.clear();
        coordinates.clear();
    }

    file.close();
    // printAnalysisResult(lines, "Lexical result:"); // Lexical result

    // Syntax Analysis
    check_syntax_errors(lines);
    // printAnalysisResult(lines, "Syntax result:"); // Syntax result

    return lines;
}
*/

int main(void) {
    Tables Ts;
    string filename = "";
    bool fileExist = true;

    cout << Ts.table1[9] << endl;

    do {
        cout << "Input your file (e.g. \"test_input.txt\"): ";
        cin >> filename;
        ifstream isFileExist(filename.c_str());
        if (isFileExist.good()) break;
        else cout << "File dpexn\'t exist! Try again." << endl;
    } while (true);

    Analysis(filename, Ts);

    return 0;
}