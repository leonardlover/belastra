#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

//Struct to simulate the physical memory being divided into m frames
struct frameSpace {
	int frameNumber;
	int usedSpace;
	bool full;
	std::vector<int> frames;
	
	//Returns index of non used frame if any, else returns -1
	//Notice how it returns first available, so it respects FIFO
	int availableIndex(){
		if(full){
			return -1;
		}
		for(int i = 0; i < frameNumber; ++i){
			if(frames[i]==-1){
				return i;
			}
		}
		return -1;
	}
	
	//map new vpn into frame, used or not 
	//returns vpn of evicted page if any, else returns -1
	void map(int vpn, int index, bool newSlotUsed){
		frames[index] = vpn;
		if(newSlotUsed){
			usedSpace++;
			if(usedSpace == frameNumber){
				full = true;
			}
		}			
	} 
	
	void printfs(){
		std::cout << "Frame Space Size: " << frames.size() << std::endl;
		std::cout << "Current Frame Space: ";
		for(int i = 0; i < usedSpace; ++i){
			std::cout << frames[i] << " ";
		}
		std::cout << std::endl;
	}	

	//Constructor
	frameSpace(int f){
		this->frameNumber = f;
		this->full = false;
		this->usedSpace = 0;
		frames.resize(frameNumber);
		for(int i = 0; i < frameNumber; ++i){
			frames[i] = -1;
		}
		//std::cout << "Successfull construction of frameSpace of size " << frameNumber<< std::endl;	
	}
};

//Struct to simulate a page table entry 
struct entry{
	int valid; //Simulates valid bit
	int ref; //reference bit, to be used with LRU and LRU clock
	int virtualPageNumber;
	int mappedFrame;

	//Constructor
	entry(int v, int r, int vpn, int mf){
		this->valid = v;
		this->ref = r;
		this->virtualPageNumber = vpn;
		this->mappedFrame = mf;
		//std::cout << "Successfull construction of entry for the VPN: " << virtualPageNumber << std::endl;
	}
};

//Struct to simulate the Page Table implemented with Hashing and Chaining
struct hashedPageTable {
	int bucketNumber;
	std::vector<std::vector<entry>> table;
	
	//Insert to be used only when map for vpn is not found
	void insertEntry(int vpn, int mf){
		int index = hashFunction(vpn);
		entry* ep;
		entry e(1,0, vpn, mf);
		ep = &e;
		table[index].push_back(e);
	        //std::cout << "ENTRY INSERTED: " << table[index][table[index].size()-1]->virtualPageNumber << std::endl;	
	}
	
	//Indicates whether entry is in table and if it is valid
	//return values:
	//-1: no entry found for specified vpn
	//0: entry found with valid bit set to 0
	//1: entry found with valid bit set to 1
	int searchEntry(int vpn){
		int index = hashFunction(vpn);
		//std::cout << "INDEX SEARCH: " << index << " TABLE[INDEX] SIZE: " << table[index].size() << std::endl;
		for(int i = 0; i < table[index].size(); ++i){
			entry e = table[index][i];
			//std::cout << "WTFFF: "<< e->virtualPageNumber << " " << e->valid <<std::endl;
			if(e.virtualPageNumber == vpn){
				if(e.valid==1){
					return 1;
				}
				return 0;		
			}
		}
		return -1;
	}

	//For already existing page (so already searched accordingly):
	//Could be evicted or re-mapped, meaning a change in valid bit
	void setValid(int vpn, int v){
		int index = hashFunction(vpn);
		for(int i = 0; i < table[index].size(); ++i){
			entry e = table[index][i];
			//std::cout << "SET VALID eVPN: " << e->virtualPageNumber << std::endl;
			if(e.virtualPageNumber == vpn){
				table[index][i].valid = v;
				return;
			}
		}
	}

	//Similar to the previous function, its used for an already existing page.
	void setRef(int vpn, int r){
		int index = hashFunction(vpn);
		for(int i = 0; i < table[index].size(); ++i){
			entry e = table[index][i];
			//std::cout << "SET REFERENCED rVPN: " << r->virtualPageNumber << std::endl;
			if(e.virtualPageNumber == vpn){
				table[index][i].ref = r;
				return;
			}
		}
	}

	// Get reference bit of a certain entry. Returns -1 if not found.
	int getRef(int vpn){
		int index = hashFunction(vpn);
		for(int i = 0; i < table[index].size(); ++i){
			entry e = table[index][i];
			//std::cout << "SET REFERENCED rVPN: " << r->virtualPageNumber << std::endl;
			if(e.virtualPageNumber == vpn){
				return table[index][i].ref;
			}
		}
		return -1;
	}
	
