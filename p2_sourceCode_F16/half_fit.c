#include "half_fit.h"
#include <lpc17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "uart.h"
#include "type.h"

#define NULL 0
#define NULL_PTR (void*) 0
#define BYTE 8
#define HEADER_SIZE 4
#define NUM_BUCKETS 11
#define CHUNK_SIZE 32
#define MAX_SIZE 32768

unsigned char array[MAX_SIZE]__attribute__((section(".ARM.__at_0x10000000"), zero_init));

typedef struct header {
	U32 prevBlock:10;
	U32 nextBlock:10;
	U32 blockSize:10;
	U32 isAllocated:1;
	U32 isFirstOrLast:1;
	U32 prevBucketBlock:10;
	U32 nextBucketBlock:10;	
} header_t;

typedef struct block {
	header_t header;
} block_t;

U16 bitVector;
header_t* first = (header_t*)(array);

void  half_init(void){
	// Initialize bit vector with the 11th bucket not empty
	bitVector = (1 << 11);
	
	// Create one bucket, max size, unallocated
	first->prevBlock = NULL_PTR;
	first->nextBlock = NULL_PTR;
	first->isAllocated = false;
	first->isFirstOrLast = true;
}
	
void *half_alloc(unsigned int size){
	U32 numChunks = (size + HEADER_SIZE) / CHUNK_SIZE; // can bit-shift by two later
	
	U8 minimumBucket = getBucketNum(numChunks);
	U8 availableBucket = getAvailableBucket(minimumBucket);
	
		
}

void  half_free(void * address){

}

U8 getBucketNum(numChunks) {
	U8 bucketNum = 0;
	U8 n = numChunks;
	while (num > 1){
		num = num >> 1;
		++bucketNum;
	}	
	return bucketNum + 1;
}

/*
 * 	Function: getAvailableBucket
 * 	----------------------------
 *	Returns the square of the largest of its two input values
 * 	startIndex: the index of the smallest size bucket that can be used
 *	returns: the next available bucket after a start index
 */
S8 getAvailableBucket(startIndex){
	
	// Check after the given bucket
	U8 bucketIndex = startIndex;
    	
	// Check bit vector until non-empty bucket is found
	while (bitVector & (1 << bucketNum) == 0) {
		
		// Move to check the next bucket
		++bucketNum;
		
		// If the end of the bit vector was reached, nothing is available
		if (bucketNum >= NUM_BUCKETS) {
			return -1;
		}
	}
	return bucketNum; 
}