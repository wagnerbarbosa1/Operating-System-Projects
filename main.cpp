#include "operatingSystem.h"
#include <vector>

int main() {
    OS os(2, 32, 128);

    int mem = 0, time = 0;

    while(true) {
        cout << "Digite a memória necessária (em MB) e o tempo necessário (em segundos) para o processo (ou -1 -1 para sair): ";
        cin >> mem >> time;

        if(mem == -1 || time == -1) break;

        if(!os.createProcess(mem, time)) cout << "Não foi possível criar o processo. Verifique se há memória e espaço em disco disponíveis." << endl;
    }

    os.stop();

    cout << "\n--- Simulação finalizada. ---\n" << endl;

    return 0;
}