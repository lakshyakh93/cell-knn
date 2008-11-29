#ifndef SORTEDLIST_H_
#define SORTEDLIST_H_

#include <cstdlib>
#include <iostream>
#include <iterator>

template <class T> class SortedList {
	struct node {
		int key;
		T value;
		node *next;
		node(int k, T v, node* n) :
			key(k), value(v), next(n) {
		}
		node(int k, T v) :
			key(k), value(v), next(NULL) {
		}
	};
	int maxSize;
	int currSize;
	node *head;
public:
	SortedList(int m) :
		maxSize(m), currSize(0), head(NULL) {
	}
	virtual ~SortedList();
	int getMaxSize();
	int getCurrSize();
	bool isEmpty();
	bool isFull();
	int insert(int key, T value);
	void print();

	class Iterator : public std::iterator<std::forward_iterator_tag, T> {
private:
		node* nd;
public:
		Iterator(node* p) :
			nd(p) {
		}
		~Iterator() {
		}

		Iterator& operator=(const Iterator& other) {
			nd = other.nd;
			return (*this);
		}

		bool operator==(const Iterator& other) {
			return (nd == other.nd);
		}

		bool operator!=(const Iterator& other) {
			return (nd != other.nd);
		}

		Iterator& operator++() {
			if (nd != NULL) {
				nd = nd->next;
			}
			return (*this);
		}

		Iterator& operator++(int) {
			Iterator tmp(*this);
			++(*this);
			return (tmp);
		}

		// Return by reference not value (like most container classes do) 
		node operator*() {
			return *nd;
		}

		node* operator->() {
			return nd;
		}

	};
	Iterator begin() {
		return (Iterator(head));
	}

	Iterator end() {
		return (Iterator(NULL));
	}

};

#endif /*SORTEDLIST_H_*/
