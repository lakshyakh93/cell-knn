#ifndef SORTEDLIST_H_
#define SORTEDLIST_H_

#include <cstdlib>
#include <iostream>

template <class T> 
class SortedList
{
	struct node {
		int key;
		T value;
		node *next;
		node(int k, T v, node* n) : key(k), value(v), next(n) {}
		node(int k, T v) : key(k), value(v), next(NULL) {}
	};
	int maxSize;
	int currSize;
	node *head;
public:
	SortedList(int m) : maxSize(m), currSize(0), head(NULL) {}
	virtual ~SortedList();
	int getMaxSize();
	int getCurrSize();
	bool isEmpty();
	bool isFull();
	int insert(int key, T value);
	void print();
};

#endif /*SORTEDLIST_H_*/
