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

class Monitor {
    private:
        int *items;
        int size;
        int count;
        int in, out;
        std::mutex mtx;
        std::condition_variable isFull;
        std::condition_variable isEmpty;

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
                in = count - 1;
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
                    in = count - 1;
                }
                
            }
        }

    public:
        Monitor(int s){
            size = s;
            items = new int[size];
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

        void consumeTest(){
            // ESTO ES SÓLO DE PRUEBA, FALTA LOCK Y DEMÁS COSAS

            if(count > 0){
                items[out] = 0;
                out = (out + 1) % size;
                count--;

                reduceSize();
            }
        }

        void produceTest(){
            // ESTO ES SÓLO DE PRUEBA, FALTA LOCK Y DEMÁS COSAS

            if(size > count){
                items[in] = 1;
                in = (in + 1) % size;
                count++;

                duplicateSize();
            }
        }
};

void producer(Monitor& monitor){
    monitor.printInfo();
    for(int i=0; i<4; i++){
        monitor.produceTest();
    }
    monitor.printInfo();
    std::cout << "PRODUCER FUNCTION ENDED" << std::endl << std::endl;
}

void consumer(Monitor& monitor){
    monitor.printInfo();
    for(int i=0; i<4; i++){
        monitor.consumeTest();
        monitor.printInfo();
    }
    std::cout << "CONSUMER FUNCTION ENDED" << std::endl << std::endl;
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

    std::cout << p << " " << c << " " << s << " " << t << std::endl;

    Monitor monitor(4);

    std::thread producerTest(producer, std::ref(monitor));

    producerTest.join();

    std::thread consumerTest(consumer, std::ref(monitor));

    consumerTest.join();

    return EXIT_SUCCESS;
}
