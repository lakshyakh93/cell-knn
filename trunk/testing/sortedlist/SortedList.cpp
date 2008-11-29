#include "SortedList.h"

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
