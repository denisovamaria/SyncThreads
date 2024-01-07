#include "pch.h"
#include "CppUnitTest.h"
#include "C:\Users\denisovamaria\source\repos\SyncThreads\SyncThreads\MainProcess.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

DWORD WINAPI testThread(LPVOID arg)
{
    return 0;
}

MainProcess* testProcess = nullptr;

static DWORD WINAPI marker_thread_test(LPVOID arg) {
    WaitForSingleObject(testProcess->start_event[(int)arg - 1], INFINITE);
    int marker_id = (int)arg;
    srand(time(NULL) + marker_id);
    int marked_elements = 0;

    while (true) {

        EnterCriticalSection(&testProcess->cs);

        int index = rand() % testProcess->arr.size();

        if (testProcess->arr[index] == 0) {
            marked_elements++;
            Sleep(5);
            testProcess->arr[index] = marker_id;
            Sleep(5);
            LeaveCriticalSection(&testProcess->cs);
        }
        else {
            marked_elements = 0;
            ResetEvent(testProcess->start_event[marker_id - 1]);
            SetEvent(testProcess->terminate_event[marker_id - 1]);
            LeaveCriticalSection(&testProcess->cs);
            WaitForSingleObject(testProcess->start_event[marker_id - 1], INFINITE);
            EnterCriticalSection(&testProcess->cs);
            if (!testProcess->terminated_threads[marker_id - 1])
            {
                for (int i = 0; i < testProcess->arr.size(); i++)
                    if (testProcess->arr[i] == marker_id)
                        testProcess->arr[i] = 0;
                SetEvent(testProcess->terminate_event[marker_id - 1]);
                LeaveCriticalSection(&testProcess->cs);
                break;
            }
            else
            {
                LeaveCriticalSection(&testProcess->cs);
                continue;
            }
        }
    }
    return 0;
}

namespace MainProcesstest
{
	TEST_CLASS(MainProcesstest)
	{
	public:
		
        TEST_METHOD(TestMethod1)
        {
            int n = 4;
            int num_threads = 10;

            MainProcess mainProcess(4, num_threads);

            Assert::AreEqual(4, mainProcess.n);
            Assert::AreEqual(num_threads, mainProcess.num_threads);
            Assert::AreEqual(static_cast<size_t>(n), mainProcess.arr.size());

            for (int i = 0; i < n; i++)
                Assert::AreEqual(0, mainProcess.arr[i]);
        }

        TEST_METHOD(InitializeArray)
        {
            int n = 5;
            int num_threads = 3;
            MainProcess mainProcess(n, num_threads);

            mainProcess.createEvents();
            mainProcess.createMarkers(testThread);

            Assert::AreEqual(static_cast<size_t>(n), mainProcess.arr.size());
            for (int i = 0; i < n; ++i) {
                Assert::AreEqual(0, mainProcess.arr[i]);
            }
        }

        TEST_METHOD(CreateEvents)
        {
            int n = 5;
            int num_threads = 3;
            MainProcess mainProcess(n, num_threads);

            mainProcess.createEvents();

            Assert::AreEqual(static_cast<size_t>(num_threads), mainProcess.start_event.size());
            for (int i = 0; i < num_threads; ++i) {
                Assert::IsNotNull(mainProcess.start_event[i]);
                Assert::IsNotNull(mainProcess.terminate_event[i]);
            }
        }

        TEST_METHOD(CreateMarkers)
        {
            int n = 5;
            int num_threads = 3;
            MainProcess mainProcess(n, num_threads);

            mainProcess.createMarkers(testThread);

            Assert::AreEqual(static_cast<size_t>(num_threads), mainProcess.marker_threads.size());
        }

        TEST_METHOD(MainCycle)
        {
            int n = 10;
            int num_threads = 7;
            testProcess = new MainProcess(n, num_threads);

            testProcess->createEvents();
            testProcess->createMarkers(marker_thread_test);
            testProcess->startEventsSet();
            testProcess->mainCycle_test();

            for (int i = 0; i < n; i++)
                Assert::AreEqual(0, testProcess->arr[i]);
        }

	};
}
