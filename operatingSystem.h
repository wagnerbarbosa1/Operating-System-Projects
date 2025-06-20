#ifndef OPERATINGSYSTEM_H
#define OPERATINGSYSTEM_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <deque>
#include <functional>
#include <atomic>

using namespace std;
using namespace chrono;
using namespace this_thread;

// Enum para os estados do processo
enum class State {
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    TERMINATED
};

// Função auxiliar para converter State para String
string stateToString(State state);

// --- Classe Process ---
class Process {
    private:
        int pid;
        int memoryRequired;
        int timeRequired;
        State state;

    public:
        Process(int id, int mem, int time);

        void transitionState(State newState);

        int getPid() const;

        int getMemoryRequired() const;

        int getTimeRequired() const;

        State getState() const;

        void setTimeRequired(int time);
};

// --- Classe OS ---
class OS {
    private:
        int cores;
        int memory;
        int memoryUsed = 0;
        int diskSpace;
        int diskSpaceUsed = 0;
        vector<int> processIdCounter = {1, 2, 3, 4, 5, 6, 7, 8};
        const int quantum = 2;

        atomic<bool> running{true};

        list<Process> processList;
        deque<reference_wrapper<Process>> readyQueue;

        mutex mtx;
        condition_variable cv;
        thread schedulerThread;
        vector<thread> coreThreads;

        void schedulerLoop();

        void coreWorkerLoop(int coreId);

        void executeProcess(Process& process, int coreId);

    public:
        OS(int c, int m, int d);

        ~OS();

        bool createProcess(int memoryRequired, int timeRequired);

        void stop();
};

#endif // OPERATINGSYSTEM_H