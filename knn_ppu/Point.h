#ifndef POINT_H_
#define POINT_H_

#include <malloc_align.h>
#include <free_align.h>

// L ... label type
// T ... value type
template <class L, class T> class Point {
	int dimension __attribute__((aligned(16)));
	int deleteValues;
	
	L *label;
	T *values;
public:
	Point(int dimension);
	Point(int dimension, T *values, L *label);
	virtual ~Point();

	int getDimension();
	void setDimension(int dimension);
	L getLabel();
	void setLabel(L label);
	void setLabel(L *label);
	T *getValues();
	void setValues(T* vector);
};

template <class L, class T> 
Point<L,T>::Point (int dim) {
	deleteValues = 1;
	setDimension(dim);
	label = (L *) _malloc_align(sizeof(L), 4);
	values = (T *) _malloc_align(dim * sizeof(T), 4);
}

template <class L, class T> 
Point<L,T>::Point (int dim, T *v, L *l) {
	deleteValues = 0;
	setDimension(dim);
	setLabel(l);
	setValues(v);
}

template <class L, class T> 
Point<L,T>::~Point () {
	if (deleteValues) {
		_free_align(label);
		_free_align(values);
	} else {
		label = NULL;
		values = NULL;
	}
}


template <class L, class T> 
int Point<L,T>::getDimension() {
	return this->dimension;
}

template <class L, class T> 
void Point<L,T>::setDimension(int dim) {
	this->dimension = dim;
}

template <class L, class T> 
L Point<L,T>::getLabel() {
	return *label;
}

template <class L, class T> 
void Point<L,T>::setLabel (L l) {
	*label = l;
}

template <class L, class T> 
void Point<L,T>::setLabel (L *l) {
	label = l;
}

template <class L, class T> 
T *Point<L,T>::getValues() {
	return values;
}

template <class L, class T> 
void Point<L,T>::setValues (T *v) {
	values = v;
}

#endif /*POINT_H_*/
