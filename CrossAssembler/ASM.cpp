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
#include "Tables.cpp"
#include "OPcodes.cpp"
#include <typeinfo> // for debug
using namespace std;

typedef struct Line { // Tokens_and_Coordinates
    vector<string> tokens; // ex. {"MOV", "AH", ",", "4CH"}
    vector<vector<int>> coordinates; // ex. {{1, 109}, {3, 1}, {4, 3}, {6, -1}}
} NewLine;

vector<string> get_token(string str, Tables Ts) { // input a single line
    vector<string> tokens;
    str.push_back('\n');
    for (int i = 0; str[i] != '\n'; i++) {
        string token = "";
        string s(1, str[i]); // single char to string, to use isDelimiter(string str) in Tables.cpp

        if (str[i] == ';') break;
        else if (str[i] == ':') token = s;
        else if (str[i] == ' ' || str[i] == '\t') {
            if (str[i+1] == '\'') { // ex. Msg BYTE 'Green '
                int j = i + 2;
                for (; str[j] != '\''; j++) token.push_back(str[j]);
                i = j;
                token = "\'" + token + "\'";
            }
            else continue;
        }
        else if (Ts.isDelimiter(s)) {
            if (str[i] != '\'') token = str[i];
            else {
                token.push_back(str[i]);
                int j = i+1;
                
                if (str[j] != '\n') { // not a line only contains "'"
                    for (; str[j] != '\''; j++) {
                        token.push_back(str[j]);
                        // cout << "i is " << i << ", j is " << j << ", str[j] is " << str[j] << ", token is" << token << endl;
                        if (str[j] == '\'') break;
                    }

                    i = j;
                }
            }
        }
        else {
            int j = i;
            stringstream ss;
            string temp = "";

            do
            {
                token.push_back(str[j]);

                // deal with the fucking char to string problem
                ss.str("");
                ss.clear();
                temp = "";
                ss << str[j+1];
                string t = "";
                ss >> temp;
                
                j++;
            } while ((str[j] != ' ') && (str[j] != '\t') && (str[j] != '\n')
                   && (str[j] != ':') && (str[j] != ';') && (! Ts.isDelimiter(temp)));

            i = j - 1;
        }

        // cout << "i is " << i << ", str[i] is " << str[i] << ", token is " + token << endl;
        tokens.push_back(token);
    }

    return tokens;
}

bool is0to9orAtoF(char ch) {
    if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F')) return true;
    else return false;
}

bool isAll0to9orAtoF(string str) {
    bool result = true;
    for (char ch: str) {
        if (! is0to9orAtoF(ch)) {
            result = false;
            break;
        }
    }

    return result;
}

bool is0to9(char ch) {
    if ('0' <= ch && ch <= '9') return true;
    else return false;
}

bool check_lexical_errors(vector<string> tokens, Tables Ts, int line_num) { // check a single line
    bool hasError = false;
    vector<int> types;

    // get token type, 0: not in tables, 1: instr., 2: pseudo-instr., 3: reg., 4: delim.
    for (int i = 0; i < tokens.size(); i++) {
        vector<int> v = Ts.find_coordinates(tokens[i]);
        if (v.size() > 0) types.push_back(v[0]);
        else types.push_back(0); // not in tables
    }
    
    for (int i = 0; i < tokens.size(); i++) {
        // cout << types[i] << endl;
        if (types[i] == 5) {
            // judge is dec
            bool isAll0to9 = true;
            for (char ch: tokens[i]) {
                if (! is0to9(ch)) {
                    isAll0to9 = false;
                    break;
                }
            }
            
            // judgu is hex
            bool isHex = true;
            for (int j = 0; j < tokens[i].size(); j++) {
                if (j != tokens[i].size()-1 && ! is0to9orAtoF(tokens[i][j])) {
                    isHex = false;
                    break;
                }
                else if (j == tokens[i].size()-1 && tokens[i][j] != 'H') isHex = false;
            }

            if (isAll0to9 && stoi(tokens[i]) > 65535) {
                cout << "Line " << line_num << " -> Lexical error: decimal \"" << tokens[i] << "\" can\'t then 65535." << endl;
                hasError = true;
                break;
            }
            else if (isHex && tokens[i].size() > 5) {
                cout << "Line " << line_num << " -> Lexical error: hexadecimal \"" << tokens[i] << "\" can\'t over FFFFH." << endl;
                hasError = true;
                break;
            }
            else if (! ('A' <= tokens[i][0] && tokens[i][0] <= 'Z')) {
                cout << "Line " << line_num << " -> Lexical error: literal only can start with uppercase letter." << endl;
                hasError = true;
                break;
            }
        }
    }

    return hasError;
}

vector<vector<int>> classify(vector<string> tokens, Tables &Ts) {
    vector<vector<int>> coordinates;

    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "") continue;
        vector<int> v = Ts.find_coordinates(tokens[i]);

        if (v.empty()) {
            // cout << "empty "; // not in any table
            // store to table
            string token_head(1, tokens[i][0]), token_tail(1, tokens[i][tokens[i].size()-1]); // get first and last char in a string
            bool isAllIntReal = true, isHex = false; // to check is all in 0~9 or A~F
            int size = tokens[i].size();

            // check is integer or real
            if (token_tail == "H") { // check is hex or not, ex. 1H, 11H, FFFH, FFFFH
                if (1 <= size - 1 && size - 1 <= 4) {
                    for (int j = 0; j < size - 1; j++) { // size - 1 because "H"
                        if (! is0to9orAtoF(tokens[i][j])) {
                            isAllIntReal = false;
                            break;
                        }
                    }

                    if (isAllIntReal) isHex = true;
                }
                else isAllIntReal = false;
                // else may be literal(label), ex. 12345H, H..., etc.
            }
            else if (1 <= size && size <= 5) { // check is decimal or not, ex. 1, 50, 698, 4567, 65535
                for (int j = 0; j < size; j++) {
                    if (! is0to9(tokens[i][j])) {
                        isAllIntReal = false;
                        break;
                    }
                }
            }
            else isAllIntReal = false; // else may be literal(label), ex. 12345, FFFFF..., etc.

            // put in Table5~7
            if (token_head == "\'" && token_tail == "\'") { // String
                v.push_back(7);
                v.push_back(Ts.put_int_hash_tables(tokens[i], 7));
            }
            else if (isAllIntReal) { // isAllIntReal == true, isHex == true/false, i.e. Integer/Real (or Hex)
                v.push_back(6);
                v.push_back(Ts.put_int_hash_tables(tokens[i], 6));
            }
            else { // Literal
                v.push_back(5);
                v.push_back(Ts.put_int_hash_tables(tokens[i], 5));
            }
        }

        coordinates.push_back(v);
    }

    return coordinates;
}

void printAnalysisResult(vector<Line> lines, string result_type) {
    cout << endl << result_type << endl;
    for (int i = 0; i < lines.size(); i++) {
        cout << "Line " << i+1 << ": | ";
        for (int j = 0; j < lines[i].tokens.size(); j++) {
            cout << lines[i].tokens[j] << " at (" << lines[i].coordinates[j][0] << ", " << lines[i].coordinates[j][1] << ") | ";
        }

        cout << endl;
    }

    cout << endl;
}

