#include "operatingSystem.h"

// Struct Process

Process::Process(int id, int mem, int time) : pid(id), memoryRequired(mem), timeRequired(time) {
    this->state = State::CREATED;
    std::cout << "Process " << pid << " created with " << memoryRequired << "MB memory required." << std::endl;
}

void Process::transitionState(State newState) {
    std::cout << "Process " << pid << " transitioned from: "<< stateToString(this->state) << " to: " << stateToString(newState) << std::endl;
    this->state = newState;
}

// Class OS

OS::OS(int c, int m, int d) : cores(c), memory(m), diskSpace(d){
    std::cout << "Operating System initialized with " << cores << " cores, " << memory << "MB memory, and " << diskSpace << "MB disk space." << std::endl;

    for(int i =0; i < c; i++) this->processor.push_back(Processor());

}

bool OS::createProcess(int memoryRequired, int timeRequired){
    if(this->processIdCounter < 8){
        if (memoryRequired <= (this->memory - this->memoryUsed)){
            Process newProcess(this->processIdCounter, memoryRequired, timeRequired);
            this->memoryUsed += memoryRequired;
            this->processIdCounter++;

            processList.push_back(newProcess);
            return true;
        }else{
            for(auto& process : processList){
                if (process.state == State::BLOCKED){
                    int sumSpace = process.memoryRequired + (this->memory - this->memoryUsed);

                    if(sumSpace >= memoryRequired) {
                        process.state = State::SUSPENDED;
                        this->memoryUsed -= process.memoryRequired;
                        this->diskSpaceUsed += process.memoryRequired;
                        
                        Process newProcess(this->processIdCounter, memoryRequired, timeRequired);
                        this->memoryUsed += memoryRequired;
                        this->processIdCounter++;

                        processList.push_back(newProcess);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void Processor::execute(Process process, int quantum) {
    std::cout << "Executing process " << process.pid << "..." << std::endl;

    process.transitionState(State::RUNNING);

    int timeToRun = min(process.timeRequired, quantum);

    sleep_for(seconds(timeToRun));
    process.timeRequired -= timeToRun;

    if(process.timeRequired <=0){
        process.transitionState(State::TERMINATED);
        this->memoryUsed -= process.memoryRequired;
        this->processList.erase(remove_if(this->processList.begin(), this->processList.end(), 
            [&process](const Process& p) { return p.pid == process.pid; }), this->processList.end());
    } else if (process.timeRequired > 0) {
        process.transitionState(State::READY);
    }

    std::this_thread::sleep_for(std::chrono::seconds(currentProcess.timeRequired));
    std::cout << "Process " << currentProcess.pid << " completed." << std::endl;
}

void OS::runProcessor(){
    while(this->runnig){
        int i = 0;
        for(auto& process : this->processList){
            if(process.state == State::READY){

                
                processor[i].execute(process, this->quantum);
                i++;

                if(i > this->cores -1){
                    i = 0;
                }

                if(process.timeRequired <=0){
                    process.transitionState(State::TERMINATED);
                    this->memoryUsed -= process.memoryRequired;
                    this->processList.erase(remove_if(this->processList.begin(), this->processList.end(), 
                        [&process](const Process& p) { return p.pid == process.pid; }), this->processList.end());
                } else if (process.timeRequired > 0) {
                    process.transitionState(State::READY);
                }
            } else if (process.state == State::BLOCKED) {
                // Simulate some processing time
                std::this_thread::sleep_for(std::chrono::seconds(1));
                process.transitionState(State::READY);
            }
        }
    }
    cout << "Processor is stopping." << endl;
}