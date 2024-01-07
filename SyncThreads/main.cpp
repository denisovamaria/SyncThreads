#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include "C:\Users\denisovamaria\source\repos\SyncThreads\SyncThreads\MainProcess.h"

MainProcess* mp;

DWORD WINAPI marker_thread(LPVOID arg) {
    int marker_id = (int)arg;
    WaitForSingleObject(mp->start_event[marker_id - 1], INFINITE);
    srand(time(NULL) + marker_id);
    int marked_elements = 0;

    while (1) {

        EnterCriticalSection(&mp->cs);

        int index = rand() % mp->arr.size();

        if (mp->arr[index] == 0) {
            marked_elements++;
            Sleep(5);
            mp->arr[index] = marker_id;
            Sleep(5);
            LeaveCriticalSection(&mp->cs);
        }
        else {
            std::cout << "Marker thread " << marker_id << ":" << '\n' << "Number of marked elements: " << marked_elements << '\n' << "Failed to mark element index: " << index << '\n' << '\n';
            marked_elements = 0;
            ResetEvent(mp->start_event[marker_id - 1]);
            SetEvent(mp->terminate_event[marker_id - 1]);
            LeaveCriticalSection(&mp->cs);
            WaitForSingleObject(mp->start_event[marker_id - 1], INFINITE);
            EnterCriticalSection(&mp->cs);
            if (!mp->terminated_threads[marker_id - 1])
            {
                for (int i = 0; i < mp->arr.size(); i++)
                    if (mp->arr[i] == marker_id)
                        mp->arr[i] = 0;
                SetEvent(mp->terminate_event[marker_id - 1]);
                LeaveCriticalSection(&mp->cs);
                break;
            }
            else
            {
                LeaveCriticalSection(&mp->cs);
                continue;
            }
        }
    }
    return 0;
}

int main() {
    int n;
    std::cout << "Enter the size of the array: ";
    std::cin >> n;

    int num_markers;
    std::cout << "Enter the number of marker threads: ";
    std::cin >> num_markers;
    std::cout << '\n';

    mp = new MainProcess(n, num_markers);
    mp->createEvents();
    mp->createMarkers(marker_thread);
    mp->startEventsSet();
    mp->mainCycle();
}