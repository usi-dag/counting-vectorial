#include "pin.H"
#include <fstream>
#include <iostream>

#include <atomic>
#include <set>
#include <filesystem>

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
using std::set;

atomic<UINT64> totalIterationInstructions = 0;  // Counter for the total number of (any) instruction

atomic<UINT64> iterationNumber = 0;

set<string> benchmarkRun; // Keeps track of which benchmarks have been run

void incrementInstructionCount(atomic<UINT64> *counter) {
    counter->fetch_add(1);
}

// Resets map counters
// Called once "afterOperationSetUp" is called by the Java Plugin
void setUpNextIteration() {
    totalIterationInstructions = 0;
}

// Called once "beforeOperationTearDown" is called by the Java Plugin
void finalizeIteration(string benchmarkName) {
    static std::set<std::string> benchmarkRun;

    if (benchmarkRun.insert(benchmarkName).second) { // element was added - first time we see this benchmark -> reset iteration counter and create new file
        iterationNumber = 0;
    }

    std::string directory = "results_total_instructions";
    std::string fileName = directory + "/" + benchmarkName + "_instructionsCount.csv";

    int itr = iterationNumber.load();
    // Initialize the output file
    if (itr == 0) {
        // Check if the directory exists and create it if it doesn't
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directory(directory);
        }
        std::ofstream atomicCounters(fileName);
        if (!atomicCounters) {
            std::cerr << "Unable to open file for writing!" << std::endl;
            return;
        }
        // Create header
        atomicCounters << "Iteration,";
        atomicCounters << "Total Number of (any) instructions";
        atomicCounters << std::endl;
    }
    
    std::ofstream atomicCounters(fileName, std::ios::app);
    if (!atomicCounters) {
        std::cerr << "Unable to open file for writing!" << std::endl;
        return;
    }
    atomicCounters << itr;  // Iteration number
    atomicCounters << "," << totalIterationInstructions.load();
    atomicCounters << std::endl;
    atomicCounters.close();

    iterationNumber.fetch_add(1);
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
        string benchmarkName = strBuffer.substr(4);
        benchmarkName.erase(benchmarkName.find_last_not_of("\n") + 1); // Remove trailing \n

        // Call the corresponding method depending if we are at the start of end of an iteration
        string response;
        if (iterationMode == "A") {
            setUpNextIteration();
            response = "Next iteration has been setup\n";
        } else if (iterationMode == "B") {
            finalizeIteration(benchmarkName);
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
    cout << "Initializing Socket" << endl;
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

    while(true) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            cerr << "Accept failed" << endl;
            return;
        }

        handle_client(client_socket);

    }

    close(server_fd);
}

// Called every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v) {
    // Increment the total counter for this iteration
    // atomic<UINT64> *total = &totalIterationInstructions;
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, &totalIterationInstructions, IARG_END);
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

    // Create thread, which starts and calls initSocket
    THREADID socketTID = PIN_SpawnInternalThread(initSocket, nullptr, 0, nullptr);

    if (INVALID_THREADID == socketTID) {
        cerr << "Thread creation failed" << endl;
        return 1; // return 1 - error
    }

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Starts the targeted program that we want to instrument
    // The function never returns, so anything below it will not execute.
    PIN_StartProgram();

    return 0;
}