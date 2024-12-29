#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cstdlib>
#include <ctime> 

const int MAX_BUFFER_SIZE = 1000;
std::queue<int> input_buffer, output_buffer;
int buffer_size;
int max_items;

std::mutex input_mutex, output_mutex, count_mutex;
std::condition_variable input_full_cv, output_full_cv, input_empty_cv, output_empty_cv;
bool input_filled = false, output_filled = false;
int produced_count = 0, copied_count = 0, consumed_count = 0;

void producer() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(count_mutex);
            if (produced_count >= max_items) return;
        }

        int item = rand() % 100; 
        {
            std::unique_lock<std::mutex> lock(input_mutex);
            input_empty_cv.wait(lock, [] { return input_buffer.size() < buffer_size; });
            input_buffer.push(item);
            std::cout << item << " inserting\n";

            if (input_buffer.size() == buffer_size) {
                input_filled = true;
                input_full_cv.notify_all();
            }
        }

        {
            std::lock_guard<std::mutex> lock(count_mutex);
            produced_count++;
        }

        if (produced_count >= max_items) break;
    }
}

void copier() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(count_mutex);
            if (copied_count >= max_items) return;
        }

        std::unique_lock<std::mutex> lock(input_mutex);
        input_full_cv.wait(lock, [] { return input_filled; });

        {
            std::unique_lock<std::mutex> out_lock(output_mutex);
            while (!input_buffer.empty()) {
                int item = input_buffer.front();
                input_buffer.pop();
                output_buffer.push(item);
            }
            copied_count = produced_count;

            output_filled = true;
            output_full_cv.notify_all();
            input_filled = false;
            input_empty_cv.notify_all();
        }
    }
}

void consumer() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(count_mutex);
            if (consumed_count >= max_items) return;
        }

        std::unique_lock<std::mutex> lock(output_mutex);
        output_full_cv.wait(lock, [] { return output_filled; });

        while (!output_buffer.empty()) {
            int item = output_buffer.front();
            output_buffer.pop();
            std::cout << item << " consuming\n";

            {
                std::lock_guard<std::mutex> lock(count_mutex);
                consumed_count++;
            }

            if (consumed_count >= max_items) break;
        }

        output_filled = false;
        output_empty_cv.notify_all();
        if (consumed_count >= max_items) break;
    }
}

int main() {
    std::cout << "Enter buffer size: ";
    std::cin >> buffer_size;
    if (buffer_size > MAX_BUFFER_SIZE) buffer_size = MAX_BUFFER_SIZE;

    max_items = buffer_size;

    srand(time(0));

    std::vector<std::thread> producers, copiers, consumers;

    producers.emplace_back(producer);
    copiers.emplace_back(copier);
    consumers.emplace_back(consumer);

    for (auto &p : producers) p.join();
    for (auto &c : copiers) c.join();
    for (auto &co : consumers) co.join();

    return 0;
}
