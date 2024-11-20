# belastra
A simple implementation of the solution of the producers-consumers problem and a simulation of basic page replacements algorithms.

## Compilation
Choose one of the directories available to compile, so either `cd page_replacement` or `cd producer_consumers`. From here,
compile the main.cpp files.

```bash
g++ main.cpp -o main
```

## Execution

### page_replacement

To execute the main file, there are values which must be filled in. 
* `-m` The initial ammount of frames (positive intiger)
* `-a` The corresopnding algorithm which will be used. This value can be `FIFO`, `OPTIMAL`, `LRU` or `LRU_CLOCK`.
* `-f` The file containing the order of the virtual page numbers. The repository contains `referencias.txt` as an example of the format to follow.

A valid execution command will be, for example:

```bash
./main -m 3 -a FIFO -f referencias.txt
```

Afterwards, a sequence of tables will be printed, showing how each of the insertions changed according to the algorithm used. Finally,
there will be a singular message displaying the ammount of page faults throughout the process.

### producer_consumers

FILL!

