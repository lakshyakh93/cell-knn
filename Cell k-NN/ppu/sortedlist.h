#ifndef SORTEDLIST_H_
#define SORTEDLIST_H_

#include <knn.h>

typedef struct {
	Point *point;
	double distance;
} ListItem;

typedef struct {
	int size;
	int length;
	ListItem **values;
} SortedList;

SortedList *createSortedList(int length);
int insert(SortedList *list, Point *point, double distance);
void closeSortedList(SortedList *list);

#endif
