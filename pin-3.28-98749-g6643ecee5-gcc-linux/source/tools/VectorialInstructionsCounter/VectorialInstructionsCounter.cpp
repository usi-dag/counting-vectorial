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
#include <unordered_map>


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
using std::unordered_map;
using std::pair;


unordered_map<string, atomic<UINT64>*> instructionsCounters;

atomic<UINT64> iterationNumber = 0;
set<string> benchmarkRun; // Keeps track of which benchmarks have been run

atomic<UINT64>* setupSharedMemory(string instruction) {

    // Remove any / from the instruction name (else it won't work)    
    std::string cleanedInstruction;
    for (char c : instruction) {
        if (c == '/') {
            cleanedInstruction += '-';
        } else {
            cleanedInstruction += c;
        }
    }
    std::string filename = "/tmp/shared_counter_" + cleanedInstruction;

    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
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

    atomic<UINT64>* counter_pointer = static_cast<atomic<UINT64>*>(addr);
    if (counter_pointer == nullptr) {
        std::cerr << "Failed to map memory for instruction " << instruction << std::endl;
        exit(EXIT_FAILURE);
    }
    counter_pointer->store(0, std::memory_order_relaxed);

    return counter_pointer;
}

void initMap() {
    std::ifstream inputFile("x86_vectorial_instructions.txt");
    string line;

    if (inputFile.is_open()) {
        while (getline(inputFile, line)) {
            instructionsCounters[line] = setupSharedMemory(line);
        }
        inputFile.close();
    } else {
        cerr << "Unable to open x86_vectorial_instructions.txt" << endl;
        exit(1); // Exit with an error code
    }
}

void incrementInstructionCount(atomic<UINT64> *counter, UINT32 numOfInstructions) {
    counter->fetch_add(numOfInstructions, std::memory_order_relaxed);
}

void setUpNextIteration() {
    // Set all the atomic counters to 0
    for (auto itr = instructionsCounters.begin(); itr != instructionsCounters.end(); itr++)
        itr->second->store(0, std::memory_order_relaxed);
}


void finalizeIteration(string benchmarkName) {

    if (benchmarkRun.insert(benchmarkName).second) { // element was added - first time we see this benchmark -> reset iteration counter and create new file
        iterationNumber = 0;
    }

    // std::string pid = std::to_string(getpid());
    std::string directory = "results_vectorial_instructions";
    // std::string fileName = directory + "/" + benchmarkName + "_instructionsCount_" + pid + ".csv";
    std::string fileName = directory + "/" + benchmarkName + "_instructionsCount.csv";

    int itr = iterationNumber.load();    // Initialize the output file
    if (itr == 0) {
        // Check if the directory exists and create it if it doesn't
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directory(directory);
        }
        ofstream atomicCounters(fileName);
        if (!atomicCounters) {
            cerr << "Unable to open file for writing!" << endl;
            return;
        }
        // Create header
        atomicCounters << "Iteration";
        for (auto itr = instructionsCounters.begin(); itr != instructionsCounters.end(); itr++)
            atomicCounters << "," << itr->first;
            
        atomicCounters << endl;
    }
    
    ofstream atomicCounters(fileName, ios::app);
    if (!atomicCounters) {
        cerr << "Unable to open file for writing!" << endl;
        return;
    }
    atomicCounters << itr;  // Iteration number
    for (auto itr = instructionsCounters.begin(); itr != instructionsCounters.end(); itr++)
        atomicCounters << "," << itr->second->load(std::memory_order_relaxed);

    atomicCounters << endl;
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
    cerr << "This tool counts the number of cmpxchg instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

VOID Instruction(INS ins, VOID *v) {
 
    auto insPair = instructionsCounters.find(INS_Mnemonic(ins));    // returns iterator to this element, or end if not found
 
    // Incremenent the counter for this specific instruction (vectorial instruction)
    if (insPair != instructionsCounters.end()) {
        
        atomic<UINT64> *counter = (insPair->second);
 
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, counter, IARG_END);
 
    }
 
}

// VOID Trace(TRACE trace, VOID *v) {
//     // Get the number of instructions in this basic block
//     UINT64 numInstructions = TRACE_NumIns(trace);

//     // Insert a call at the beginning of the basic block to increment the instruction count
//     TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, totalIterationInstructions, IARG_UINT32, numInstructions, IARG_END);
// }

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID* v) {

    // Mapping: <Mnemonic, (First occurence pointer, counter)>
    unordered_map<string, UINT64> bbl_counters;

    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        // Forward pass over all instructions in bbl
        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
            string mnemonic = INS_Mnemonic(ins);
            auto insPair = instructionsCounters.find(mnemonic);    // returns iterator to this element, or end if not found

            if (insPair != instructionsCounters.end()) {
                auto pair = bbl_counters.find(mnemonic);
                if(pair == bbl_counters.end()) {
                    bbl_counters[mnemonic] = 1;
                } else {
                    pair->second++;
                }
            }
        }
        
        for (auto itr = bbl_counters.begin(); itr != bbl_counters.end(); itr++) {
            string mnemonic = itr->first;
            auto insPair = instructionsCounters.find(mnemonic);
            atomic<UINT64>* counter_pointer = insPair->second;
            UINT64 increment = itr->second;
            
            BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)incrementInstructionCount, IARG_PTR, counter_pointer, IARG_UINT32, increment, IARG_END);

            itr->second = 0;
        }
    }


}

int main(int argc, char *argv[]) {
    if (PIN_Init(argc, argv))
        return Usage();

    THREADID socketTID = PIN_SpawnInternalThread(initSocket, nullptr, 0, nullptr);
    if (INVALID_THREADID == socketTID) {
        cerr << "Thread creation failed" << endl;
        return 1;
    }

    initMap();

    // Use TRACE_AddInstrumentFunction to instrument basic blocks
    // INS_AddInstrumentFunction(Instruction, 0);
    TRACE_AddInstrumentFunction(Trace, 0);


    PIN_StartProgram();

    return 0;
}
