#ifndef KNN_H_
#define KNN_H_

typedef struct {
	int count;
	int *query;
	int *reference;
	double distance;
} Parameters;

typedef struct {
	int label;
	int dimensions;
	int *vector;
} Point;

//double distance(Point *query, Point *reference, int dimension);
//void classify(int k, Point *query, Point **training, int length, int dimension);

#endif
