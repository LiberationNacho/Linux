#include <iostream>
#include <csignal>
#include <unistd.h>

using namespace std;

int main() {
    pid_t pid;
    cout << "Enter the process ID to send signal: ";
    cin >> pid;

    int signal;
    cout << "Enter the signal number to send: ";
    cin >> signal;

    kill(pid, signal); // 시그널 전달

    cout << "Signal " << signal << " sent to process " << pid << endl;

    return 0;
}

