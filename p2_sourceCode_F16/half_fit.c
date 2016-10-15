#include "half_fit.h"
#include <lpc17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "uart.h"
#include "type.h"

#define NULL 0
#define NULL_PTR (void*) 0
#define BYTE_SIZE 8
#define HEADER_SIZE 4
#define NUM_BUCKETS 11
#define CHUNK_SIZE 32
#define MAX_SIZE 32768
#define MAX_CHUNKS = MAX_SIZE / CHUNK_SIZE;

unsigned char array[MAX_SIZE]__attribute__((section(".ARM.__at_0x10000000"), zero_init));

#define SET_BUCKET_EMPTY(bucket_num) bitVector |= (1 << bucket_num);
#define SET_BUCKET_NON_EMPTY(bucket_num) bitVector &= ~(1 << bucket_num);
#define GET_POINTER(index) (&array[index])

typedef struct allocated_header {
	U32 prevBlock:10;
	U32 nextBlock:10;
	U32 blockSize:10;
	U32 isAllocated:1;
	U32 isFirst:1;
} allocated_header_t;

typedef struct unallocated_header {
	U64 prevBlock:10;
	U64 nextBlock:10;
	U64 blockSize:10;
	U64 isAllocated:1;
	U64 isFirst:1;
	U64 prevBucketBlock:10;
	U64 nextBucketBlock:10;	
} unallocated_header_t;


U16 bitVector;
unallocated_header_t* buckets[NUM_BUCKETS];

void  half_init(void){
	// Initialize bit vector with the 11th bucket not empty
	bitVector = (1 << 11);
	
	// Create one bucket, max size, unallocated
	unallocated_header_t* block = (unallocated_header_t*) get_chunk_pointer(0);

	block->prevBlock = 0;
	block->nextBlock = 0;
	block->blockSize = MAX_CHUNKS;
	block->isAllocated = __FALSE;
	
	block->prevBlock = 0;
	block->nextBlock = 0;
}
	
void* half_alloc(unsigned int size){
	U32 numChunks = (size + HEADER_SIZE) / CHUNK_SIZE; // can bit-shift by two later
	
	U8 minimumBucket = getBucketNum(numChunks);
	U8 availableBucket = getAvailableBucket(minimumBucket);
	
		
}

void  half_free(void * address){
	// Backtrack by 4 bytes to get to header location
	unallocated_header_t* block = (void *)((char *)(address) - HEADER_SIZE);

	// change the header data to deallocate the block
	block->isAllocated = __FALSE;
 
 	// Obtain the blocks before and after the freed block
	header_t* prevBlock = (header_t*)get_chunk_pointer(block->prevBlock)
	header_t* nextBlock = (header_t*)get_chunk_pointer(block->nextBlock);

	// coalesce blocks if possible
	if (prevBlock) {
		block = coalesce_blocks(block, prevBlock);
	}
	if (nextBlock) {
		block = coalesce_blocks(block, nextBlock);
	}

	// Place block in the appropriate bucket
	add_to_bucket(block);
}

void* get_chunk_pointer(U16 chunk_location) {
	// Verify that the chunk requested is within the bounds of the available space
	if (chunk_location > MAX_CHUNKS - 1) {
		return NULL;
	} 

	// Return the pointer to the appropriate location
	return array + (chunk_location * CHUNK_SIZE);
}

U16 get_chunk_location(void* chunk) {
	return ((U8*)chunk_p - array) / CHUNK_SIZE;
}

void add_to_bucket(header_t * block) {

	// Determine the bucket the block should belong to 
	S16 bucketNum = get_bucket_num();

	// Obtain the existing head of the linked list
	header_t * headBlock = buckets[bucketNum];

	// If previous linked list exists, attach the head to the front
	if (headBlock != NULL) {
		headBlock->prevBucketBlock = get_chunk_location(block);
		block->nextBucketBlock = get_chunk_location(headBlock);
	}

	// Point the bucket pointer to the newly unallocated block
	buckets[bucketNum] = block;
}

void remove_from_bucket(unallocated_header_t * block) {
	
	unallocated_header_t* prevBlockInBucket = get_chunk_pointer(block->prevBucketBlock);
	unallocated_header_t* nextBlockInBucket = get_chunk_pointer(block->nextBucketBlock);

	// Verify whether the next and previous blocks are actual values and not just zero(default)
	if (block != get_chunk_pointer(prevBlockInBucket->nextBucketBlock)) {
		prevBlockInBucket = NULL;
	}
	if (block != get_chunk_pointer(nextBlockInBucket->prevBucketBlock)) {
		nextBlockInBucket = NULL;
	}

	// If next block in the bucket exists, re-assign its previous pointer
	if (nextBlockInBucket != NULL) {
		nextBlockInBucket->prevBucketBlock = block->prevBucketBlock;
	} 
	
	// If prev block in the bucket exists, re-assign its next pointer;
	if (prevBlockInBucket != NULL) {
		prevBlockInBucket->nextBucketBlock = block->nextBucketBlock;
	}

	// If block is head of the bucket, make next block the head.
	if (prevBlockInBucket == NULL) {
		U8 bucketNum = get_bucket_num(block);
		bucket[bucketNum] = nextBlockInBucket;
	}
}

void * coalesce_blocks(header_t * firstBlock, header_t * secondBlock) {
	
	// Ensure the blocks exist
	if (firstBlock == NULL || second_block_p == NULL) {
		return NULL;
	}

	// Ensure blocks are unallocated 
	if (firstBlock->isAllocated == __TRUE || secondBlock->isAllocated == __TRUE) {
		return NULL;
	}

	// Initialize the unallocated block header
	firstBlock->nextBlock = secondBlock->nextBlock;
	firstBlock->blockSize += secondBlock->blockSize;

	return firstBlock;
}

S16 get_bucket_num(num_chunks) {
	U8 bucketNum = 0;
	U8 n = numChunks;
	while (num > 1){
		num = num >> 1;
		++bucketNum;
	}	
	return bucketNum;
}

/*
 * 	Function: getAvailableBucket
 * 	----------------------------
 *	Returns the square of the largest of its two input values
 * 	startIndex: the index of the smallest size bucket that can be used
 *	returns: the next available bucket after a start index
 */
S8 getAvailableBucket(U8 startIndex){
	
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