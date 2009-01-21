#ifndef SORTEDMAP_H_
#define SORTEDMAP_H_

typedef struct {
	int size;
	int length;
	float *keys;
	unsigned char *values;
} SortedMap;

SortedMap *createSortedMap(int length);
int insert(SortedMap *list, float key, unsigned char value);
void closeSortedMap(SortedMap *map);

#endif
