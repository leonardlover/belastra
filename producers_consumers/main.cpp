#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

long p = 0;
long c = 0;
long s = 0;
long t = 0;
long p_done = 0;

std::ofstream log_file;

class Monitor
{
private:
    int *items;
    int size;
    int count;
    int in, out;
    int time;
    std::mutex mtx;
    std::condition_variable productAvailable;
    std::condition_variable spaceAvailable;

    void duplicateSize()
    {
        /*
           Como realmente nos importa el contador y no dónde están exactamente los elementos,
           simplemente creamos un nuevo arreglo y lo llenamos con 1's hasta donde corresponda
           */ 
        if(count == size){
            delete items;
            size = 2*size;
            items = new int[size];

            int i = 0;
            for(; i<count; i++)
                items[i] = 1;
            for(; i<size; i++)
                items[i] = 0;

            out = 0;
            in = count;

            log_file << "SIZE GOT DUPLICATED, IT'S NOW " << size << std::endl;
        }
    }

    void reduceSize()
    {
        /*
           Como realmente nos importa el contador y no dónde están exactamente los elementos,
           simplemente creamos un nuevo arreglo y lo llenamos con 1's hasta donde corresponda
           */ 
        if(count <= size/4 && size > 1){
            delete items;
            size = size/2;
            items = new int[size];

            int i = 0;
            for(; i<count; i++)
                items[i] = 1;
            for(; i<size; i++)
                items[i] = 0;

            out = 0;
            if (size == 1)
                in = 0;
            else
                in = count;

            log_file << "SIZE GOT REDUCED, IT'S NOW " << size << std::endl;
        }
    }

public:
    Monitor(int s, int t)
    {
        size = s;
        items = new int[size];
        time = t;
        count = 0;
        in = 0;
        out = 0;
    }

    void printInfo()
    {
        std::cout << "Size: " << size << std::endl;
        std::cout << "Count: " << count << std::endl;
        std::cout << "In position: " << in << std::endl;
        std::cout << "Out position: " << out << std::endl << std::endl;

        std::cout << "Elements in items[]:" << std::endl;
        for(int i = 0; i < size; i++)
            std::cout << i << ": " << items[i] << std::endl;
        std::cout << std::endl;
    }

    void consume(int id)
    {
        std::unique_lock<std::mutex> mtxWrapper(mtx);
        log_file << "CONSUMER " << id << " STARTED" << std::endl;

        // if all producers are done, make consumer sleep and exit
        // ONLY DO THIS IS QUEUE IS EMPTY
        if (p_done == p && count == 0) {
            log_file << "CONSUMER " << id << " BEGINS SLEEPING!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(t));
            log_file << "CONSUMER " << id << " AWOKE FROM SLUMBER TO EXIT!" << std::endl;
            return;
        }

        if(count == 0) {
            productAvailable.wait(mtxWrapper);
            log_file << "CONSUMER " << id << " WOKE UP!" << std::endl;
        }

        items[out] = 0;
        out = (out + 1) % size;
        count--;

        reduceSize();

        spaceAvailable.notify_one();

        log_file << "CONSUMER " << id << " ENDED" << std::endl;

        mtxWrapper.unlock();

        // consume again, until queue is empty
        // no need for while or if since thread WILL sleep when all producers
        // are done and all elements consumed
        consume(id);
    }

    void produce(int id)
    {
        std::unique_lock<std::mutex> mtxWrapper(mtx);
        log_file << "PRODUCER " << id << " STARTED" << std::endl;

        if(count == size) {
            spaceAvailable.wait(mtxWrapper);
            log_file << "PRODUCER " << id << " WOKE UP!" << std::endl;
        }

        items[in] = 1;
        in = (in + 1) % size;
        count++;

        duplicateSize();

        productAvailable.notify_one();

        log_file << "PRODUCER " << id << " ENDED" << std::endl;
        p_done++;

        mtxWrapper.unlock();
    }
};

void producer_job(Monitor& monitor, int id)
{ 
    monitor.produce(id);
}

void consumer_job(Monitor& monitor, int id)
{
    monitor.consume(id);
}

int main(int argc, char *argv[])
{
    int opt, i;

    log_file.open("log_file.txt");

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "p:c:s:t:")) != -1) {
        switch (opt) {
            case 'p':
                p = strtol(optarg, NULL, 0);
                break;
            case 'c':
                c = strtol(optarg, NULL, 0);
                break;
            case 's':
                s = strtol(optarg, NULL, 0);
                break;
            case 't':
                t = strtol(optarg, NULL, 0);
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    // Sanitize input
    if (p <= 0) {
        std::cerr << "Number of producers has to be positive." << std::endl;
        return EXIT_FAILURE;
    }
    if (c <= 0) {
        std::cerr << "Number of consumers has to be positive." << std::endl;
        return EXIT_FAILURE;
    }
    if (s <= 0) {
        std::cerr << "Initial size of queue has to be positive." << std::endl;
        return EXIT_FAILURE;
    }
    if (t <= 0) {
        std::cerr << "Consumers waiting time has to be positive." << std::endl;
        return EXIT_FAILURE;
    }

    Monitor monitor(s, t);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (i = 0; i < p; i++)
        producers.emplace_back(producer_job, std::ref(monitor), i + 1);

    for(i = 0; i < c; i++)
        consumers.emplace_back(consumer_job, std::ref(monitor), i + 1);

    for(auto& prod: producers)
        prod.join();

    for(auto& cons: consumers)
        cons.join();

    return EXIT_SUCCESS;
}
