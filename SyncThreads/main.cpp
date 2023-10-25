#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

// ���������� ����������
std::vector<int> arr;
std::vector<HANDLE> marker_threads;
CRITICAL_SECTION cs;
HANDLE start_event;
HANDLE terminate_event;

// ������� ������ marker
DWORD WINAPI marker_thread(LPVOID arg) {
    int marker_id = (int)arg;
    std::cout << marker_id;
    srand(time(NULL) + marker_id);

    while (1) {
        WaitForSingleObject(start_event, INFINITE); // ���� ������� �� ������ ������

        EnterCriticalSection(&cs);

        int rand_num = rand();
        int index = rand_num % arr.size();

        if (arr[index] == 0) {
            Sleep(5); // ����� 5 �����������
            arr[index] = marker_id;
            std::cout << arr[index];
            Sleep(5); // ����� 5 �����������
        }
        else {
            // ������������� ������ main � ������������� ����������� ������
            SetEvent(terminate_event);
            LeaveCriticalSection(&cs);
            ResetEvent(start_event);
            break;
        }

        LeaveCriticalSection(&cs);
        ResetEvent(start_event);
    }

    return 0;
}

int main() {
    int n;
    std::cout << "Enter the size of the array: ";
    std::cin >> n;
    arr.resize(n, 0);

    int num_markers;
    std::cout << "Enter the number of marker threads: ";
    std::cin >> num_markers;

    InitializeCriticalSection(&cs);
    start_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    terminate_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    // �������� ������� marker
    for (int i = 1; i < num_markers + 1; i++) {
        HANDLE marker = CreateThread(NULL, 0, marker_thread, (LPVOID)i, 0, NULL);
        marker_threads.push_back(marker);
    }

    while (1) {
        EnterCriticalSection(&cs);
        // ����, ���� ��� ������ marker �� �������� ���� ������
        /*bool all_done = true;
        for (int val: arr) {
            if (val == 0) {
                all_done = false;
                break;
            }
        }
        if (all_done) {
            break;
        }*/

        // ������� ���������� ������� �� �������
        std::cout << "Array content: ";
        for (int val : arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // ����������� ���������� ����� ������ marker ��� ����������
        int marker_to_terminate;
        std::cout << "Enter the marker thread number to terminate (0 to exit): ";
        std::cin >> marker_to_terminate;

        if (marker_to_terminate == 0) {
            LeaveCriticalSection(&cs);
            break;
        }

        // ������ ������ �� ������ ������
        LeaveCriticalSection(&cs);
        SetEvent(start_event);

        // ������� ������ ������ marker � ������������� ����������� ������
        WaitForSingleObject(terminate_event, INFINITE);

        // ������� ���������� ������� �� �������
        std::cout << "Array content after termination: ";
        for (int val : arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // ���� ������ �� ����������� ������ ���������� ������� marker
        LeaveCriticalSection(&cs);
        ResetEvent(terminate_event);
    }

    // ��������� ������ ���� ������� marker
    for (HANDLE marker : marker_threads) {
        WaitForSingleObject(marker, INFINITE);
        CloseHandle(marker);
    }

    // ����������� �������
    DeleteCriticalSection(&cs);
    CloseHandle(start_event);
    CloseHandle(terminate_event);

    return 0;
}
