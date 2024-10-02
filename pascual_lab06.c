/*
	Author: Jamaika T. Pascual
	B4L
	202102167

    EXER 6:
 */

 #include<stdio.h>
#include<stdlib.h>
#define TLB_SIZE 16

//TLB data structure
typedef struct TLBnode{
	int pageNum;
	int frameNumber;
	struct TLBnode *next;
}TLB;

TLB *tlb_head = NULL;

//Initialize an empty page table (size 256)
void initializePageTable(int pageTable[]){
	int i;
	for(i=0;i<256;i+=1){
		pageTable[i] = -1;
	}
}

//Update the TLB: add a new page-frame pair and handle TLB overflow
int updateTLB(int pnum, int freeNum, int count, TLB **tlb_head) {
	TLB *new_node = (TLB *) malloc(sizeof(TLB));
	new_node->pageNum = pnum;
	new_node->frameNumber = freeNum;
	new_node->next = NULL;
	
	//Check if TLB is empty
	if (*tlb_head == NULL) {
		*tlb_head = new_node;
	} else {
		// Traverse to the end
		TLB *temp = *tlb_head;
		while(temp->next) {
			temp = temp->next;
		}
		temp->next = new_node;

		//If TLB exceeds the size limit, remove the oldest entry
		if (count >= TLB_SIZE) {
			TLB *oldest = *tlb_head;
			*tlb_head = (*tlb_head)->next;
			free(oldest);
		}
	}

	//Increment and return the count
	return count < TLB_SIZE ? count + 1 : count;
}

//Main function
int main() {
	int virtual_address;
	int TLB_count = 0;
	int num, i, temp, pnum, poff, val, next_free_frame = 0;
	FILE *fp, *bp, *fpp;
	char physicalMemory[256][256];
	int pageTable[256];
	TLB *ptr;

	fp = fopen("addresses.txt", "r");
	bp = fopen("BACKING_STORE.bin", "rb");

	initializePageTable(pageTable);

	if(fp != NULL && bp != NULL) {
		fpp = fopen("answers.txt", "w"); //File to store results
		fscanf(fp, "%d", &num); //Read number of virtual addresses

		for (i = 0; i < num; i++) {
			fscanf(fp, "%d", &virtual_address); //Read virtual address
			int fnum;
		
			pnum = virtual_address >> 8;  //Extract page number
			poff = virtual_address & 255; //Extract page offset

			fprintf(fpp, "Virtual address: %d, PageNum: %d, PageOff: %d, ", virtual_address, pnum, poff);

			//Check TLB for a hit
			temp = 0;
			ptr = tlb_head;
			while (ptr != NULL) {
				if (ptr->pageNum == pnum) {
					temp = 1; //TLB hit
					fnum = ptr->frameNumber;
					fprintf(fpp, "TLB HIT::");
					break;
				}
				ptr = ptr->next;
			}

			if (temp == 0) {  //TLB miss
				fprintf(fpp, "TLB MISS::");
				
				//Check the page table for the page
				if (pageTable[pnum] == -1) { //Page fault: not in physical memory
					fprintf(fpp, "PAGE NOT IN MEM, FAULT, LOAD FROM DISK, UPDATE PT::");

					fseek(bp, pnum * 256, SEEK_SET); //Seek to the page location in backing store
					
					//Load page from backing store into physical memory
					for (int j = 0; j < 256; j++) {
						fread(&physicalMemory[next_free_frame][j], sizeof(char), 1, bp);
					}
					
					pageTable[pnum] = next_free_frame; //Update page table
					
					//Update TLB with the new page-frame pair
					TLB_count = updateTLB(pnum, next_free_frame, TLB_count, &tlb_head);
					
					next_free_frame++; //Increment frame number for the next page
				} else {
					fprintf(fpp, "PAGE FOUND IN PT::");
				}

				//Update the frame number from the page table
				fnum = pageTable[pnum];
				
				//Update the TLB with the page-table result
				fprintf(fpp, "UPDATING TLB::");
				TLB_count = updateTLB(pnum, fnum, TLB_count, &tlb_head);
			}

			//Calculate the physical address and retrieve the value from physical memory
			val = physicalMemory[fnum][poff];
			int physical_address = (fnum << 8) + poff;
			fprintf(fpp, "Physical address: %d, Value: %d\n", physical_address, val);
		}

		fclose(fp);
		fclose(bp);
		fclose(fpp);
	}

	return 0;
}