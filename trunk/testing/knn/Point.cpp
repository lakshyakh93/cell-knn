#include "Point.h"

template <class L, class T> 
Point<L,T>::Point (L l, std::vector<T> v) {
	label = l;
	values = v;
}

template <class L, class T> 
Point<L,T>::Point (std::vector<T> v) {
	this(NULL, v);
}

template <class L, class T> 
Point<L,T>::~Point () {
	delete values;
	delete label;
}

template <class L, class T> 
L Point<L,T>::getLabel() {
	return label;
}

template <class L, class T> 
void Point<L,T>::setLabel (L l) {
	label = l;
}

template <class L, class T> 
std::vector<T> Point<L,T>::getValues() {
	return values;
}
