#include "pin.H"
#include <fstream>
#include <iostream>

#include <atomic>
#include <unordered_map>

// Socket
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using std::atomic;
using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

using std::pair;
using std::unordered_map;

string fileName = "instructionTrace.txt";
ofstream traceFile(fileName);

void print(string mnemonic) {
    ofstream traceFile(fileName, ios::app);
    traceFile << mnemonic << endl;
}

VOID Fini(INT32 code, VOID *v) {
    traceFile.close();
}

unordered_map<string, int> M;

// Called every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v) {

    auto p = M.find(INS_Mnemonic(ins));
    if (p == M.end()) {
        M[INS_Mnemonic(ins)] = 1;
        print(INS_Mnemonic(ins));
    }
    // INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)print, IARG_PTR, mnemonic, IARG_END);
}

INT32 Usage() {
    cerr << "This tool counts the number of cmpxchg instructions executed" << endl;
    cerr << endl
         << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char *argv[]) {

    // Initialize Pin
    if (PIN_Init(argc, argv)) // Returns true if command line arguments are wrong ==> Calls Usage() and describes what this tools is
        return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Starts the targeted program that we want to instrument
    // The function never returns, so anything below it will not execute.
    PIN_StartProgram();

    return 0;
}