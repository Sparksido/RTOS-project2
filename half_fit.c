#include "half_fit.h"
#include <lpc17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "uart.h"
#include "type.h"

#define NULL 0
#define NULL_PTR 0 // *****iffy
#define BYTE 8
#define HEADER_SIZE 4
#define NUM_BUCKETS 11
#define CHUNK_SIZE 32
#define MAX_SIZE 32768

#define SET_BIT(bitNum) bitVector |= (1 << bitNum)
#define UNSET_BIT(bitNum) bitVector &= ~(1 << bitNum)


typedef struct header {
	U32 prevBlock:10;
	U32 nextBlock:10;
	U32 blockSize:10;
	U32 isAllocated:1;
	U32 isFirstOrLast:1;
	U32 prevBucketBlock:10;
	U32 nextBucketBlock:10;	
} header_t;

unsigned char array[MAX_SIZE]__attribute__((section(".ARM.__at_0x10000000"), zero_init));
header_t *mempoint[11];

typedef struct block {
	header_t header;
} block_t;

U16 bitVector;
header_t* first = (header_t*)(&array);

// first helper function
U8 getBucketNum(U16 numChunks) {
	U8 bucketNum = 0;
	U8 num = numChunks;
	while (num > 1){
		num = num >> 1;
		++bucketNum;
	}	
	return bucketNum;
}

// second helper function
U8 getAvailableBucket(bucketNum){
	
	// Check after the given bucket
	U8 bucketIndex = bucketNum + 1;
	bucketNum++;// need to check the next bucket
    	
	// Check bit vector until non-empty bucket is found
	while (bitVector & (1 << buketNum) == 0){
		
		// Move to check the next bucket
		++bucketNum;
		
		// If the end of the bit vector was reached, nothing is available
		if (bucketNum >= NUM_BUCKETS) {
			return -1;
		}
	}
	return bucketNum; 
}

void  half_init(void){
	// Initialize bit vector with the 11th bucket not empty
	bitVector = (1 << 11);
	mempoint[10] = (header_t *)array;
	
	// Create one bucket, max size, unallocated
	first->prevBlock = first->prevBlock;  // ****ifffy
	first->nextBlock = first->prevBlock;  // ****ifffy
	first->blockSize = 1024;
	first->isAllocated = false;
	first->isFirstOrLast = true;
}
	
void *half_alloc(unsigned int size){
	
	//if there is a reminder, add 1, otherwise round down
	U8 minimumBucket, availableBucket, bucketNum;
	U16 frontBlock , remainingSize;
	U32 numChunks;
	U8* newBlockLocation, headerLocation;
	header_t* block, newBlock, bucket;
	
	if (((size + HEADER_SIZE) % CHUNK_SIZE) == 0){
		U32 numChunks = (size + HEADER_SIZE) / CHUNK_SIZE; // can bit-shift by two later
	}else{
		U32 numChunks = (size + HEADER_SIZE) / CHUNK_SIZE; // can bit-shift by two later
	}
	
	minimumBucket = getBucketNum(numChunks);
	//check if bucket is available and keep moving forward until one is found
	availableBucket = getAvailableBucket(minimumBucket);
	
	//if there isn't an available bucket 
	if(availableBucket == -1){
		return NULL;
	}
	
	//fetch the free memory block in that location
	block = mempoint[availableBucket];
	//store the location of block that follows the current block
	frontBlock = block->nextBlock;
	
	//what's the size of the remaining block after allocation
	remainingSize = block->blockSize - numChunks;
	block->isAllocated = true;
	block->nextBlock -= (block->blockSize) - numChunks; //should this perhaps be: blocksize - (numchunks*CHUNKSIZE) 
	block->blockSize =  numChunks;
	
	//compute the location of the new block
	newBlockLocation = (U8*) block;
	newBlockLocation += numChunks * CHUNK_SIZE;
	
	//assign values to the new block's headers
	newBlock = (header_t*) newBlockLocation;
	newBlock->isAllocated = false;
	newBlock->blockSize = remainingSize;
	newBlock->nextBlock = frontBlock;
	
	// Place new header into appropriate bucket
	bucketNum = getBucketNum(newBlock->blockSize);
	
	bucket = mempoint[bucketNum];
	
	//connecting the new block in the appropriate bucket
	if (bucket != NULL){
		newBlock->nextBucketBlock = bucket;
		bucket->prevBucketBlock = newBlock;
	}
	mempoint[bucketNum] = newBlock;
	SET_BIT(bucketNum);
	
	//computing the location of the memory being allocated and returning it
	headerLocation = (U8*)block;
	void* allocatedMemory = (void*)(headerLocation + HEADER_SIZE);
	return allocatedMemory;
}


void  half_free(void * address){

}

/*
 * 	Function: getAvailableBucket
 * 	----------------------------
 *	Returns the square of the largest of its two input values
 * 	startIndex: the index of the smallest size bucket that can be used
 *	returns: the next available bucket after a start index
 */

