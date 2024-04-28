#include "pin.H"
#include <fstream>
#include <iostream>
#include <atomic>
#include <set>
#include <filesystem>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <string> 

#include <sys/mman.h>
#include <fcntl.h>


using std::atomic;
using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::set;

atomic<UINT64>* totalIterationInstructions;  // Pointer to the shared counter

atomic<UINT64> iterationNumber = 0;
set<string> benchmarkRun; // Keeps track of which benchmarks have been run

void setupSharedMemory() {
    int fd = open("/tmp/shared_counter", O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Ensure the file is large enough to hold an UINT64
    if (ftruncate(fd, sizeof(UINT64)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // Memory map the file
    void* addr = mmap(NULL, sizeof(UINT64), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    close(fd);

    totalIterationInstructions = static_cast<atomic<UINT64>*>(addr);
    if (totalIterationInstructions == nullptr) {
        std::cerr << "Failed to map memory for totalIterationInstructions." << std::endl;
        exit(EXIT_FAILURE);
    }
    totalIterationInstructions->store(0, std::memory_order_relaxed);
}


void incrementInstructionCount(atomic<UINT64> *counter, UINT32 numOfInstructions) {
    counter->fetch_add(numOfInstructions, std::memory_order_relaxed);
}

void setUpNextIteration() {
    totalIterationInstructions->store(0, std::memory_order_relaxed);
}

void finalizeIteration(const std::string& benchmarkName) {
    static std::set<std::string> benchmarkRun;

    if (benchmarkRun.insert(benchmarkName).second) { // element was added - first time we see this benchmark -> reset iteration counter and create new file
        iterationNumber = 0;
    }

    // std::string pid = std::to_string(getpid());
    std::string directory = "results_total_instructions";
    // std::string fileName = directory + "/" + benchmarkName + "_instructionsCount_" + pid + ".csv";
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
    atomicCounters << "," << totalIterationInstructions->load(std::memory_order_relaxed);
    atomicCounters << std::endl;
    atomicCounters.close();

    iterationNumber.fetch_add(1);
}


void handle_client(int client_socket) {
    char buffer[1024] = {0};
    while (true) {
        ssize_t bytesRead = read(client_socket, buffer, 1024);
        if (bytesRead <= 0)
            break;

        string strBuffer(buffer);
        string iterationMode = strBuffer.substr(0, 1);
        string benchmarkName = strBuffer.substr(4);
        benchmarkName.erase(benchmarkName.find_last_not_of("\n") + 1);

        string response;
        if (iterationMode == "A") {
            setUpNextIteration();
            response = "Next iteration has been setup\n";
        } else if (iterationMode == "B") {
            finalizeIteration(benchmarkName);
            response = "Iteration has been finalized\n";
        } else {
            cerr << "Server (Pintool) could not recognize the type state of the current iteration." << endl;
        }

        send(client_socket, response.c_str(), strlen(response.c_str()), 0);
    }

    close(client_socket);
}

VOID initSocket(void *nothing) {
    // cout << "Initializing Socket" << endl;
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    const int PORT = 1234;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation failed" << endl;
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        // perror("Bind failed, please wait a minute and try again");
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

INT32 Usage() {
    cerr << "This tool counts the total number of instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}


// VOID Trace(TRACE trace, VOID *v) {
//     // Get the number of instructions in this basic block
//     UINT64 numInstructions = TRACE_NumIns(trace);

//     // Insert a call at the beginning of the basic block to increment the instruction count
//     TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, totalIterationInstructions, IARG_UINT32, numInstructions, IARG_END);
// }

VOID Trace(TRACE trace, VOID* v) {
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        // Insert a call to totalIterationInstructions before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, totalIterationInstructions, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }
}

int main(int argc, char *argv[]) {
    if (PIN_Init(argc, argv))
        return Usage();

    setupSharedMemory();

    THREADID socketTID = PIN_SpawnInternalThread(initSocket, nullptr, 0, nullptr);
    if (INVALID_THREADID == socketTID) {
        cerr << "Thread creation failed" << endl;
        return 1;
    }

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_StartProgram();

    return 0;
}
