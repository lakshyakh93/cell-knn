#ifndef SORTEDMAP_H_
#define SORTEDMAP_H_

typedef struct {
	int size;
	int length;
	double *keys;
	int *values;
} SortedMap;

SortedMap *createSortedMap(int length);
int insert(SortedMap *list, double key, int value);
void closeSortedMap(SortedMap *map);

#endif