	//Hash: just a modulo for the moment, could be changed
	int hashFunction(int x){
		return (x % bucketNumber);
	}
	
	void printTable(){
		std::cout << "Current Page Table: " << std::endl;
		for(int i = 0; i < table.size(); ++i){
			std::cout << "Table Entry " << i << std::endl;
			for(int j=0; j< table[i].size(); ++j){
				entry p = table[i][j];
				std::cout<< "vpn: " << p.virtualPageNumber << " valid: " << p.valid << " --- ";
			}
			std::cout << std::endl;
		}
	}


	//Constructor
	hashedPageTable(int b){
		this->bucketNumber = b;
		table.resize(bucketNumber);
		//std::cout << "Successful construction of Hashed Page Table with size: " << b << std::endl;
	}
};

int optimal(hashedPageTable* pageTable, frameSpace* fs, int* clockPos){
	
	return 0;
}

//All of the below will return index of evicted vpn 
int fifo(hashedPageTable* pageTable, frameSpace* fs, int* clockPos){
	std::vector<int> frames = fs->frames;
	//It is assumed that order of frames is a queue
	//This is because of how availableIndex is implemented 
	//And because of what is about to happen here:
	//First element (0) is evicted
	//To preserve the order, other members move up in the queue:
	for(int i = 0; i < frames.size()-1; ++i){
		frames[i] = frames[i+1];
	}
	fs->frames = frames;
	//Returns index to use
	return frames.size()-1;
}

int lru(hashedPageTable* pageTable, frameSpace* fs, int* clockPos){
	std::vector<int> frames = fs->frames;

	// When popping from a LRU array, we're still
	// popping the element at [0] and moving the
	// rest of the array one space. Therefore, we use
	// the same algorithm from FIFO to replace
	// this element
	for(int i = 0; i < frames.size()-1; ++i){
		frames[i] = frames[i+1];
	}
	fs->frames = frames;
	// Returns index to use
	return frames.size()-1;
}

void lruUpdate(frameSpace* fs, int hit){
	std::vector<int> frames = fs->frames;

	// We musts find the index of the vpn
	// that has been recently used to move it
	// back to the start of the queue
	int recentlyUsed;
	for(int i = 0; i < frames.size(); ++i){
		if(frames[i] == hit){
			recentlyUsed = i;
			break;
		} 
	}

	// Update the queue accordingly, using
	// recentlyUsed as the starting point, and later
	// adding the most recently used frame at the start
	for(int i = recentlyUsed; i < frames.size()-1; ++i){
		frames[i] = frames[i+1];
	}
	frames[frames.size()-1] = hit;

	// Update framespace
	fs->frames = frames;
}

int lruClock(hashedPageTable* pageTable, frameSpace* fs, int* clockPos){
	std::vector<int> frames = fs->frames;

	// We need the PageTable to get and set reference bits
	hashedPageTable tb = *pageTable;

	// We need the clock's position from where it left off
	int circularQueueIndex = *clockPos;

	while(true){
		
		// Using the clock, we get the reference bits of each element in frames
		// Each time a 1 is found, we set it to 0 - their second chance is given,
		// and then we keep ticking the clock.
		// When we find a 0, we stop the clock and set its position, we
		// replace the table to the one with new reference values, and then
		// we return the index in which the new element will be inserted
		if(tb.getRef(frames[circularQueueIndex % (int)frames.size()]) == 1){
			tb.setRef(frames[circularQueueIndex % (int)frames.size()], 0);
		} else {
			*clockPos = (circularQueueIndex % (int)frames.size());
			tb.setValid(frames[circularQueueIndex % (int)frames.size()], 0);
			*pageTable = tb; 
			return (circularQueueIndex % (int)frames.size());
		}
		
		circularQueueIndex++;
	}

	return 0;
}

