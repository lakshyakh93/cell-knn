#ifndef KNN_H_
#define KNN_H_

typedef struct {
	// Number of elements in the query and reference arrays.
	int count __attribute__((aligned(16)));

	// Array of integers representing the vector of the query point.
	int *query __attribute__((aligned(16)));

	// Array of integers representing the vector of the reference point.
	int *reference __attribute__((aligned(16)));

	// Pointer to the distance of the query vector to the reference vector.
	// The pointer is necessary for DMA transfers from SPE to PPE.
	double *distance __attribute__((aligned(16)));
} Parameters;

typedef struct {
	int label;
	int dimensions;
	int *vector;
} Point;

void classify(int k, Point *query, Point **training, int length);

#endif
