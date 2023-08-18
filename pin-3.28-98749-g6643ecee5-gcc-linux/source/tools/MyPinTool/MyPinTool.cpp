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

using std::unordered_map;
using std::pair;

// Socket
bool useSocket; // Boolean set through command line argument -s 1 or -s 0
KNOB<BOOL> KnobUseSocket(KNOB_MODE_WRITEONCE, "pintool",
    "s", "false", "Enable or disable the socket functionality");


const int instructionsIndexSize = 19; // (there are 19 of these and this number won't change in the future, can't do instructionsIndex.size() because it's not const)

// mnemonics for instructions where the LOCK prefix may be used
// and for XCHG, where the LOCK prefix is implicit
const unordered_map<string, int> instructionsIndex = {
    {"ADC_LOCK", 0},
    {"ADD_LOCK", 1},
    {"AND_LOCK", 2},
    {"BTC_LOCK", 3},
    {"BTR_LOCK", 4},
    {"BTS_LOCK", 5},
    {"CMPXCHG_LOCK", 6},
    {"CMPXCH8B_LOCK", 7},
    {"CMPXCHG16B_LOCK", 8},
    {"DEC_LOCK", 9},
    {"INC_LOCK", 10},
    {"NEG_LOCK", 11},
    {"NOT_LOCK", 12},
    {"OR_LOCK", 13},
    {"SBB_LOCK", 14},
    {"SUB_LOCK", 15},
    {"XOR_LOCK", 16},
    {"XADD_LOCK", 17},
    {"XCHG", 18}
};

// Keeps track of each instruction counter
// Check instructionsIndex for which instruction is at which index
atomic<UINT64> instructionsCounters[instructionsIndexSize];

void incrementInstructionCount(atomic<UINT64> *counter) {
    counter->fetch_add(1);
}


// Resets map counters
// Called once "afterOperationSetUp" is called by the Java Plugin
void setUpNextIteration() {
    // Set all the atomic counters to 0
    for (int i = 0; i < instructionsIndexSize; i++)
        instructionsCounters[i] = 0;
}

// Called once "beforeOperationTearDown" is called by the Java Plugin
void finalizeIteration(string iterationNumber, string benchmark) {

    string fileName = "results/" + benchmark + "_instructionsCount.csv";

    // Initialize the output file
    if (iterationNumber == "0") {
        ofstream atomicCounters(fileName);
        if (!atomicCounters) {
            cerr << "Unable to open file for writing!" << endl;
            return;
        }
        // Create header
        atomicCounters << "Iteration,ADC_LOCK,ADD_LOCK,AND_LOCK,BTC_LOCK,BTR_LOCK,BTS_LOCK,CMPXCHG_LOCK,CMPXCH8B_LOCK,CMPXCHG16B_LOCK,DEC_LOCK,INC_LOCK,NEG_LOCK,NOT_LOCK,OR_LOCK,SBB_LOCK,SUB_LOCK,XOR_LOCK,XADD_LOCK,XCHG" << endl;
    }
    
    ofstream atomicCounters(fileName, ios::app);
    if (!atomicCounters) {
        cerr << "Unable to open file for writing!" << endl;
        return;
    }
    atomicCounters << iterationNumber;
    for (const atomic<UINT64>& count : instructionsCounters) 
        atomicCounters << "," << count.load();

    atomicCounters << endl;
    atomicCounters.close();
}

void handle_client(int client_socket) {

    char buffer[1024] = {0};

    while (true) { // Handle multiple requests on the same connection
        // Read message from client and send a reply
        ssize_t bytesRead = read(client_socket, buffer, 1024);
        if (bytesRead <= 0)
            break; // Break the loop if connection is closed

        // Parse the buffer into:
        // - Iteration mode (starting or ending)
        // - Iteration number
        // - Benchmark being run (used to name the output file)
        // The buffer looks something like this A10 ~ fj-kmeans
        string strBuffer(buffer);
        string iterationMode = strBuffer.substr(0, 1);
        string strIteration = strBuffer.substr(1);
        int strIterationEndIdx = strIteration.find_first_not_of("0123456789");
        int strBenchmarkStartIdx = strIteration.find("~");
        string benchmark = strIteration.substr(strBenchmarkStartIdx + 2, strIteration.find_first_of("\n") - (strBenchmarkStartIdx + 2));
        strIteration = strIteration.substr(0, strIterationEndIdx);

        // Call the corresponding method depending if we are at the start of end of an iteration
        string response;
        if (iterationMode == "A") {
            setUpNextIteration();
            response = "Next iteration has been setup\n";
        } else if (iterationMode == "B") {
            finalizeIteration(strIteration, benchmark);
            response = "Iteration has been finalized\n";
        } else {
            cerr << "Server (Pintool) could not recognize the type state of the current iteration." << endl
                      << "Should be either A for afterOperationSetUp or B for beforeOperationTearDown." << endl
                      << "But was " << strBuffer.substr(0, 1) << endl;
        }

        send(client_socket, response.c_str(), strlen(response.c_str()), 0);
    }

    close(client_socket);
}

// Function signature required to be like this to use Pin's Thread API function
// Initializes the socket and starts listening for incoming messages
VOID initSocket(void *nothing) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    const int PORT = 1234;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation failed" << endl;
        return;
    }

    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed, please wait a minute and try again");
        return;
    }

    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed" << endl;
        return;
    }

    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        cerr << "Accept failed" << endl;
        return;
    }

    handle_client(client_socket);

    close(server_fd);
}

// Called every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v) {

    auto insPair = instructionsIndex.find(INS_Mnemonic(ins));    // returns iterator to this element, or end if not found

    if (insPair != instructionsIndex.end()) {
        
        atomic<UINT64> *counter = &instructionsCounters[insPair->second];

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, counter, IARG_END);

    }
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

    useSocket = KnobUseSocket.Value();

    if(useSocket) {
        // Create thread, which starts and calls initSocket
        THREADID socketTID = PIN_SpawnInternalThread(initSocket, nullptr, 0, nullptr);

        if (INVALID_THREADID == socketTID) {
            cerr << "Thread creation failed" << endl;
            return 1; // return 1 - error
        }
    } else {
        cerr << "Not using Socket, to enable add flag -s 1 after your pintool" << endl;
        setUpNextIteration();
    }

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Starts the targeted program that we want to instrument
    // The function never returns, so anything below it will not execute.
    PIN_StartProgram();

    return 0;
}