void check_syntax_errors(vector<Line> &lines) {
    vector<int> error_lines;
    vector<string> token_types = {"", "instruction", "pseudo-instruction", "register", "delimiter", "literal", "integer/real", "string"};

    for (int i = 0; i < lines.size(); i++) { // whole code
        for (int j = 0; j < lines[i].tokens.size(); j++) { // a line in code

            if (lines[i].coordinates[j][0] == 5) { // Table5: Literal (label), correct syntax: literal + (instruction/pseudo-instruction/:)
                int last_index = lines[i].tokens.size() - 1; // ex. "CODE SEGMANT" = > size = 2, last_index = 1
                if (last_index == 0) { // ex. "Mycode"
                    cout << "Line " << i+1 << " -> Syntax error: literal \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (j < last_index && ((lines[i].coordinates[j+1][0] != 1 && lines[i].coordinates[j+1][0] != 2) || (lines[i].tokens[j+1] != ":"))) { // label not at the end
                    if (lines[i].coordinates[j+1][0] != 1 && lines[i].coordinates[j+1][0] != 2 && lines[i].tokens[j+1] != ":") { // ex. "Mycode +"
                        cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can\'t behind literal \"" << lines[i].tokens[j] << "\"." << endl;
                        error_lines.push_back(i);
                        break;
                    }
                    else { // lines[i].tokens[j+1] == ":" (ex. " Msg: BYTE 'Green ' ")
                        if (lines[i].tokens.size() == 2 && lines[i].tokens[j+1] == ":") { // ex. "Mycode:"
                            cout << "Line " << i+1 << " -> Syntax error: literal \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                            error_lines.push_back(i);
                            break;
                        }
                        else if (lines[i].tokens.size() >= 3 && lines[i].tokens[j] == "PROC" && (lines[i].tokens[j+1] == "NEAR" || lines[i].tokens[j+1] == "FAR")) continue; // ex."Mycode: PROC NEAR", originally want to do start-end check
                        else if (lines[i].tokens[j] == "ASSUME" && (lines[i].tokens[j+1] == "CS" || lines[i].tokens[j+1] == "DS")) {
                            if (lines[i].tokens.size() < 4 || (lines[i].tokens.size() == 4 && (lines[i].tokens[j+2] != ":" || lines[i].tokens[j+3] != "CODE"))) { // "ASSUME CS/DS" or "ASSUME CS/DS CODE" or "ASSUME CS/DS :"
                                cout << "Line " << i+1 << " -> Syntax error: you didn\'t assume \"" << lines[i].tokens[j+1] << "\" correctly." << endl;
                                error_lines.push_back(i);
                                break;
                            }
                        }
                        else if (lines[i].tokens[j+1] != ":" && lines[i].coordinates[j+1][0] != 1 && lines[i].coordinates[j+1][0] != 2) { // only "Mycode" + (instruction/pseudo-instruction) are legal
                            cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can\'t behind literal \"" << lines[i].tokens[j] << "\"." << endl;
                            error_lines.push_back(i);
                            break;
                        }
                        else if (lines[i].tokens[j+1] == ":" && lines[i].coordinates[j+2][0] != 1 && lines[i].coordinates[j+2][0] != 2) {
                            cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+2] << "\" can\'t behind literal \"" << lines[i].tokens[j] << ":\"." << endl;
                            error_lines.push_back(i);
                            break;
                        }
                        // else may be like ("Mycode: MOV AX, [SI+1]") (legal)
                        // or ("Normal: PROC:" or "Normal: PROC: NEAR") (error, will check after 2 loops)
                    }
                }
                // else may be like "(Mycode)" => "(", "Mycode", ")" => check in "lines[i].coordinates[j][0] == 4"..., etc.
            }
            else if (lines[i].coordinates[j][0] == 4) { // Table4: Delimiter, correct syntax: delimiter + (register/literal/integer/real/string)
                if (lines[i].tokens.size() == 1) { // ex. ","
                    cout << "Line " << i+1 << " -> Syntax error: delimiter \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (lines[i].tokens[j] == ":" && j != 0) continue; // already check in literal + ":", continue
                else if (j == lines[i].tokens.size()-1) { // ex. "Mycode: MOV DX," (delimiter at the end)
                    if (lines[i].tokens[j] != "]" && lines[i].tokens[j] != ")") { // ex. Mycode: MOV DX, [SI+2]
                        cout << "Line " << i+1 << " -> Syntax error: delimiter \"" << lines[i].tokens[j] << "\" shouldn\'t at the end." << endl;
                        error_lines.push_back(i);
                        break;
                    }
                }
                else if (lines[i].tokens[j] == ",") { // "," can't + (instruction || string)
                    /*
                    "," + pseudo-instruction cases -> legal ", WORD/BYTE PTR":
                    "MOV AX, WORD PTR Msg" => ",": j=2, "WORD/BYTE": j+1=3, "PTR": j+2=4 => size must > 5
                    "label MOV AX, WORD PTR Msg" => ",": j=3, "WORD/BYTE": j+1=4, "PTR": j+2=5 => size must > 6
                    "label: MOV AX, WORD PTR Msg" => ",": j=4, "WORD/BYTE": j+1=5, "PTR": j+2=6 => size must > 7
                    => tokens.size() must > (j+2), i.e. there must have a label after ",", "BYTE/WORD", "PTR"
                    */
                    if (lines[i].tokens.size() > j+2 && (lines[i].tokens[j+1] == "BYTE" || lines[i].tokens[j+1] == "WORD") && lines[i].tokens[j+2] == "PTR" && lines[i].coordinates[j+3][0] == 5) continue;
                    else if ((find(lines[i].tokens.begin(), lines[i].tokens.end(), "BYTE") == lines[i].tokens.end() || find(lines[i].tokens.begin(), lines[i].tokens.end(), "DB") == lines[i].tokens.end()
                             || find(lines[i].tokens.begin(), lines[i].tokens.end(), "WORD") == lines[i].tokens.end() || find(lines[i].tokens.begin(), lines[i].tokens.end(), "DW") == lines[i].tokens.end())
                             && lines[i].tokens.size() >= 5 && lines[i].coordinates[j+1][0] == 7) continue; // ex. LF BYTE '$', 'yg', '$' or "LF: BYTE '$', 12, 'yg', 12H, '$'"
                    else if (lines[i].coordinates[j+1][0] == 1 || lines[i].coordinates[j+1][0] == 2 || lines[i].coordinates[j+1][0] == 7) { // illegal: "," + (instruction/pseudo-instruction/string)
                        cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind delimiter \"" << lines[i].tokens[j] << "\"." << endl;
                        error_lines.push_back(i);
                        break;
                    }
                }
                else if ((lines[i].tokens[j] == "[" || lines[i].tokens[j] == "+" || lines[i].tokens[j] == "-" || lines[i].tokens[j] == "*" || lines[i].tokens[j] == "/" || lines[i].tokens[j] == "(")
                         && lines[i].coordinates[j+1][0] != 3 && lines[i].coordinates[j+1][0] != 6) { // "[", "+", "-", "*", "/", "(" only can + (register || integer/real)
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind delimiter \"" << lines[i].tokens[j] << "\"." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if ((lines[i].tokens[j] == "]" || lines[i].tokens[j] == ")") && lines[i].tokens[j+1] != "+" && lines[i].tokens[j+1] != "-" && lines[i].tokens[j+1] != "*"
                         && lines[i].tokens[j+1] != "/" && lines[i].tokens[j+1] != ",") { // "]" and ")" only can + ("+"||"-"||"*"||"/"||",")
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind delimiter \"" << lines[i].tokens[j] << "\"." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (j == 0) { // ex. ":-1", "+1", etc.
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j] << "\" can't be the first token in a line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                // "?" -> legal ?
            }
            else if (lines[i].coordinates[j][0] == 6) { // Table6: Integer/Real
                if (lines[i].tokens.size() == 1) { // cna't not be a single line, ex. "2" or "2H"
                    cout << "Line " << i+1 << " -> Syntax error:  number \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (j == lines[i].tokens.size()-1) continue; // number at the end, legal
                else if (lines[i].tokens[j+1] != "]" && lines[i].tokens[j+1] != "," && lines[i].tokens[j+1] != "+" && lines[i].tokens[j+1] != "-" && lines[i].tokens[j+1] != "*"
                         && lines[i].tokens[j+1] != "/" && lines[i].tokens[j+1] != ")") { // only integer/real + ("]"||"+"||"-"||"*"||"/"||","||")") are legal
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind number \"" << lines[i].tokens[j] << "\"." << endl;
                    error_lines.push_back(i);
                    break;
                }
            }
            else if (lines[i].coordinates[j][0] == 7) { // Table7: String
                if (lines[i].tokens.size() == 1) { // can't not be a single line, ex. "'Mycode'"
                    cout << "Line " << i+1 << " -> Syntax error:  string \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (j == lines[i].tokens.size()-1) continue; // string at the end, legal
                else if (lines[i].tokens[j+1] != ",") { // only string + "," is legal
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind string \"" << lines[i].tokens[j] << "\"." << endl;
                    error_lines.push_back(i);
                    break;
                }
            }
            else if (lines[i].coordinates[j][0] == 3) { // Table3: Register
                if (lines[i].tokens.size() == 1) { // can't not be a single line, ex. "AH"
                    cout << "Line " << i+1 << " -> Syntax error:  register \"" << lines[i].tokens[j] << "\" can\'t be a single line." << endl;
                    error_lines.push_back(i);
                    break;
                }
                else if (j == lines[i].tokens.size()-1) continue; // register at the end, legal
                else if ((lines[i].tokens[j] == "CS" || lines[i].tokens[j] == "DS") && lines[i].tokens[j+1] == ":") continue; // ex. "ASSUME CS:CODE"
                else if (lines[i].tokens[j+1] != "]" && lines[i].tokens[j+1] != "," && lines[i].tokens[j+1] != "+" && lines[i].tokens[j+1] != "-" && lines[i].tokens[j+1] != "*"
                         && lines[i].tokens[j+1] != "/" && lines[i].tokens[j+1] != ")") { // only register + ("]"||"+"||"-"||"*"||"/"||","||")") are legal
                    cout << "Line " << i+1 << " -> Syntax error: \"" << lines[i].tokens[j+1] << "\" can't behind register \"" << lines[i].tokens[j] << "\"." << endl;
                    error_lines.push_back(i);
                    break;
                }
            }
            // Table1 and Table2 undo
        }
    }

    // delete the error lines
    if (! error_lines.empty()) {
        for (int i = 0; i < error_lines.size(); i++) {
            int index = error_lines[i];
            if (0 <= index && index < lines.size()) {
                lines.erase(lines.begin() + index);

                // because error happens will cnahge size, so adjust index to be delete
                for (int j = i + 1; j < error_lines.size(); j++) {
                    if (error_lines[j] > index) error_lines[j]--;
                }
            }
        }
    }
}

    // pass1 ===================================================================================================================================================================================================================================================== //

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

//---------------------------------------------------------------------------------------------------------//

string pad_bits(const string &str) { // pad to 2 or 4 bits
    // hex to dec
    std::istringstream iss(str);
    int decimalValue;
    iss >> hex >> decimalValue;

    // dec to hex and pad
    ostringstream oss;
    if (str.size() < 3) oss << setw(2) << setfill('0') << uppercase << hex << decimalValue; // ex. "A" => "0A"
    else oss << setw(4) << setfill('0') << uppercase << hex << decimalValue; // ex. "AAA" => "0AAA"

    return oss.str();
}

string pad_addr_bits(const string &str, char ch) { // pad to 4 bits
    // hex to dec
    std::istringstream iss(str);
    int decimalValue;
    iss >> hex >> decimalValue;

    // dec to hex and pad
    ostringstream oss;
    oss << setw(4) << setfill(ch) << uppercase << hex << decimalValue; // ex. "AAA" => "0AAA"

    return oss.str();
}

string DecToHex(const string &dec_str) {
    int decimalValue = stoi(dec_str); // string to int
    ostringstream stream;
    stream << uppercase << hex << decimalValue; // string to hex
    return stream.str();
}

string BinaryToHex(string binary_str) {
    string hex_str = "";

    for (size_t i = 0; i < binary_str.length(); i += 4) { // Iterate through the binary string 4 bits at a time
        string four_bits = binary_str.substr(i, 4); // Extract 4 bits from the binary string
        int decimal_value = bitset<4>(four_bits).to_ulong(); // Convert the 4 bits to an integer

        // Convert the decimal value to its hexadecimal representation
        string hex_digit = "";
        if (decimal_value < 10) {
            hex_digit = static_cast<char>('0' + decimal_value);
        } else {
            hex_digit = static_cast<char>('A' + decimal_value - 10);
        }
        
        hex_str += hex_digit; // Append the hexadecimal digit to the result string
    }

    return hex_str;
}

typedef struct FromatInfo {
    string r_m;
    string w;
    string mod;
} NewFromatInfo;

FromatInfo getInfo(string str, bool isDirectAddress, bool isReg) {
    FromatInfo FI;
    if (isReg) {
        // Segment Registers
        if (str == "ES") FI.r_m = "00";
        else if (str == "CS") FI.r_m = "01";
        else if (str == "SS") FI.r_m = "10";
        else if (str == "DS") FI.r_m = "11";
        // General Registers
        else if (str == "AL" || str == "AX") {
            FI.mod = "11";
            FI.r_m = "000";
            if (str == "AL") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "CL" || str == "CX") {
            FI.mod = "11";
            FI.r_m = "001";
            if (str == "CL") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "DL" || str == "DX") {
            FI.mod = "11";
            FI.r_m = "010";
            if (str == "DL") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "BL" || str == "BX") {
            FI.mod = "11";
            FI.r_m = "011";
            if (str == "BL") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "AH" || str == "SP") {
            FI.mod = "11";
            FI.r_m = "100";
            if (str == "AH") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "CH" || str == "BP") {
            FI.mod = "11";
            FI.r_m = "101";
            if (str == "CH") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "DH" || str == "SI") {
            FI.mod = "11";
            FI.r_m = "110";
            if (str == "DH") FI.w = "0";
            else FI.w = "1";
        }
        else if (str == "BH" || str == "DI") {
            FI.mod = "11";
            FI.r_m = "111";
            if (str == "BH") FI.w = "0";
            else FI.w = "1";
        }
    }
    else if (str == "BX+SI+D16" || str == "BX+DI+D16" || str == "BP+SI+D16" || str == "BP+DI+D16"
             || str == "SI+D16" || str == "DI+D16" || str == "BP+D16" || str == "BX+D16") {
        FI.mod = "10";
        if (str == "BX+SI+D16") FI.r_m = "000";
        else if (str == "BX+DI+D16") FI.r_m = "001";
        else if (str == "BP+SI+D16") FI.r_m = "010";
        else if (str == "BP+DI+D16") FI.r_m = "011";
        else if (str == "SI+D16") FI.r_m = "100";
        else if (str == "DI+D16") FI.r_m = "101";
        else if (str == "BP+D16") FI.r_m = "110";
        else FI.r_m = "111"; // BX+D16
    }
    else if (str == "BX+SI+D8" || str == "BX+DI+D8" || str == "BP+SI+D8" || str == "BP+DI+D8"
             || str == "SI+D8" || str == "DI+D8" || str == "BP+D8" || str == "BX+D8") {
        FI.mod = "01";
        if (str == "BX+SI+D8") FI.r_m = "000";
        else if (str == "BX+DI+D8") FI.r_m = "001";
        else if (str == "BP+SI+D8") FI.r_m = "010";
        else if (str == "BP+DI+D8") FI.r_m = "011";
        else if (str == "SI+D8") FI.r_m = "100";
        else if (str == "DI+D8") FI.r_m = "101";
        else if (str == "BP+D8") FI.r_m = "110";
        else FI.r_m = "111"; // BX+D8
    }
    else {
        FI.mod = "00";
        if (str == "BX+SI") FI.r_m = "000";
        else if (str == "BX+DI") FI.r_m = "001";
        else if (str == "BP+SI") FI.r_m = "010";
        else if (str == "BP+DI") FI.r_m = "011";
        else if (str == "SI") FI.r_m = "100";
        else if (str == "DI") FI.r_m = "101";
        else if (isDirectAddress) FI.r_m = "110";
        else FI.r_m = "111"; // BX
    }

    return FI;
}

typedef struct MEMformat {
    string MEMtype;
    string hex = "";
    bool isDirectAddress = false;
} NewMEMformat;

MEMformat getMEMformat(Line line) { // get from "[" to "]"
    MEMformat node;
    bool in = false;
    string type = "";
    for (int i = 0; line.tokens[i] != "]"; i++) {
        if (in) {
            if (line.coordinates[i][0] == 3 || line.tokens[i] == "+") type += line.tokens[i]; // ex. "BX+DI+"
            else if (line.tokens[i][line.tokens[i].size()-1] == 'H') { // ex. 1H
                if (type == "") node.isDirectAddress = true;
                node.hex = pad_bits(line.tokens[i].substr(0, line.tokens[i].size()-1));
                if (node.hex.size() == 2) type += "D8"; // ex. 12H
                else type += "D16"; // ex. 1234H, 0123H
            }
            else if (isAll0to9orAtoF(line.tokens[i])) {
                if (type == "") node.isDirectAddress = true;
                node.hex = pad_bits(DecToHex(line.tokens[i]));
                if (node.hex.size() == 2) type += "D8"; // ex. 12
                else type += "D16"; // ex. 1234, 0123
            }
        }
        
        if (line.tokens[i] == "[") in = true;
    }

    node.MEMtype = type;
    return node;
}

typedef struct MachineCodeAndDistance {
    string machine_code;
    int distance;
} NewMachineCodeAndDistance;

typedef struct LabelAndAddress {
    string label;
    int addr;
    int line_index;
    int pc = -1;
} NewLabelAndAddress;

int find_longest_length(const vector<std::string> &str) {
    
    auto longest_string = max_element(
        str.begin(),
        str.end(),
        [](const std::string &a, const string &b) {
            return a.length() < b.length();
        }
    );
    
    return longest_string->length();
}

string spaceToFill(int longest_length, int cur_length) {
    if (longest_length == cur_length) return "";
    else {
        int spaceNum = longest_length - cur_length;
        string spaces(spaceNum, ' ');
        return spaces;
    }
}

string get_2s_complement_of_negative_int(int dec) {
    /*
    ex. -98(dec) = FF9E(hex)
    1. 2^4 = 16 <= -(-98)
    2. 2^8 = 256 > -(-98)
    3. 256 - 98 = 158
    4. 158(dec) = 9E(hex)
    5. pad with F
    */
    
    if (dec < 0) dec = -dec;
    int i = 16;
    while (i <= dec) i *= 16;
    string str = pad_addr_bits(DecToHex(to_string(i - dec)), 'F');
    return str;
}

void translate(vector<Line> lines, Tables Ts, OPcodes op_codes) {
    int starting_address = 0;
    vector<int> addr_dec;
    vector<string> machine_code;
    string NEARorFAR = "";
    vector<LabelAndAddress> label_and_addr;
    vector<LabelAndAddress> forward_reference_label_and_addr;
    vector<int> PC;

    for (const auto &line: lines) { // get starting address(if no ORG then use 0000)
        auto it = find(line.tokens.begin(), line.tokens.end(), "ORG");
        if (it != line.tokens.end()) {
            starting_address = stoi(*(it+1), nullptr, 16); // (it)->"ORG". (it+1)-> starting address
            break;
        }
    }

    int distance = 0;
    for (int i = 0; i < lines.size(); i++) {
        /*
        cout << "Line" << i+1 << " "; // debug
        */

        int tokens_size = lines[i].tokens.size();
        for (int j = 0; j < tokens_size; j++) {
            if ((lines[i].coordinates[j][0] == 5)
                && ((lines[i].tokens[j+1] != ":"&& lines[i].tokens[j+1] != "ENDP") // ex. "Mycode PROC NEAR", "LF BYTE 0AH, 0DH, '$'"
                    || (lines[i].tokens[j+1] == ":" && lines[i].tokens[j+2] != "ENDP"))) { // ex. "Mycode: PROC NEAR", "LF: BYTE 0AH, 0DH, '$'"
                LabelAndAddress temp;
                temp.label = lines[i].tokens[j];
                if (lines[i].tokens[j+1] != "CODE" && lines[i].tokens[j+2] != "CODE") temp.addr = addr_dec[addr_dec.size()-1] + distance; // not the first line
                temp.line_index = i;
                label_and_addr.push_back(temp);

                // check forward reference
                if (! forward_reference_label_and_addr.empty()) {
                    for (LabelAndAddress node: forward_reference_label_and_addr) {
                        if (node.label == lines[i].tokens[j]) node.addr = temp.addr;
                    }
                }
            }
            
            if (lines[i].tokens[j] == "CODE" && lines[i].tokens[j+1] == "SEGMENT") { // whole program segment start
                addr_dec.push_back(starting_address);
                machine_code.push_back("");
                break;
            }
            else if ((lines[i].tokens[j] == "PROC") // program (Mycode) start
                     || (lines[i].tokens[j] == "CODE" && lines[i].tokens[j+1] == "ENDS") // whole program segment end
                     || (lines[i].tokens[j] == "END")) { // final ending
                if (lines[i].tokens[j+1] == "NEAR") NEARorFAR = "NEAR"; // RET = "C3"
                else NEARorFAR = "FAR"; // RET = "CB"
                addr_dec.push_back(addr_dec[addr_dec.size()-1]); // use the previous address
                machine_code.push_back("");
                break;
            }
            else if (lines[i].tokens[j] == "ENDP") { // program (ex. Mycode) end
                addr_dec.push_back(addr_dec[addr_dec.size()-1] + distance);
                distance = 0;
                machine_code.push_back("");
                break;
            }
            else if (lines[i].tokens[j] == "BYTE" || lines[i].tokens[j] == "DB" || lines[i].tokens[j] == "WORD" || lines[i].tokens[j] == "DW") { // define
                addr_dec.push_back(addr_dec[addr_dec.size()-1] + distance);
                distance = 0;

                string temp_machine_code = "";
                for (int k = j+1; k < tokens_size; k++) {
                    if (lines[i].coordinates[k][0] == 4) continue; // delimiter, ex. "BYTE 0AH, 0DH, '$'"
                    else if (lines[i].coordinates[k][0] == 6) {
                        if (lines[i].tokens[k][lines[i].tokens[k].size()-1] == 'H') { // Hex
                            string temp_hex = lines[i].tokens[k].substr(0, lines[i].tokens[k].length() - 1);
                            if (! ((lines[i].tokens[j] == "BYTE" || lines[i].tokens[j] == "DB") && temp_hex.size() < 3)) temp_hex = pad_bits(temp_hex); // pad to 4 bits
                            distance += temp_hex.size() / 2; // 2bits -> distance = 1, 4bits -> distance = 2
                            // append to machine code
                            if (temp_machine_code == "") { // if the first one
                                if (temp_hex.size() == 2) temp_machine_code = temp_hex; // temp_hex = "AB" => first = "AB"
                                else temp_machine_code = temp_hex.substr(0, 2) + " " + temp_hex.substr(2); // temp_hex = "ABCD" => first = "AB CD"
                            }
                            else {
                                if (temp_hex.size() == 2) temp_machine_code += (" " + temp_hex); // temp_hex = "AB" => temp_machine_code = prev + " AB"
                                else temp_machine_code += (" " + temp_hex.substr(0, 2) + " " + temp_hex.substr(2)); // temp_hex = "ABCD" => temp_machine_code = prev + " AB CD"
                            }
                        }
                        else { // Dec
                            string temp_hex = pad_bits(DecToHex(lines[i].tokens[k]));
                            distance += temp_hex.size() / 2; // 2bits -> distance = 1, 4bits -> distance = 2
                            // append to machine code
                            if (temp_machine_code == "") { // if the first one
                                if (temp_hex.size() == 2) temp_machine_code = temp_hex; // temp_hex = "AB" => first = "AB"
                                else temp_machine_code = temp_hex.substr(0, 2) + " " + temp_hex.substr(2); // temp_hex = "ABCD" => first = "AB CD"
                            }
                            else {
                                if (temp_hex.size() == 2) temp_machine_code += (" " + temp_hex); // temp_hex = "AB" => temp_machine_code = prev + " AB"
                                else temp_machine_code += (" " + temp_hex.substr(0, 2) + " " + temp_hex.substr(2)); // temp_hex = "ABCD" => temp_machine_code = prev + " AB CD"
                            }
                        }
                    }
                    else { // string
                        string str = lines[i].tokens[k].substr(1, lines[i].tokens[k].size()-2); // ex. get "abc" from "'abc'"
                        for (char ch: str) {
                            string temp_hex = pad_bits(DecToHex(to_string(static_cast<int>(ch))));
                            distance += temp_hex.size() / 2; // 2bits -> distance = 1
                            if (temp_machine_code == "") temp_machine_code = temp_hex;
                            else temp_machine_code += (" " + temp_hex);
                        }
                    }
                }

                if ((lines[i].tokens[j] == "WORD" || lines[i].tokens[j] == "DW") && (distance % 2 != 0)) { // ex. "0A 0D 24"=> "00 0A 0D 24"
                    temp_machine_code = "00 " + temp_machine_code;
                    distance++;
                }

                machine_code.push_back(temp_machine_code);
                break;
            }
            else if (lines[i].coordinates[j][0] == 2) { // other pseudo-instructions
                addr_dec.push_back(addr_dec[addr_dec.size()-1]);
                distance = 0;
                machine_code.push_back("");
                break;
            }
            else if (lines[i].coordinates[j][0] == 1) { // instructions
                addr_dec.push_back(addr_dec[addr_dec.size()-1] + distance);
                distance = 0;

                OPs op = op_codes.findOP(lines[i].tokens[j]);
                string temp_hex = "";

                switch(op.types) {
                    case 1: // AAD, AAM
                        temp_hex = op.opcodes;
                        distance = 1;
                        break;
                    case 2: // AAA ~ XLAT
                        temp_hex = op.opcodes;
                        distance = 1;
                        break;
                    case 3: // PUSH
                        if (lines[i].coordinates[j+1][0] == 3 && 1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 20) { // PUSH + reg
                            if (1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 16) temp_hex = BinaryToHex("01010" + getInfo(lines[i].tokens[j+1], false, true).r_m); // general register
                            else temp_hex = BinaryToHex("000" + getInfo(lines[i].tokens[j+1], false, true).r_m + "110"); // segment register
                            distance = 1;
                        }
                        else { // memory
                            MEMformat mem_format = getMEMformat(lines[i]);
                            FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                            string temp_addr = pad_addr_bits(mem_format.hex, '0'); // pad with 0
                            temp_hex = "FF " + BinaryToHex(format_info.mod + "110" + format_info.r_m) + " " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2);
                            distance = 4;
                        }

                        break;
                    case 4: // POP
                        if (lines[i].coordinates[j+1][0] == 3 && 1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 20) { // PUSH + reg
                            if (1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 16) temp_hex = BinaryToHex("01011" + getInfo(lines[i].tokens[j+1], false, true).r_m); // general register
                            else temp_hex = BinaryToHex("000" + getInfo(lines[i].tokens[j+1], false, true).r_m + "111"); // segment register
                            distance = 1;
                        }
                        else { // memory
                            MEMformat mem_format = getMEMformat(lines[i]);
                            FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                            string temp_addr = pad_addr_bits(mem_format.hex, '0'); // pad with 0
                            temp_hex = "8F " + BinaryToHex(format_info.mod + "000" + format_info.r_m) + " " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2);
                            distance = 4;
                        }
                        break;
                    case 5: // RET, RETF
                        if (lines[i].tokens[j] == "RET" && NEARorFAR == "NEAR") {
                            if (j == tokens_size - 1) { // "RET" => "C3"
                                temp_hex = "C3";
                                distance = 1;                                }
                            else { // "RET" + data => "C2" + data ;ex. "RET 12H"
                                string data = pad_bits(lines[i].tokens[j+1].substr(0, lines[i].tokens[j+1].size())); // 0012
                                temp_hex = "C2 " + data.substr(2) + " " + data.substr(0, 2); // "C2 12 00"
                                distance = 3;
                            }
                        }
                        else { // RET FAR || RETF
                            if (j == tokens_size - 1) { // (PROC FAR, "RET") or "RETF" => "CB"
                                temp_hex = "CB";
                                distance = 1;
                            }
                            else { // (PROC FAR, "RET" + data) or RETF + data => "CA" + data ;ex. "RET 123H"
                                string data = pad_bits(lines[i].tokens[j+1].substr(0, lines[i].tokens[j+1].size())); // 0123
                                if (data.size() == 2) data = "00" + data;
                                temp_hex = "CA " + data.substr(2) + " " + data.substr(0, 2); // "C2 23 01"
                                distance = 3;
                            }
                        }

                        break;
                    case 6: // INT
                        if (j == tokens_size - 1) { // Type 3=> CC
                            temp_hex = "CC";
                            distance = 1;
                        }
                        else if (j < tokens_size - 1) { // Type specified: CD + type(00 ~ FF)H ([j]: INT, [j+1]: type)
                            temp_hex = "CD " + lines[i].tokens[j+1].substr(0, 2); // hex
                            distance = 2;
                        }
                        break;
                    case 7: // IN => "IN" + op1 + "," + op2
                        if (lines[i].tokens[j+3] == "DX") { // only opcode
                            if (lines[i].tokens[j+1] == "AL") temp_hex = "EC"; // "IN AL, DX"
                            else temp_hex = "ED"; // "IN AX, DX" || "IN EAX, DX"
                            distance = 1;
                        }
                        else { // imm8 (ex. FFH)
                            if (lines[i].tokens[j+1] == "AL") temp_hex = "E4 " + lines[i].tokens[j+3].substr(0, 2); // "IN AL, imm8"
                            else temp_hex = "E5 " + lines[i].tokens[j+3].substr(0, 2); // "IN AX, imm8" || "IN EAX, imm8"
                            distance = 2;
                        }
                        
                        break;
                    case 8: // OUT => "OUT" + op1 + "," + op2
                        if (lines[i].tokens[j+1] == "DX") { // only opcode
                            if (lines[i].tokens[j+3] == "AL") temp_hex = "EE"; // "OUT DX, AL"
                            else temp_hex = "EF"; // "OUT DX, AX" || "OUT DX, EAX"
                            distance = 1;
                        }
                        else { // imm8 (ex. FFH)
                            if (lines[i].tokens[j+3] == "AL") temp_hex = "E6 " + lines[i].tokens[j+1].substr(0, 2); // "OUT imm8, AL"
                            else temp_hex = "E7 " + lines[i].tokens[j+1].substr(0, 2); // "OUT imm8, AX" || "OUT imm8, EAX"
                            distance = 2;
                        }
                        
                        break;
                    case 9: // JMP
                        break;
                    case 10: // CALL
                        if (NEARorFAR == "NEAR") {  // Within Segment
                            if (lines[i].coordinates[j+1][0] == 5 || lines[i].coordinates[j+1][0] == 6) { // E8 + disp_lo + disp_hi (direct)
                                distance = 3;
                                if (lines[i].coordinates[j+1][0] == 6) { // ex. CALL 1234H
                                    string temp_addr = lines[i].tokens[j+1];
                                    temp_addr.pop_back(); // pop 'H'
                                    temp_addr = pad_addr_bits(temp_addr, '0');
                                    temp_hex = "E8 " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2);
                                }
                                else { // ex. CALL DispMsg
                                    int target_addr = 0, pc = addr_dec[i] + distance;
                                    bool forward_reference = true;
                                    for (LabelAndAddress node: label_and_addr) {
                                        if (node.label == lines[i].tokens[j+1]) {
                                            target_addr = node.addr;
                                            node.line_index = i;
                                            forward_reference = false;
                                            break;
                                        }
                                    }

                                    if (! forward_reference) { // backward reference, disp is negative
                                        int temp_dec = target_addr - pc; // negative decimal
                                        string disp = get_2s_complement_of_negative_int(temp_dec); // ex. input -6, output "FFFA"
                                        temp_hex = "E8 " + disp.substr(2) + " " + disp.substr(0, 2);
                                    }
                                    else { // forward reference
                                        temp_hex = "E8 ";
                                        LabelAndAddress temp_node;
                                        temp_node.label = lines[i].tokens[j+1]; // undefine label name
                                        temp_node.addr = -1; // unknown label address
                                        temp_node.line_index = i; // which line has forward reference
                                        temp_node.pc = pc; // set pc to calculate disp in pass 2
                                        forward_reference_label_and_addr.push_back(temp_node); // record in the vector
                                    }
                                }
                            }
                            else { // FF + (mod 010 r/m) + disp-lo + disp-hi (indirect)
                                MEMformat mem_format = getMEMformat(lines[i]); // get [mem]'s format
                                FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                                temp_hex = "FF " + BinaryToHex(format_info.mod + "010" + format_info.r_m);
                                distance = 2;
                                if (mem_format.hex != "") {
                                    string disp = pad_bits(mem_format.hex); // pad with 0
                                    distance += disp.size() / 2;
                                    if (disp.size() == 2) temp_hex += (" " + disp); // disp-lo
                                    else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2)); // disp-lo + disp-hi
                                }
                            }
                        }
                        else { // Intersegment
                            if (lines[i].coordinates[j+1][0] == 5 || lines[i].coordinates[j+1][0] == 6) { // 9A + disp_lo/cs-lo + disp_hi/cs-hi (direct)
                                distance = 3;
                                if (lines[i].coordinates[j+1][0] == 6) { // ex. CALL 1234H
                                    string temp_addr = lines[i].tokens[j+1];
                                    temp_addr.pop_back(); // pop 'H'
                                    temp_addr = pad_addr_bits(temp_addr, '0');
                                    temp_hex = "9A " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2);
                                }
                                else { // ex. CALL DispMsg
                                    int target_addr = 0, pc = addr_dec[i] + distance;
                                    bool forward_reference = true;
                                    for (LabelAndAddress node: label_and_addr) {
                                        if (node.label == lines[i].tokens[j+1]) {
                                            target_addr = node.addr;
                                            node.line_index = i;
                                            forward_reference = false;
                                            break;
                                        }
                                    }

                                    if (! forward_reference) { // backward reference, disp is negative
                                        int temp_dec = target_addr - pc; // negative decimal
                                        string disp = get_2s_complement_of_negative_int(temp_dec); // ex. input -6, output "FFFA"
                                        temp_hex = "9A " + disp.substr(2) + " " + disp.substr(0, 2);
                                    }
                                    else { // forward reference
                                        temp_hex = "9A ";
                                        LabelAndAddress temp_node;
                                        temp_node.label = lines[i].tokens[j+1]; // undefine label name
                                        temp_node.addr = -1; // unknown label address
                                        temp_node.line_index = i; // which line has forward reference
                                        temp_node.pc = pc; // set pc to calculate disp in pass 2
                                        forward_reference_label_and_addr.push_back(temp_node); // record in the vector
                                    }
                                }
                            }
                            else { // FF + (mod 011 r/m) + disp-lo + disp-hi (indirect)
                                MEMformat mem_format = getMEMformat(lines[i]); // get [mem]'s format
                                FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                                temp_hex = "FF " + BinaryToHex(format_info.mod + "011" + format_info.r_m);
                                distance = 2;
                                if (mem_format.hex != "") {
                                    string disp = pad_bits(mem_format.hex); // pad with 0
                                    distance += disp.size() / 2;
                                    if (disp.size() == 2) temp_hex += (" " + disp); // disp-lo
                                    else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2)); // disp-lo + disp-hi
                                }
                            }
                        }

                        break;
                    case 11: // JA ~ LOOPZ
                        {
                            distance = 2;
                            int target_addr = 0, pc = addr_dec[i] + distance;
                            bool forward_reference = true;

                            for (LabelAndAddress node: label_and_addr) {
                                if (node.label == lines[i].tokens[j+1]) { // if backward reference
                                    target_addr = node.addr;
                                    node.line_index = i;
                                    forward_reference = false;
                                    break;
                                }
                            }

                            if (! forward_reference) { // backward reference, disp is negative
                                int temp_dec = target_addr - pc; // decimal
                                string disp = get_2s_complement_of_negative_int(temp_dec).substr(2); // ex. input -6, output "FA"
                                temp_hex = op.opcodes + " " + disp;
                            }
                            else { // forward reference
                                temp_hex = op.opcodes + " ";
                                LabelAndAddress temp_node;
                                temp_node.label = lines[i].tokens[j+1]; // undefine label name
                                temp_node.addr = -1; // unknown label address
                                temp_node.line_index = i; // which line has forward reference
                                temp_node.pc = pc; // set pc to calculate disp in pass 2
                                forward_reference_label_and_addr.push_back(temp_node); // record in the vector
                            }
                        }

                        break;
                    case 12: // LDS, LEA, LES
                        {
                            string reg = lines[i].tokens[j+1]; // op1
                            int last_index = lines[i].tokens.size() - 1;
                            if (lines[i].tokens[last_index] == "]") { // instr reg, [mem] ex. LDS AX, [BX]
                                MEMformat mem_format = getMEMformat(lines[i]); // get [mem]'s format (op2)
                                FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                                temp_hex = op.opcodes + " " + BinaryToHex(format_info.mod + getInfo(reg, false, true).r_m + format_info.r_m);
                                distance = 2;
                                if (mem_format.hex != "") { // has address
                                    string temp_addr = pad_addr_bits(mem_format.hex, '0'); // pad with 0
                                    temp_hex += (" " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2));
                                    distance = 4;
                                }
                            }
                            else if (lines[i].coordinates[last_index][0] == 3) { // instr reg, reg
                                FromatInfo op1 = getInfo(reg, false, true), op2 = getInfo(lines[i].tokens[last_index], false, true);
                                temp_hex = op.opcodes + " " + BinaryToHex(op2.mod + op1.r_m + op2.r_m);
                                distance = 2;
                            }
                            else if (lines[i].coordinates[last_index][0] == 5) { // instr reg, label
                                distance = 4;
                                int target_addr = 0, pc = addr_dec[i] + distance;
                                bool forward_reference = true;

                                for (LabelAndAddress node: label_and_addr) {
                                    if (node.label == lines[i].tokens[last_index]) { // if backward reference
                                        target_addr = node.addr;
                                        node.line_index = i;
                                        forward_reference = false;
                                        break;
                                    }
                                }

                                if (! forward_reference) { // backward reference, disp is negative
                                    int temp_dec = target_addr - pc; // negative decimal
                                    string disp = get_2s_complement_of_negative_int(temp_dec); // ex. input -6, output "FFFA"
                                    temp_hex = op.opcodes + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " " + disp.substr(2) + " " + disp.substr(0, 2);
                                }
                                else { // forward reference
                                    temp_hex = op.opcodes + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " ";
                                    LabelAndAddress temp_node;
                                    temp_node.label = lines[i].tokens[last_index]; // undefine label name
                                    temp_node.addr = -1; // unknown label address
                                    temp_node.line_index = i; // which line has forward reference
                                    temp_node.pc = pc; // set pc to calculate disp in pass 2
                                    forward_reference_label_and_addr.push_back(temp_node); // record in the vector
                                }
                            }
                        }

                        break;
                    case 13: // DEC, INC
                        break;
                    case 14: // ESC
                        break;
                    case 15: // DIV ~ NOT
                        {
                            int last_index = lines[i].tokens.size() - 1;
                            string w = "", mod = "", r_m = "";
                            if (lines[i].tokens[last_index] == "]") { // [mem]
                                MEMformat mem_format = getMEMformat(lines[i]); // get [mem]'s format
                                FromatInfo format_info = getInfo(mem_format.MEMtype, mem_format.isDirectAddress, false);
                                w = format_info.w;
                                mod = format_info.mod;
                                r_m = format_info.r_m;
                            }

                            if (op.instructions == "DIV") temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "110" + r_m);
                            else if (op.instructions == "IDIV") temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "111" + r_m);
                            else if (op.instructions == "IMUL") temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "101" + r_m);
                            else if (op.instructions == "MUL") temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "100" + r_m);
                            else if (op.instructions == "NEG") temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "011" + r_m);
                            else temp_hex = BinaryToHex("1111011" + w) + " " + BinaryToHex(mod + "010" + r_m); // NOT
                            distance = 2;
                        }

                        break;
                    case 16: // RCL ~ SHR
                        {
                            int last_index = lines[i].tokens.size() - 1;
                            string v = "", w = getInfo(lines[i].tokens[j+1], false, true).w, mod = "", r_m = getInfo(lines[i].tokens[j+1], false, true).r_m, disp = "";
                            if (lines[i].coordinates[last_index][0] == 3) {
                                mod = "11"; // reg, reg
                                if (lines[i].tokens[last_index] == "CL") v = "1"; // CL
                                else v = "0";
                            }
                            else if (lines[i].coordinates[last_index][0] == 6) {
                                mod = "00"; // direct address(?)
                                if (lines[i].tokens[last_index][lines[i].tokens[last_index].size() - 1] != 'H') { // ex.1234
                                    if (pad_addr_bits(lines[i].tokens[last_index], '0') == "0001") v = "0"; // 1
                                    else v = "1";
                                }
                                else { // immediate ex. 1234H
                                    v = "1";
                                    disp = lines[i].tokens[last_index];
                                    disp.pop_back(); // pop 'H'
                                    disp = pad_addr_bits(disp, '0');
                                }
                            }

                            temp_hex = BinaryToHex("110100" + v + w) + " ";
                            if (op.instructions == "SHL" || op.instructions == "SAL") temp_hex += BinaryToHex(mod + "100" + r_m);
                            else if (op.instructions == "SHR") temp_hex += BinaryToHex(mod + "101" + r_m);
                            else if (op.instructions == "SAR") temp_hex += BinaryToHex(mod + "111" + r_m);
                            else if (op.instructions == "ROL") temp_hex += BinaryToHex(mod + "000" + r_m);
                            else if (op.instructions == "ROR") temp_hex += BinaryToHex(mod + "001" + r_m);
                            else if (op.instructions == "RCL") temp_hex += BinaryToHex(mod + "010" + r_m);
                            else temp_hex += BinaryToHex(mod + "011" + r_m); // RCR
                            distance = 2;

                            if (disp != "") {
                                temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2));
                                distance = 4;
                            }
                        }

                        break;
                    case 17: // XCHG
                        {
                            int last_index = lines[i].tokens.size() - 1;
                            if (lines[i].tokens[j+1] == "AX") { // XCHG AX, reg
                                temp_hex = BinaryToHex("10010" + getInfo(lines[i].tokens[last_index], false, true).r_m);
                                distance = 1;
                            }
                            else if (lines[i].tokens[last_index] == "AX") { // XCHG reg, AX
                                temp_hex = BinaryToHex("10010" + getInfo(lines[i].tokens[j+1], false, true).r_m);
                                distance = 1;
                            }
                            else if (lines[i].coordinates[j+1][0] == 3 && lines[i].tokens[last_index] == "]") { // XCHG reg, [mem]
                                string reg = lines[i].tokens[j+1]; // op1
                                MEMformat op2_mem_format = getMEMformat(lines[i]); // get [mem]'s format (op2)
                                bool hasAddr = false;
                                if (op2_mem_format.hex != "") hasAddr = true;
                                FromatInfo op1 = getInfo(reg, false, true), op2 = getInfo(op2_mem_format.MEMtype, hasAddr, false);
                                temp_hex = BinaryToHex("1000011" + op1.w) + " " + BinaryToHex(op2.mod + op1.r_m + op2.r_m); // ex. XCHG CX, [BX+SI]
                                distance = 2;
                                if (op2_mem_format.hex != "") { // has address
                                    string temp_addr = pad_addr_bits(op2_mem_format.hex, '0'); // pad with 0
                                    temp_hex += (" " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2)); // XCHG CX, [BX+SI+34H]
                                    distance = 4;
                                }
                            }
                            else if (lines[i].tokens[j+1] == "[" && lines[i].coordinates[last_index][0] == 3) { // XCHG [mem], reg
                                string reg = lines[i].tokens[last_index]; // op2
                                MEMformat op1_mem_format = getMEMformat(lines[i]); // get [mem]'s format (op1)
                                bool hasAddr = false;
                                if (op1_mem_format.hex != "") hasAddr = true;
                                FromatInfo op1 = getInfo(op1_mem_format.MEMtype, hasAddr, false), op2 = getInfo(reg, false, true);
                                temp_hex = BinaryToHex("1000011" + op2.w) + " " + BinaryToHex(op1.mod + op2.r_m + op1.r_m); // ex. XCHG [SI], CX
                                distance = 2;
                                if (op1_mem_format.hex != "") { // has address
                                    string temp_addr = pad_addr_bits(op1_mem_format.hex, '0'); // pad with 0
                                    temp_hex += (" " + temp_addr.substr(2) + " " + temp_addr.substr(0, 2)); // XCHG [BP+2H], DH
                                    distance = 4;
                                }
                            }
                        }
                        
                        break;
                    case 18: // TEST
                        break;
                    case 19: // MOV
                        {
                            int last_index = lines[i].tokens.size() - 1;
                            if ((lines[i].coordinates[j+1][0] == 3 && lines[i].coordinates[last_index][0] == 3) // MOV reg, reg
                                || (lines[i].coordinates[j+1][0] == 3 && lines[i].tokens[last_index] == "]") // MOV reg, [mem]
                                || (lines[i].tokens[j+1] == "[" && lines[i].coordinates[last_index][0] == 3)) { // MOV [mem], reg

                                if (lines[i].coordinates[j+1][0] == 3 && lines[i].coordinates[last_index][0] == 3) { // MOV reg, reg
                                    FromatInfo op1 = getInfo(lines[i].tokens[j+1], false, true), op2 = getInfo(lines[i].tokens[last_index], false, true);
                                    string d = "1";
                                    if ((1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 16) // general reg(op2) to general reg(op1)
                                        && (1 <= lines[i].coordinates[last_index][1] && lines[i].coordinates[last_index][1] <= 16)) { // ex. MOV BX, AX->8BD8(d=1) or 89C3(d=0)
                                        temp_hex = BinaryToHex("100010" + d + op1.w) + " " + BinaryToHex(op2.mod + op1.r_m + op2.r_m); // op2 be mem
                                        distance = 2;
                                    }
                                    else if ((17 <= lines[i].coordinates[j+1][1]) // general reg(op2) to segment reg(op1)
                                             && (1 <= lines[i].coordinates[last_index][1] && lines[i].coordinates[last_index][1] <= 16)) { // ex. MOV DS, AX -> 8ED8
                                        temp_hex = BinaryToHex("10001110") + " " + BinaryToHex(op2.mod + "0" + op1.r_m + op2.r_m);
                                        distance = 2;
                                    }
                                    else if ((1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 16) // segment reg(op2) to general reg(op1)
                                             && (17 <= lines[i].coordinates[last_index][1])) { // ex. MOV AX, DS -> 8CD8
                                        temp_hex = BinaryToHex("10001100") + " " + BinaryToHex(op1.mod + "0" + op2.r_m + op1.r_m);
                                        distance = 2;
                                    }
                                }
                                else if (lines[i].coordinates[j+1][0] == 3 && lines[i].tokens[last_index] == "]") { // MOV reg, [mem]
                                    if (1 <= lines[i].coordinates[j+1][1] && lines[i].coordinates[j+1][1] <= 16) { // memory to general register, ex. MOV CX, [BX+SI+12H] -> 8B4812
                                        MEMformat op2_mem_format = getMEMformat(lines[i]);
                                        FromatInfo op1 = getInfo(lines[i].tokens[j+1], false, true), op2 = getInfo(op2_mem_format.MEMtype, op2_mem_format.isDirectAddress, false);
                                        temp_hex = BinaryToHex("1000101" + op1.w) + " " + BinaryToHex(op2.mod + op1.r_m + op2.r_m); // d = 1
                                        distance = 2;
                                        if (op2_mem_format.hex != "") {
                                            string disp = pad_bits(op2_mem_format.hex);
                                            distance += op2_mem_format.hex.size() / 2;
                                            if (op2_mem_format.hex.size() == 2) temp_hex += (" " + disp);
                                            else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2));
                                        }
                                    }
                                    else { // memory to segment register, ex. MOV SS, [SI] -> 8E14
                                        MEMformat op2_mem_format = getMEMformat(lines[i]);
                                        FromatInfo op1 = getInfo(lines[i].tokens[j+1], false, true), op2 = getInfo(op2_mem_format.MEMtype, op2_mem_format.isDirectAddress, false);
                                        temp_hex = BinaryToHex("10001110") + " " + BinaryToHex(op2.mod + "0" + op1.r_m + op2.r_m);
                                        distance = 2;
                                        if (op2_mem_format.hex != "") {
                                            string disp = pad_bits(op2_mem_format.hex);
                                            distance += op2_mem_format.hex.size() / 2;
                                            if (op2_mem_format.hex.size() == 2) temp_hex += (" " + disp);
                                            else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2));
                                        }
                                    }
                                }
                                else { // MOV [mem], reg
                                    if (1 <= lines[i].coordinates[last_index][1] && lines[i].coordinates[last_index][1] <= 16) { // general register to memory, ex. MOV [BX+1234H], AX -> 89873412
                                        MEMformat op1_mem_format = getMEMformat(lines[i]);
                                        FromatInfo op1 = getInfo(op1_mem_format.MEMtype, op1_mem_format.isDirectAddress, false), op2 = getInfo(lines[i].tokens[last_index], false, true);
                                        temp_hex = BinaryToHex("1000100" + op2.w) + " " + BinaryToHex(op1.mod + op2.r_m + op1.r_m); // d = 0
                                        distance = 2;
                                        if (op1_mem_format.hex != "") {
                                            string disp = pad_bits(op1_mem_format.hex);
                                            distance += op1_mem_format.hex.size() / 2;
                                            if (op1_mem_format.hex.size() == 2) temp_hex += (" " + disp);
                                            else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2));
                                        }
                                    }
                                    else { // segment register to memory, ex. MOV [SI+53H], SS -> 8C5453
                                        MEMformat op1_mem_format = getMEMformat(lines[i]);
                                        FromatInfo op1 = getInfo(op1_mem_format.MEMtype, op1_mem_format.isDirectAddress, false), op2 = getInfo(lines[i].tokens[last_index], false, true);
                                        temp_hex = BinaryToHex("10001100") + " " + BinaryToHex(op1.mod + "0" + op2.r_m + op1.r_m);
                                        distance = 2;
                                        if (op1_mem_format.hex != "") {
                                            string disp = pad_bits(op1_mem_format.hex);
                                            distance += op1_mem_format.hex.size() / 2;
                                            if (op1_mem_format.hex.size() == 2) temp_hex += (" " + disp);
                                            else temp_hex += (" " + disp.substr(2) + " " + disp.substr(0, 2));
                                        }
                                    }
                                }
                            }
                            else if (lines[i].coordinates[j+1][0] == 3 && lines[i].coordinates[last_index][0] == 6) { // immediate to register
                                FromatInfo reg = getInfo(lines[i].tokens[j+1], false, true);
                                string temp_data = lines[i].tokens[last_index];
                                if (temp_data[temp_data.size()-1] == 'H') temp_data.pop_back();
                                string data = pad_addr_bits(temp_data, '0');
                                if (reg.w == "0") { // w = 0, ex. MOV AH, 4CH -> B44C
                                    data = data.substr(2);
                                    temp_hex = BinaryToHex("1011" + reg.w + reg.r_m) + " " + data;
                                } // w = 1, ex. MOV CX, 100H -> B90001
                                else temp_hex = BinaryToHex("1011" + reg.w + reg.r_m) + " " + data.substr(2) + " " + data.substr(0, 2);
                                distance = 1 + data.size() / 2;
                            }
                            else {
                                bool label_after_comma = false;
                                string label = "";
                                int size = lines[i].tokens.size();
                                string reg = "";
                                for (int k = 0; k < size; k++) {
                                    if (lines[i].coordinates[k][0] == 5 && k < size - 1) {
                                        label = lines[i].tokens[k];
                                        reg = lines[i].tokens[last_index];
                                        break;
                                    }
                                    else if (lines[i].coordinates[k][0] == 5 && k == size - 1) {
                                        label = lines[i].tokens[k];
                                        label_after_comma = true;
                                        reg = lines[i].tokens[j+1];
                                        break;
                                    }
                                }

                                // directly reverse addr-hi and addr-lo, no need pc

                                int target_addr = 0;
                                bool forward_reference = true;

                                for (LabelAndAddress node: label_and_addr) {
                                    if (node.label == label) { // if backward reference
                                        target_addr = node.addr;
                                        node.line_index = i;
                                        forward_reference = false;
                                        break;
                                    }
                                }

                                if (! forward_reference) { // backward reference
                                    string addr = pad_addr_bits(DecToHex(to_string(target_addr)), '0');
                                    if (label_after_comma) {
                                        if (reg == "AX") { // memory to accumulator, ex. MOV AX, WORD PTR DispMsg
                                            temp_hex = BinaryToHex("1010000" + getInfo(lines[i].tokens[j+1], false, true).w) + " " + addr.substr(2) + " " + addr.substr(0, 2) + " R";
                                            distance = 3;
                                        }
                                        else { // memory to reg(not AX)
                                            temp_hex = BinaryToHex("1000101" + getInfo(reg, false, true).w) + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " " + addr.substr(2) + " " + addr.substr(0, 2) + " R"; // d = 1
                                            distance = 4;
                                        }
                                    }
                                    else {
                                        if (reg == "AX") { // accumulator to memory, ex. MOV WORD PTR Msg, AX
                                            temp_hex = BinaryToHex("1010001" + getInfo(lines[i].tokens[last_index], false, true).w) + " " + addr.substr(2) + " " + addr.substr(0, 2) + " R";
                                            distance = 3;
                                        }
                                        else { // reg(not AX) to memory
                                            temp_hex = BinaryToHex("1000100" + getInfo(reg, false, true).w) + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " " + addr.substr(2) + " " + addr.substr(0, 2); // d = 0
                                            distance = 4;
                                        }
                                    }
                                }
                                else { // forward reference
                                    if (label_after_comma) {
                                        if (reg == "AX") { // memory to accumulator
                                            temp_hex = BinaryToHex("1010000" + getInfo(lines[i].tokens[j+1], false, true).w) + " "; // temp_hex.size() = 3
                                            distance = 3;
                                        }
                                        else { // memory to reg(not AX)
                                            temp_hex = BinaryToHex("1000101" + getInfo(reg, false, true).w) + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " "; // d = 1 // temp_hex.size() = 6
                                            distance = 4;
                                        }
                                    }
                                    else {
                                        if (reg == "AX") { // accumulator to memory
                                            temp_hex = BinaryToHex("1010001" + getInfo(lines[i].tokens[last_index], false, true).w) + " "; // temp_hex.size() = 3
                                            distance = 3;
                                        }
                                        else { // reg(not AX) to memory
                                            temp_hex = BinaryToHex("1000100" + getInfo(reg, false, true).w) + " " + BinaryToHex("00" + getInfo(reg, false, true).r_m + "110") + " "; // d = 0 // temp_hex.size() = 6
                                            distance = 4;
                                        }
                                    }
                                    LabelAndAddress temp_node;
                                    temp_node.label = label; // undefine label name
                                    temp_node.addr = -1; // unknown label address
                                    temp_node.line_index = i; // which line has forward reference
                                    forward_reference_label_and_addr.push_back(temp_node); // record in the vector
                                }
                            }
                        }
                        break;
                    case 20: // ADC ~ XOR
                        break;
                    default:
                        cout << "Line " << i+1 << " -> ERROR: can\'t find instruction \"" << lines[i].tokens[j] << "\"." << endl;
                        break;
                }
                
                machine_code.push_back(temp_hex);
                break;
            }
        }

        PC.push_back(addr_dec[i] + distance);
        /*
        // debug
        cout << "addr: " << addr_dec[i] << ", code: " << machine_code[i] << ", pc: " << PC[i] << endl; // PC[i]
        */
    }

    /*
    // debug
    cout << "===label_and_address===" << endl;
    for (LabelAndAddress node: label_and_addr) {
        cout << node.label << " " << node.addr << " " << node.line_index << endl;
    }
    cout << "=======================" << endl;
    */

    // pass2 ===================================================================================================================================================================================================================================================== //

    // adjust forward references address in forward_reference_label_and_addr
    for (int i = 0; i < forward_reference_label_and_addr.size(); i++) {
        for (int j = 0; j < label_and_addr.size(); j++) {
            if (forward_reference_label_and_addr[i].label == label_and_addr[j].label) {
                forward_reference_label_and_addr[i].addr = label_and_addr[j].addr;
                break;
            }
        }
    }

    /*
    // debug
    cout << "===forward_reference_label_and_addr===" << endl;
    for (LabelAndAddress node: forward_reference_label_and_addr) {
        cout << node.label << " " << node.addr << " " << node.line_index << endl;
    }
    cout << "======================================" << endl;
    */

    // fix the forward reference
    for (int i = 0; i < forward_reference_label_and_addr.size(); i++) {
        int line_index = forward_reference_label_and_addr[i].line_index;
        Line line = lines[line_index];
        for (int j = 0; j < line.tokens.size(); j++) {
            OPs op = op_codes.findOP(line.tokens[j]);
            switch(op.types) {
                case 10: // CALL
                    {
                        int disp = forward_reference_label_and_addr[i].addr - forward_reference_label_and_addr[i].pc; // disp in forward reference must be positive
                        string disp_str = pad_addr_bits(DecToHex(to_string(disp)), '0'); // pad with 0 to 4 bits and get low 2 bits (cuz 1-> 0001->01)
                        machine_code[line_index] += (disp_str.substr(2) + " " + disp_str.substr(0, 2)); // disp-Lo + disp-Hi
                    }
                    break;
                case 11: // JA ~ LOOPZ
                    {
                        int disp = forward_reference_label_and_addr[i].addr - forward_reference_label_and_addr[i].pc; // disp in forward reference must be positive
                        string disp_str = pad_addr_bits(DecToHex(to_string(disp)), '0').substr(2); // pad with 0 to 4 bits and get low 2 bits (cuz 1-> 0001->01)
                        machine_code[line_index] += disp_str;
                    }
                    break;
                case 12: // LDS, LEA, LES
                    {
                        int disp = forward_reference_label_and_addr[i].addr - forward_reference_label_and_addr[i].pc; // disp in forward reference must be positive
                        string disp_str = pad_addr_bits(DecToHex(to_string(disp)), '0'); // pad with 0 to 4 bits and get low 2 bits (cuz 1-> 0001->01)
                        machine_code[line_index] += (disp_str.substr(2) + " " + disp_str.substr(0, 2)); // disp-Lo + disp-Hi
                    }
                    break;
                case 19: // MOV
                    {
                        string addr = pad_addr_bits(DecToHex(to_string(forward_reference_label_and_addr[i].addr)), '0'); // directly get label's address
                        if (machine_code[line_index].size() == 3) machine_code[line_index] += (addr.substr(2) + " " + addr.substr(0, 2) + " R");
                        else if (machine_code[line_index].size() == 6) machine_code[line_index] += (addr.substr(2) + " " + addr.substr(0, 2));
                    }
            }

            break;
        }
    }

    // write result file
    int longest_length = find_longest_length(machine_code);
    string filename = "output.txt";
    ofstream file(filename.c_str());
    for (int i = 0; i < lines.size(); i++) { // not done
        file << pad_addr_bits(DecToHex(to_string(addr_dec[i])), '0') << "\t" << machine_code[i] << spaceToFill(longest_length, machine_code[i].size()) << "\t"; // pad with 0
        if (lines[i].coordinates[0][0] != 5 && lines[i].tokens[0] != "CODE") file << "\t\t";
        for (int j = 0; j < lines[i].tokens.size(); j++) {
            file << lines[i].tokens[j] << " ";
            if (j == 0 && lines[i].coordinates[j][0] == 5) file << "\t";
        }

        file << endl;
    }
    file.close();
}

int main(void) {
    Tables Ts;
    OPcodes OPs;
    string filename = "";
    bool fileExist = true;

    do {
        cout << "Input your file (e.g. \"test_input.txt\"): ";
        cin >> filename;
        ifstream isFileExist(filename.c_str());
        if (isFileExist.good()) break;
        else cout << "File dpexn\'t exist! Try again." << endl;
    } while (true);
    
    vector<Line> lines = Analysis(filename, Ts);
    translate(lines, Ts, OPs);

    return 0;
}