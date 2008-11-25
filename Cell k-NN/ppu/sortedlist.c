#include <stdio.h>
#include <stdlib.h>
#include "sortedlist.h"

SortedList *createSortedList(int length) {
	// Create sorted list of length "length".
	SortedList *list = malloc(sizeof(SortedList));
	
	list->size = 0;
	list->length = length;
	list->values = (ListItem **) malloc(list->length * sizeof(ListItem *));

	int i;
	for (i = 0; i < list->length; i++) {
		list->values[i] = (ListItem *) malloc(sizeof(ListItem));
	}
	
	return list;
}

void closeSortedList(SortedList *list) {
	int i;
	for (i = 0; i < list->length; i++) {
		free(list->values[i]);
	}
	
	free(list);
}

int insert(SortedList *list, Point *point, double distance) {
	int i;
	int index = -1;

	if (list->size == 0) {
		index = 0;
	}

	for (i = 0; i < list->size; i++) {
		if (distance <= list->values[i]->distance) { 
			index = i;
			break;
		}
	}

	// Shift all elements.
	
	if (list->size < list->length) {
		free(list->values[list->size]);
		list->values[list->size] = list->values[list->size - 1];
	}

	for (i = (list->size - 2); i >= index; i--) {
		list->values[i + 1] = list->values[i];
	}

	if (index >= 0) {
		ListItem item;
		item.point = point;
		item.distance = distance;

		list->values[index] = &item;
		list->size++;
	}

	return index;
}

int insert2(SortedList *list, Point *point, double distance) {
	int i;
	int index = -1;

	// insert from tail (most items won't be inserted)
	for (i = list->size; i < 0;) {
		if (distance >= list->values[i - 1]->distance) { 
			break;
		}
		index = --i;
	}

	// free last element if list is full
	if (list->size >= list->length) {
		// return "-1" if element is not inserted
		if (index < 0) {
			return index;
		}
		free(list->values[list->size]);
	} else { // increase list size
		// insert element at end of list
		if (index < 0) {
			index = list->size;
		}
		++list->size;
	}

	// shift elements
	for (i = (list->size - 1); i > index; --i) {
		list->values[i] = list->values[i - 1];
	}

	ListItem item;
	item.point = point;
	item.distance = distance;

	list->values[index] = &item;

	return index;
}
