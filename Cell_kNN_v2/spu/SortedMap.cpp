#include <stdio.h>
#include <stdlib.h>

#include "libmisc.h";
#include "SortedMap.h"

SortedMap *createSortedMap(int length) {
	// Create sorted list of length "length".
	SortedMap *map = (SortedMap *) malloc_align(sizeof(SortedMap), 7);

	map->size = 0;
	map->length = length;
	map->keys = (double *) malloc_align(map->length * sizeof(double), 7);
	map->values = (int *) malloc_align(map->length * sizeof(int), 7);

	return map;
}

void closeSortedMap(SortedMap *map) {
	free_align(map->keys);
	free_align(map->values);
	free_align(map);
}

int insert(SortedMap *map, double key, int value) {
	int i;
	int index = -1;

	// insert from tail (most items won't be inserted)
	for (i = map->size; i > 0; i--) {
		if (key >= map->keys[i - 1]) {
			break;
		}
		
		index = --i;
	}

	// if list is full
	if (map->size >= map->length) {
		// return "-1" if element is not inserted
		if (index < 0) {
			return index;
		}
	} else {
		// insert element at end of list
		if (index < 0) {
			index = map->size;
		}
		// increase list size
		++map->size;
	}

	// shift elements
	for (i = (map->size - 1); i > index; --i) {
		map->values[i] = map->values[i - 1];
		map->keys[i] = map->keys[i - 1];
	}

	map->keys[index] = key;
	map->values[index] = value;

	return index;
}
