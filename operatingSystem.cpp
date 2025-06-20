#include "operatingSystem.h"

// --- Implementação da Função Auxiliar ---

string stateToString(State state) {
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

// --- Fim da Implementação da Função Auxiliar ---

// --- Implementação da Classe Process ---

Process::Process(int id, int mem, int time) : pid(id), memoryRequired(mem), timeRequired(time), state(State::CREATED) {
    cout << "[PID " << pid << "] Processo criado. Memória: " << memoryRequired << "MB, Tempo: " << timeRequired << "s." << endl;
}

void Process::transitionState(State newState) {
    cout << "[PID " << this->pid << "] Transição de Estado: " << stateToString(this->state) << " -> " << stateToString(newState) << endl;
    this->state = newState;
}

int Process::getPid() const { return this->pid; }

int Process::getMemoryRequired() const { return this->memoryRequired; }

int Process::getTimeRequired() const { return this->timeRequired; }

State Process::getState() const { return this->state; }

void Process::setTimeRequired(int time) { this->timeRequired = time; }

// --- Fim da Implementação da Classe Process ---

// --- Implementação da Classe OS ---

OS::OS(int c, int m, int d) : cores(c), memory(m), diskSpace(d) {
    cout << "Sistema Operacional inicializado com " << cores << " cores, " << memory << "MB de memória e " << diskSpace << "MB de disco." << endl;
    
    schedulerThread = thread(&OS::schedulerLoop, this);

    for (int i = 0; i < this->cores; i++) coreThreads.emplace_back(&OS::coreWorkerLoop, this, i + 1);
}

OS::~OS() {
    stop();

    if(schedulerThread.joinable()) schedulerThread.join();
    for(auto& t : coreThreads) if(t.joinable()) t.join();
}

void OS::stop() {
    if(running){
        running = false;
        cv.notify_all();
    }
}

bool OS::createProcess(int memoryRequired, int timeRequired) {
    lock_guard<mutex> lock(mtx);

    if(this->processIdCounter.empty()){
        cout << "Limite máximo de processos (8) atingido." << endl;
        return false;
    }

    if(memoryRequired <= (this->memory - this->memoryUsed)){
        processList.emplace_back(this->processIdCounter[0], memoryRequired, timeRequired);
        this->memoryUsed += memoryRequired;
        this->processIdCounter.erase(this->processIdCounter.begin());
        return true;
    }else{
        for(auto& process : processList){
            if(process.getState() == State::BLOCKED){
                int spaceAfterSuspend = process.getMemoryRequired() + (this->memory - this->memoryUsed);
                if(spaceAfterSuspend >= memoryRequired){
                    process.transitionState(State::SUSPENDED);
                    this->memoryUsed -= process.getMemoryRequired();
                    this->diskSpaceUsed += process.getMemoryRequired();
                    
                    processList.emplace_back(this->processIdCounter[0], memoryRequired, timeRequired);
                    this->memoryUsed += memoryRequired;
                    this->processIdCounter.erase(this->processIdCounter.begin());
                    return true;
                }
            }
        }
    }

    cout << "Falha ao criar processo: Memória insuficiente." << endl;
    return false;
}

void OS::schedulerLoop() {
    while(running){
        {
            lock_guard<mutex> lock(mtx);
            bool newProcessAdded = false;

            for(auto& process : processList){
                if(process.getState() == State::CREATED){
                    process.transitionState(State::READY);
                    readyQueue.push_back(ref(process));
                    newProcessAdded = true;
                }else if(process.getState() == State::SUSPENDED){
                    if(memoryUsed + process.getMemoryRequired() <= memory){
                        process.transitionState(State::READY);
                        readyQueue.push_back(ref(process));
                        memoryUsed += process.getMemoryRequired();
                        diskSpaceUsed -= process.getMemoryRequired();
                        newProcessAdded = true;
                    }
                }
            }

            if(newProcessAdded) cv.notify_all();
        }
        sleep_for(milliseconds(500));
    }
}

void OS::coreWorkerLoop(int coreId) {
    cout << "[Core " << coreId << "] Iniciado e pronto para trabalhar." << endl;
    while(running){
        Process* processToRun = nullptr;
        {
            unique_lock<mutex> lock(mtx);

            cv.wait(lock, [this] { return !readyQueue.empty() || !running; });

            if(!running && readyQueue.empty()) break;
            
            if(!readyQueue.empty()){
                processToRun = &readyQueue.front().get();
                readyQueue.pop_front();
            }
        }

        if(processToRun){
            executeProcess(*processToRun, coreId);
            {
                lock_guard<mutex> lock(mtx);
                if(processToRun->getTimeRequired() > 0){
                    processToRun->transitionState(State::READY);
                    readyQueue.push_back(*processToRun);
                    cv.notify_one();
                }else{
                    processToRun->transitionState(State::TERMINATED);
                    this->processIdCounter.push_back(processToRun->getPid());
                    memoryUsed -= processToRun->getMemoryRequired();
                    cout << "[Core " << coreId << "][PID " << processToRun->getPid() << "] Processo finalizado e memória liberada." << endl;
                }
            }
        }
    }
    cout << "[Core " << coreId << "] Encerrando." << endl;
}

void OS::executeProcess(Process& process, int coreId) {
    process.transitionState(State::RUNNING);
    int timeToRun = min(process.getTimeRequired(), this->quantum);
    cout << "[Core " << coreId << "][PID " << process.getPid() << "] Executando por " << timeToRun << "s (Tempo Restante: " << process.getTimeRequired() << "s)" << endl;
    
    sleep_for(seconds(timeToRun));

    process.setTimeRequired(process.getTimeRequired() - timeToRun);
}

// --- Fim da Implementação da Classe OS ---