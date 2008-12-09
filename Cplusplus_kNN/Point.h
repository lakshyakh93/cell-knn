#ifndef POINT_H_
#define POINT_H_

// L ... label type
// T ... value type
template <class L, class T> class Point {
	int dimension;
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
	label = new L;
	values = new T[dim];
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
		delete label;
                delete[] values;
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
