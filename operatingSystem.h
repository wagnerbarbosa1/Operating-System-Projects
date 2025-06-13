#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;
using namespace this_thread;

string stateToString(enum class State state) {
    switch (state) {
        case State::CREATED: return "CREATED";
        case State::READY: return "READY";
        case State::RUNNING: return "RUNNING";
        case State::BLOCKED: return "BLOCKED";
        case State::SUSPENDED: return "SUSPENDED";
        case State::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

enum class State {
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    TERMINATED
};

struct Process{
    int pid;
    int memoryRequired;
    int timeRequired; // Not used in this example, but can be extended
    State state;

    Process(int id, int mem, int time);

    void transitionState(State newState);
};

class OS{
    private:
        int cores;
        int memory;
        int memoryUsed = 0;
        int diskSpace;
        int diskSpaceUsed = 0;
        int processIdCounter = 1;
        int quantum = 2;

        bool runnig = true;

        vector<Process> processList;

        vector<Processor> processor;

        mutex mtx;
        thread threadProcessor;

    public:
        OS(int c, int m, int d);

        bool createProcess(int memoryRequired, int timeRequired);

        void runProcessor();
};

class Processor{
    private:
        Process currentProcess;

    public:
        Processor();

        void execute(Process process, int quantum);
};