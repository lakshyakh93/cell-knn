#include <stdio.h>
#include <stdlib.h>
#include "sortedlist.h"

SortedList *createSortedList(int length) {
	// Create sorted list of length "length".
	SortedList *list = new SortedList();

	list->size = 0;
	list->length = length;
	list->values = (ListItem **) malloc(list->length * sizeof(ListItem *));

	return list;
}

void closeSortedList(SortedList *list) {
	int i;
	for (i = 0; i < list->size; i++) {
		free(list->values[i]);
	}

	delete list;
}

int insert(SortedList *list, Point *point, double distance) {
	int i;
	int index = -1;

	// insert from tail (most items won't be inserted)
	for (i = list->size; i > 0;) {
		if (distance >= list->values[i - 1]->distance) {
			break;
		}
		index = --i;
	}

	// if list is full
	if (list->size >= list->length) {
		// return "-1" if element is not inserted
		if (index < 0) {
			return index;
		}
		// or free last element
		free(list->values[list->size - 1]);
	} else {
		// insert element at end of list
		if (index < 0) {
			index = list->size;
		}
		// increase list size
		++list->size;
	}

	// shift elements
	for (i = (list->size - 1); i > index; --i) {
		list->values[i] = list->values[i - 1];
	}

	list->values[index] = (ListItem *) malloc(sizeof(ListItem));
	list->values[index]->point = point;
	list->values[index]->distance = distance;

	return index;
}