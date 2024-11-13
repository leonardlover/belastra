#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <condition_variable>
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

class Monitor {
    private:
        int *items;
        int size;
        int count;
        int in, out;
        int time;
        std::mutex mtx;
        std::condition_variable productAvailable;
        std::condition_variable spaceAvailable;

        void duplicateSize(){
            /*
            Como realmente nos importa el contador y no dónde están exactamente los elementos,
            simplemente creamos un nuevo arreglo y lo llenamos con 1's hasta donde corresponda
            */ 
            if(count == size){
                delete items;
                size = 2*size;
                items = new int[size];

                int i = 0;
                for(i; i<count; i++){
                    items[i] = 1;
                }
                for(i; i<size; i++){
                    items[i] = 0;
                }

                out = 0;
                in = count;

                std::cout << "SIZE GOT DUPLICATED" << std::endl << std::endl;
            }
        }

        void reduceSize(){
            /*
            Como realmente nos importa el contador y no dónde están exactamente los elementos,
            simplemente creamos un nuevo arreglo y lo llenamos con 1's hasta donde corresponda
            */ 
            if(count == size/4 && size > 1){
                delete items;
                size = size/2;
                items = new int[size];

                int i = 0;
                for(i; i<count; i++){
                    items[i] = 1;
                }
                for(i; i<size; i++){
                    items[i] = 0;
                }

                out = 0;
                if(size == 1){
                    in = 0;
                }
                else {
                    in = count;
                }

                std::cout << "SIZE GOT REDUCED" << std::endl << std::endl;                
            }
        }

    public:
        Monitor(int s, int t){
            size = s;
            items = new int[size];
            time = t;
            count = 0;
            in = 0;
            out = 0;
        }

        void printInfo(){
            std::cout << "Size: " << size << std::endl;
            std::cout << "Count: " << count << std::endl;
            std::cout << "In position: " << in << std::endl;
            std::cout << "Out position: " << out << std::endl << std::endl;

            std::cout << "Elements in items[]: " << std::endl;
            for(int i=0; i<size; i++){
                std::cout << i << ": " << items[i] << std::endl;
            }
            std::cout << std::endl;
        }

        void consume(int id){
            std::unique_lock<std::mutex> mtxWrapper(mtx);
            std::cout << "CONSUMER " << id << " STARTED" << std::endl << std::endl;

            if(count == 0){
                productAvailable.wait(mtxWrapper);
                std::cout << "CONSUMER " << id << " WOKE UP!" << std::endl;
            }

            items[out] = 0;
            out = (out + 1) % size;
            count--;

            reduceSize();

            spaceAvailable.notify_one();

            printInfo();
            std::cout << "CONSUMER " << id << " ENDED" << std::endl;
            std::cout << "--------------------------" << std::endl;

            mtxWrapper.unlock();
        }

        void produce(int id){
            std::unique_lock<std::mutex> mtxWrapper(mtx);
            std::cout << "PRODUCER " << id << " STARTED" << std::endl << std::endl;

            if(count == size){
                spaceAvailable.wait(mtxWrapper);
                std::cout << "PRODUCER " << id << " WOKE UP!" << std::endl;
            }

            items[in] = 1;
            in = (in + 1) % size;
            count++;

            duplicateSize();

            productAvailable.notify_one();

            printInfo();
            std::cout << "PRODUCER " << id << " ENDED" << std::endl;
            std::cout << "--------------------------" << std::endl;

            mtxWrapper.unlock();
        }
};

void producer(Monitor& monitor, int id){ 
    monitor.produce(id);
}

void consumer(Monitor& monitor, int id){
    monitor.consume(id);
}

int main(int argc, char *argv[])
{
    int opt;
    long p = 0;
    long c = 0;
    long s = 0;
    long t = 0;

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

    for(int i=0; i<p; i++){
        producers.emplace_back(producer, std::ref(monitor), i+1);
    }

    for(int i=0; i<c; i++){
        consumers.emplace_back(consumer, std::ref(monitor), i+1);
    }

    for(auto& producer : producers){
        producer.join();
    }

    for(auto& consumer : consumers){
        consumer.join();
    }

    return EXIT_SUCCESS;
}
