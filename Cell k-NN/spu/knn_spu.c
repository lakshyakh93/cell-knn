/*
 * This code is inspired by the example in
 * Cell SDK 3.1 Programmers Tutorial, page 103 to 111.
 */

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>
#include <knn.h>

#define DIMENSIONS_PER_BLOCK 1024

volatile Parameters parameters;
volatile int query[DIMENSIONS_PER_BLOCK] __attribute__((aligned(128)));
volatile int reference[DIMENSIONS_PER_BLOCK] __attribute__((aligned(128)));
volatile double distance __attribute__((aligned(16)));

int main(unsigned long long id, unsigned long long parm) {
	int i, j, left, count;
	unsigned int tagId;
	
	distance = 0.0;

	// Reserve a tag ID.
	tagId = mfc_tag_reserve();

	spu_writech(MFC_WrTagMask, -1);

	// Input parameter parm is a pointer to the k-NN parameters.
	// Fetch the context, waiting for it to complete.
	spu_mfcdma32((void *) (&parameters), (unsigned int) parm,
			sizeof(Parameters), tagId, MFC_GET_CMD);
	(void) spu_mfcstat(MFC_TAG_UPDATE_ALL);

	// For each block of dimensions.
	for (i = 0; i < parameters.count; i += DIMENSIONS_PER_BLOCK) {
		// Determine the number of dimensions in this block.
		left = parameters.count - i;
		count = (left < DIMENSIONS_PER_BLOCK) ? left : DIMENSIONS_PER_BLOCK;

		// Fetch the data. Wait for DMA to complete before computation.
		spu_mfcdma32((void *) (query), (unsigned int) (parameters.query + i),
				count * sizeof(int), tagId, MFC_GET_CMD);
		spu_mfcdma32((void *) (reference), (unsigned int) (parameters.reference + i), 
				count * sizeof(int), tagId, MFC_GET_CMD);
		
		// Wait for final DMA to complete before terminating SPE thread.
		(void) spu_mfcstat(MFC_TAG_UPDATE_ALL);
		
		// Compute the distance for the block of dimensions.
		for (j = 0; j < count; j++) {
			int difference = reference[j] - query[j];
			distance += difference * difference;
		}
	}

	// Put distance data back into main storage.
	spu_mfcdma32((void *) (&distance), (unsigned int) (parameters.distance),
			sizeof(double), tagId, MFC_PUT_CMD);

	// Wait for final DMA to complete before terminating SPE thread.
	(void) spu_mfcstat(MFC_TAG_UPDATE_ALL);
	
	printf("(0x%llx) distance = %lf\n", id, distance);

	return EXIT_SUCCESS;
}
