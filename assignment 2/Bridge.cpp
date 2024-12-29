#include <iostream>
#include <thread>
#include <vector>
#include <semaphore.h>
#include <mutex>

using namespace std;

sem_t bridgeSemaphore;  
sem_t directionSemaphore; 
mutex printMutex;
mutex countMutex;          
int maxPeople;
int crossedCount = 0;  

void crossBridge(const string& direction, int id, bool isFirst) {
    sem_wait(&directionSemaphore);

    sem_wait(&bridgeSemaphore);

    {
        lock_guard<mutex> lock(printMutex);
        cout << "person is passing to " << direction << " " << id << endl;
        if (isFirst) {
            cout << "first person crossing the bridge to " << direction << endl;
        }
    }

    this_thread::sleep_for(chrono::seconds(1));

    sem_post(&bridgeSemaphore);

    {
        lock_guard<mutex> lock(countMutex);
        crossedCount++;
    }

    sem_post(&directionSemaphore);
}

int main() {
    cout << "Enter the maximum number of people who can cross the bridge: ";
    cin >> maxPeople;

    sem_init(&bridgeSemaphore, 0, maxPeople);  
    sem_init(&directionSemaphore, 0, 1);      

    vector<thread> threads;

    //crossing for each direction
    threads.push_back(thread(crossBridge, "south", 1, true));  
    threads.push_back(thread(crossBridge, "north", 1, true)); 

    for (int i = 2; i <= maxPeople; ++i) {
        threads.push_back(thread(crossBridge, "south", i, false)); 
        threads.push_back(thread(crossBridge, "north", i, false)); 
    }

    //join
    for (auto& t : threads) {
        t.join();
    }

    sem_destroy(&bridgeSemaphore);
    sem_destroy(&directionSemaphore);

    return 0;
}
