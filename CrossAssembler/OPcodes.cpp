#include <iostream>
#include <string>
#include <vector>
using namespace std;

typedef struct OPs {
    string instructions; // ex. "AAD"
    string opcodes; // ex. "D5"
    int types; // ex. 1
} NewOPs;

class OPcodes {
    vector<OPs> ops;
    public:
        OPcodes() {
            ops.push_back({"AAD", "D5", 1});
            ops.push_back({"AAM", "D4", 1});
            ops.push_back({"AAA", "37", 2});
            ops.push_back({"AAS", "3F", 2});
            ops.push_back({"CBW", "98", 2});
            ops.push_back({"CLC", "F8", 2});
            ops.push_back({"CLD", "FC", 2});
            ops.push_back({"CLI", "FA", 2});
            ops.push_back({"CMC", "F5", 2});
            ops.push_back({"CMPSB", "A6", 2});
            ops.push_back({"CMPSW", "A7", 2});
            ops.push_back({"CWD", "99", 2});
            ops.push_back({"DAA", "27", 2});
            ops.push_back({"DAS", "2F", 2});
            ops.push_back({"FWAIT", "9B", 2});
            ops.push_back({"HLT", "F4", 2});
            ops.push_back({"INTO", "CE", 2});
            ops.push_back({"IRET", "CF", 2});
            ops.push_back({"LAHF", "9F", 2});
            ops.push_back({"LOCK", "F0", 2});
            ops.push_back({"LODSB", "AC", 2});
            ops.push_back({"LODSW", "AD", 2});
            ops.push_back({"MOVSB", "A4", 2});
            ops.push_back({"MOVSW", "A5", 2});
            ops.push_back({"NOP", "90", 2});
            ops.push_back({"POPF", "9D", 2});
            ops.push_back({"PUSHF", "9C", 2});
            ops.push_back({"REP", "F3", 2});
            ops.push_back({"REPE", "F3", 2});
            ops.push_back({"REPNE", "F2", 2});
            ops.push_back({"REPNZ", "F2", 2});
            ops.push_back({"REPZ", "F3", 2});
            ops.push_back({"SAHF", "9E", 2});
            ops.push_back({"SCASB", "AE", 2});
            ops.push_back({"SCASW", "AF", 2});
            ops.push_back({"STC", "F9", 2});
            ops.push_back({"STD", "FD", 2});
            ops.push_back({"STI", "FB", 2});
            ops.push_back({"STOSB", "AA", 2});
            ops.push_back({"STOSW", "AB", 2});
            ops.push_back({"WAIT", "9B", 2});
            ops.push_back({"XLAT", "D7", 2});
            ops.push_back({"PUSH", "30", 3});
            ops.push_back({"POP", "00", 4});
            ops.push_back({"RET", "C3", 5});
            ops.push_back({"RETF", "CB", 5});
            ops.push_back({"INT", "CC", 6});
            ops.push_back({"IN", "EC", 7});
            ops.push_back({"OUT", "EE", 8});
            ops.push_back({"JMP", "20", 9});
            ops.push_back({"CALL", "E8", 10});
            ops.push_back({"JA", "77", 11});
            ops.push_back({"JAE", "73", 11});
            ops.push_back({"JB", "72", 11});
            ops.push_back({"JBE", "76", 11});
            ops.push_back({"JC", "72", 11});
            ops.push_back({"JCXZ", "E3", 11});
            ops.push_back({"JE", "74", 11});
            ops.push_back({"JG", "7F", 11});
            ops.push_back({"JGE", "7D", 11});
            ops.push_back({"JL", "7C", 11});
            ops.push_back({"JLE", "7E", 11});
            ops.push_back({"JNA", "76", 11});
            ops.push_back({"JNAE", "72", 11});
            ops.push_back({"JNB", "73", 11});
            ops.push_back({"JNBE", "77", 11});
            ops.push_back({"JNC", "73", 11});
            ops.push_back({"JNE", "75", 11});
            ops.push_back({"JNG", "7E", 11});
            ops.push_back({"JNGE", "7C", 11});
            ops.push_back({"JNL", "7D", 11});
            ops.push_back({"JNLE", "7F", 11});
            ops.push_back({"JNO", "71", 11});
            ops.push_back({"JNP", "7B", 11});
            ops.push_back({"JNS", "79", 11});
            ops.push_back({"JNZ", "75", 11});
            ops.push_back({"JO", "70", 11});
            ops.push_back({"JP", "7A", 11});
            ops.push_back({"JPE", "7A", 11});
            ops.push_back({"JPO", "7B", 11});
            ops.push_back({"JS", "78", 11});
            ops.push_back({"JZ", "74", 11});
            ops.push_back({"LOOP", "E2", 11});
            ops.push_back({"LOOPE", "E1", 11});
            ops.push_back({"LOOPNE", "E0", 11});
            ops.push_back({"LOOPNZ", "E0", 11});
            ops.push_back({"LOOPZ", "E1", 11});
            ops.push_back({"LDS", "C5", 12});
            ops.push_back({"LEA", "8D", 12});
            ops.push_back({"LES", "C4", 12});
            ops.push_back({"DEC", "08", 13});
            ops.push_back({"INC", "00", 13});
            ops.push_back({"ESC", "D8", 14});
            ops.push_back({"DIV", "30", 15});
            ops.push_back({"IDIV", "38", 15});
            ops.push_back({"IMUL", "28", 15});
            ops.push_back({"MUL", "20", 15});
            ops.push_back({"NEG", "18", 15});
            ops.push_back({"NOT", "10", 15});
            ops.push_back({"RCL", "10", 16});
            ops.push_back({"RCR", "18", 16});
            ops.push_back({"ROL", "00", 16});
            ops.push_back({"ROR", "08", 16});
            ops.push_back({"SAR", "38", 16});
            ops.push_back({"SHL", "20", 16});
            ops.push_back({"SHR", "28", 16});
            ops.push_back({"XCHG", "86", 17});
            ops.push_back({"TEST", "F6", 18});
            ops.push_back({"MOV", "C6", 19});
            ops.push_back({"ADC", "10", 20});
            ops.push_back({"ADD", "00", 20});
            ops.push_back({"AND", "20", 20});
            ops.push_back({"CMP", "38", 20});
            ops.push_back({"OR", "08", 20});
            ops.push_back({"SBB", "18", 20});
            ops.push_back({"SUB", "28", 20});
            ops.push_back({"XOR", "30", 20});
            // ops.push_back({"DW", "FF", 21});
            // ops.push_back({"DB", "FF", 22});
            // ops.push_back({"ORG", "FF", 23});
        }
    
    OPs findOP(string instruction) {
        OPs temp;
        for (int i = 0; i < ops.size(); i++) {
            if (instruction == ops[i].instructions) {
                temp = ops[i];
                break;
            }
        }

        return temp;
    }
};