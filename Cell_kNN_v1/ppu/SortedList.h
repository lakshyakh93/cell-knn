#ifndef SORTEDLIST_H_
#define SORTEDLIST_H_

#include <cstdlib>
#include <iostream>
#include <iterator>

/**
* @brief SortedList class used to organice the distances of Images with the according labels. \n
*       Elements with smaller keys (distances) are always inserted, elements with greater keys not always, depending of the current size and the maximum size of the list.
*/
template <class K, class V> class SortedList {
/**
* @brief Node for the LinkedList with key and value
*/
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
/**
* @brief Constructor with the maximal size of the list as argument
*
* @param m Maximal size of the list
*/
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
/**
* @brief Iterator to iterate over the list
*/
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

/**
* @brief Getter function to get the maximum size of the list
*
* @return Maximum size of the list
*/
template <class K, class V> 
int SortedList<K, V>::getMaxSize() {
	return maxSize;
}

/**
* @brief Getter function to get the current size of the list
*
* @return Current size of the list
*/
template <class K, class V> 
int SortedList<K, V>::getCurrSize() {
	return currSize;
}

/**
* @brief Function to check whether the list is empty
*
* @return true if the list ist empty \n
*         false else
*/
template <class K, class V> 
bool SortedList<K, V>::isEmpty() {
	return getCurrSize() == 0;
}

/**
* @brief Function to check whether the list is full
*
* @return true if the list is full \n
*         false else
*/
template <class K, class V> 
bool SortedList<K, V>::isFull() {
	return getCurrSize() >= getMaxSize();
}

/**
* @brief Function to print the values and keys of the list to the std::out
*
*/
template <class K, class V> 
void SortedList<K, V>::print() {
	node *temp = head;
	
	while(temp) {
		std::cout << temp->value << " (key=" << temp->key << ")\t";
		temp = temp->next;
	}
	std::cout << "\n";
}

/**
* @brief Function to insert an Element to the list depending of the key value. The element is only inserted, if the key is smaller than the biggest in the list or the list isn't full.
*
* @param key Key of the element
* @param value Value of the element
*
* @return 0 if the element wasn't inserted ( key too high ) \n
*       else the current size of the list
*/
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
