#ifndef CELLKNN_H_
#define CELLKNN_H_

#define ALIGNMOD 16

typedef struct {
	// Number of elements in the query and reference arrays.
	int count __attribute__((aligned(ALIGNMOD)));

	// Array of integers representing the vector of the query point.
	int *testPoint __attribute__((aligned(ALIGNMOD)));

	// Array of integers representing the vector of the reference point.
	int *trainPoint __attribute__((aligned(ALIGNMOD)));

	// Pointer to the distance of the query vector to the reference vector.
	// The pointer is necessary for DMA transfers from SPE to PPE.
	double *distance __attribute__((aligned(ALIGNMOD)));
} Parameters;

#endif /*CELLKNN_H_*/
