#ifndef KNN_H_
#define KNN_H_

typedef struct {
	int count __attribute__ ((aligned (16)));
	int *query __attribute__ ((aligned (16)));
	int *reference __attribute__ ((aligned (16)));
	double *distance __attribute__ ((aligned (16)));
} Parameters;

typedef struct {
	int label __attribute__ ((aligned (16)));
	int dimensions __attribute__ ((aligned (16)));
	int *vector __attribute__ ((aligned (16)));
} Point;

//double distance(Point *query, Point *reference, int dimension);
//void classify(int k, Point *query, Point **training, int length, int dimension);

#endif
