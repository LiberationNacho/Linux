#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <unistd.h>

using namespace std;

// SIGINT
void signalHandler_intr(int signum) {
    cout << "Received signal: " << signum << endl;
	cout << "intr" << endl;
	exit(signum);
}

// SIGTERM 
void signalHandler_term(int signum) {
    cout << "Received signal: " << signum << endl;
	cout << "term" << endl;
	exit(signum);
}

// SIGUSR
void signalHandler_usr(int signum) {
    cout << "Received signal: " << signum << endl;
	cout << "usr" << endl;
    exit();
}

int main() {
    signal(SIGINT, signalHandler_intr); // 시그널 핸들러 등록(인터럽트)
    signal(SIGTERM, signalHandler_term); // 시그널 핸들러 등록(터미널)
    signal(10, signalHandler_usr); // 시그널 핸들러 등록(터미널)

    cout << "Process ID: " << getpid() << endl;

    int signal;
    cout << "Waiting for signal... (Press Ctrl+C to send SIGINT)" << endl;
    while (true) {
        // 무한 루프
    }

    return 0;
}
