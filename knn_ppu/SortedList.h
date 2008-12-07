#ifndef SORTEDLIST_H_
#define SORTEDLIST_H_

#include <cstdlib>
#include <iostream>
#include <iterator>

template <class K, class V> class SortedList {
	struct node {
		K key;
		V value;
		node *next;
		node(K k, V v, node* n) :
			key(k), value(v), next(n) {
		}
		node(K k, V v) :
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
	int insert(K key, V value);
	void print();

	class Iterator : public std::iterator<std::forward_iterator_tag, K, V> {
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

template <class K, class V> 
SortedList<K, V>::~SortedList()
{
	node *temp;
	
	while(head) {
		temp = head;
		head = head->next;
		delete temp;
	}
}

template <class K, class V> 
int SortedList<K, V>::getMaxSize() {
	return maxSize;
}

template <class K, class V> 
int SortedList<K, V>::getCurrSize() {
	return currSize;
}

template <class K, class V> 
bool SortedList<K, V>::isEmpty() {
	return getCurrSize() == 0;
}

template <class K, class V> 
bool SortedList<K, V>::isFull() {
	return getCurrSize() >= getMaxSize();
}

template <class K, class V> 
void SortedList<K, V>::print() {
	node *temp = head;
	
	while(temp) {
		std::cout << temp->value << " (key=" << temp->key << ")\t";
		temp = temp->next;
	}
	std::cout << "\n";
}

template <class K, class V> 
int SortedList<K, V>::insert(K key, V value) {
	node *bigger, *smaller;
	
	if (isFull()) {
		if (key >= head->key)
			return 0;
		
		--currSize;
		bigger = head;
		head = head->next;
		delete bigger;
		return insert(key, value);
	}
	
	if (isEmpty()) {
		head = new node(key, value);
	} else {
		bigger = smaller = head;
		
		while (smaller && key < smaller->key) {
			bigger = smaller;
			smaller = smaller->next;
		}
		if (smaller != head)		
			bigger->next = new node(key, value, smaller);
		else
			head = new node(key, value, smaller);
	}
	
	return ++currSize;;
}


// **********************
// *main fct for testing*
// **********************
/*
int main() {
	SortedList<char> mylist(5);
	mylist.insert(5, '5');
	mylist.print();
	mylist.insert(6, '5');
	mylist.print();
	mylist.insert(3, '5');
	mylist.print();
	mylist.insert(5, '5');
	mylist.print();
	mylist.insert(8, '5');
	mylist.print();
	mylist.insert(8, '5');
	mylist.print();
	mylist.insert(8, '5');
	mylist.print();
	mylist.insert(1, '5');
	mylist.print();
	mylist.insert(2, '5');
	mylist.print();
	
	for (SortedList<char>::Iterator it = mylist.begin(); it != mylist.end(); ++it) {
		std::cout << it->key << ": " << (*it).value << "\t";
	}
	std::cout << "\n";
}
*/

#endif /*SORTEDLIST_H_*/
