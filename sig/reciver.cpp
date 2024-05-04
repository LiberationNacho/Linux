#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void signalHandler_intr(int signum) {
    cout << "Received signal: " << signum << endl;
	cout << "intr" << endl;
	exit(signum);
}

int main() {
    signal(SIGINT, signalHandler_intr); // 시그널 핸들러 등록

    cout << "Process ID: " << getpid() << endl;

    int signal;
    cout << "Waiting for signal... (Press Ctrl+C to send SIGINT)" << endl;
    while (true) {
        // 무한 루프
    }

    return 0;
}
