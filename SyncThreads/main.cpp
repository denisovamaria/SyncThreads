#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

// Глобальные переменные
std::vector<int> arr;
std::vector<HANDLE> marker_threads;
CRITICAL_SECTION cs;
HANDLE start_event;
HANDLE terminate_event;

// Функция потока marker
DWORD WINAPI marker_thread(LPVOID arg) {
    int marker_id = (int)arg;
    std::cout << marker_id;
    srand(time(NULL) + marker_id);

    while (1) {
        WaitForSingleObject(start_event, INFINITE); // Ждем сигнала на начало работы

        EnterCriticalSection(&cs);

        int rand_num = rand();
        int index = rand_num % arr.size();

        if (arr[index] == 0) {
            Sleep(5); // Пауза 5 миллисекунд
            arr[index] = marker_id;
            std::cout << arr[index];
            Sleep(5); // Пауза 5 миллисекунд
        }
        else {
            // Сигнализируем потоку main о невозможности продолжения работы
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

    // Создание потоков marker
    for (int i = 1; i < num_markers + 1; i++) {
        HANDLE marker = CreateThread(NULL, 0, marker_thread, (LPVOID)i, 0, NULL);
        marker_threads.push_back(marker);
    }

    while (1) {
        EnterCriticalSection(&cs);
        // Ждем, пока все потоки marker не закончат свою работу
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

        // Выводим содержимое массива на консоль
        std::cout << "Array content: ";
        for (int val : arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // Запрашиваем порядковый номер потока marker для завершения
        int marker_to_terminate;
        std::cout << "Enter the marker thread number to terminate (0 to exit): ";
        std::cin >> marker_to_terminate;

        if (marker_to_terminate == 0) {
            LeaveCriticalSection(&cs);
            break;
        }

        // Подаем сигнал на начало работы
        LeaveCriticalSection(&cs);
        SetEvent(start_event);

        // Ожидаем сигнал потока marker о невозможности продолжения работы
        WaitForSingleObject(terminate_event, INFINITE);

        // Выводим содержимое массива на консоль
        std::cout << "Array content after termination: ";
        for (int val : arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // Даем сигнал на продолжение работы оставшимся потокам marker
        LeaveCriticalSection(&cs);
        ResetEvent(terminate_event);
    }

    // Завершаем работу всех потоков marker
    for (HANDLE marker : marker_threads) {
        WaitForSingleObject(marker, INFINITE);
        CloseHandle(marker);
    }

    // Освобождаем ресурсы
    DeleteCriticalSection(&cs);
    CloseHandle(start_event);
    CloseHandle(terminate_event);

    return 0;
}
