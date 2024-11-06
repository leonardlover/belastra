#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <iostream>

int main(int argc, char *argv[])
{
    int opt;
    const char *algorithms[] = {"OPTIMAL", "FIFO", "LRU", "LRU_CLOCK"};
    bool found;
    struct stat stat_buffer;
    long m = 0;
    char *a = NULL;
    char *f = NULL;

    // Read command line arguments
    while ((opt = getopt(argc, argv, "m:a:f:")) != -1) {
        switch (opt) {
        case 'm':
            m = strtol(optarg, NULL, 0);
            break;
        case 'a':
            a = optarg;
            break;
        case 'f':
            f = optarg;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    // Sanitize input
    if (m <= 0) {
        std::cerr << "Number of frames has to be positive." << std::endl;
        return EXIT_FAILURE;
    }

    if (!a) {
        std::cerr << "-a requires an argument." << std::endl;
        return EXIT_FAILURE;
    } else {
        found = false;
        for (auto algo: algorithms) {
            if (strcmp(a, algo) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << "Algorithm has to be: 'OPTIMAL', 'FIFO', 'LRU', 'LRU_CLOCK'" << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (!f) {
        std::cerr << "-f requires an argument." << std::endl;
        return EXIT_FAILURE;
    } else if (stat(f, &stat_buffer) != 0) {
        std::cerr << "Path: '" << f << "' does not exist." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << m << " " << a << " " << f << std::endl;

    return EXIT_SUCCESS;
}