int main(int argc, char *argv[]){
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

    std::cout << "Received Input: " << m << " " << a << " " << f << std::endl;
	
    //Continuation of program
    //Obtention of file input
    std::vector<int> pageReferences;
    std::ifstream file(f);
    std::string s;
    getline(file, s);
    file.close();

    //std::cout << s << std::endl;
    
    std::istringstream stream(s);
    int number;

    while(stream >> number){
    	pageReferences.push_back(number);
    }

    /*
    for(int num: pageReferences){
    	std::cout << num << " ";
    }
    std::cout << std::endl;
    */

    //Inits
    frameSpace fs(m);
    frameSpace* fsPointer = &fs;
    //m table entries at the moment
    hashedPageTable pageTable(m);
	hashedPageTable* ptPointer = &pageTable;	
    
    //function pointer for page replacement 
    int (*pageReplacer)(hashedPageTable*, frameSpace*, int*);
    if(strcmp(a, "OPTIMAL")==0){
    	pageReplacer = optimal;
    }	
    if(strcmp(a, "FIFO")==0){
    	pageReplacer = fifo;
    }
    if(strcmp(a, "LRU")==0){
    	pageReplacer = lru;
    }
    if(strcmp(a, "LRU_CLOCK")==0){
    	pageReplacer = lruClock;
    }
    
    //Keep track of amount of page faults
    int pageFaults = 0;	

	//Index for LRU Clock, may not be used
	int clockIndex = 0;

    //Now the main page referencing happens
    for(int i = 0; i< pageReferences.size(); ++i ){
    	int vpn = pageReferences[i];
	//Exception for page fault
	try{
	   int output = pageTable.searchEntry(vpn);
	   //std::cout << "SEARCH DONE: " << output << std::endl;
  	   if(output == 1){
		//Hit

		//if LRU, we must update the LRU accordingly
		if(pageReplacer == lru){
			lruUpdate(fsPointer, vpn);
		}

		//if LRU Clock, we must update the referenced bit
		if(pageReplacer == lruClock){
			pageTable.setRef(vpn, 1);
		}

	   } else{
		//Miss
	   	throw(output);
	   }
	}
	catch(int searchCode){
	    if(searchCode == 0){
	    	//Page was evicted previosly: Re-map into memory
	     	//If the page was evicted, the frame space is full

		if(pageReplacer == fifo || pageReplacer == lru){

			// Element WILL BE at the first position, though this can be
			// managed inside the pageReplacer() function. Should we?
			int oldvpn = fs.frames[0];
			int rframe = pageReplacer(ptPointer, fsPointer, &clockIndex);
	
			fs.map(vpn, rframe, false);
			//std::cout << "OLD VPN: " << oldvpn << " NEW VPN: " << vpn << std::endl;
			pageTable.setValid(oldvpn, 0); //Evicted
			pageTable.setValid(vpn, 1); //re-mapped
			pageTable.setRef(vpn, 1); // referenced
		} else{
			// We don't know for certain if the element will be in the first
			// position as for the nature of the clock. pageRaplacer()
			// manages the valid bit of the old vpn.
			int rframe = pageReplacer(ptPointer, fsPointer, &clockIndex);

			fs.map(vpn, rframe, false);
			//std::cout << "OLD VPN: " << oldvpn << " NEW VPN: " << vpn << std::endl;
			pageTable.setValid(vpn, 1); //re-mapped
			pageTable.setRef(vpn, 1); // referenced
		}

		
	    }
	    if(searchCode == -1){
		//std::cout << "FLAG FOR -1 SEARCHCODE" << std::endl;
            	//Page is being referenced for the first time
	    	//Map into memory
		//The frame space could be or not be full
		int available = fs.availableIndex();
		if(available == -1){ //Full
			if(pageReplacer == fifo || pageReplacer == lru){

				// Element WILL BE at the first position, though this can be
				// managed inside the pageReplacer() function. Should we?
				int oldvpn = fs.frames[0];
				int rframe = pageReplacer(ptPointer, fsPointer, &clockIndex);
		
				fs.map(vpn, rframe, false);
				pageTable.insertEntry(vpn, rframe);
				
				pageTable.setValid(oldvpn, 0); //Evicted
				pageTable.setValid(vpn, 1); //re-mapped
				pageTable.setRef(vpn, 1); // referenced
			} else{
				// We don't know for certain if the element will be in the first
				// position as for the nature of the clock. pageRaplacer()
				// manages the valid bit of the old vpn.
				int rframe = pageReplacer(ptPointer, fsPointer, &clockIndex);
					
				fs.map(vpn, rframe, false);
				pageTable.insertEntry(vpn, rframe);

				pageTable.setValid(vpn, 1); //re-mapped
				pageTable.setRef(vpn, 1); // referenced
			}
		}else{ //Not full
		       	//std::cout << "FLAG NOT FULL: " << available << std::endl;
			fs.map(vpn, available, true);
			//Add to page table
			pageTable.insertEntry(vpn, available);
		}
	    }
	
	}
	std::cout << "------------------------------------" << std::endl;
	fs.printfs();
	pageTable.printTable();
	std::cout << "------------------------------------" << std::endl;
    }
    
   
	
    return EXIT_SUCCESS;
}
