#ifndef POINTS_H_
#define POINTS_H_

#include "Point.h"
#include <stdio.h>

// L ... label type
// T ... value type
template <class L, class T>
class Points {
	int count; //60000
	int dimension; //28*28

	L *labels; //unsigned char
	T *values; //int
public:
	Points(int count, int dim);
	virtual ~Points();

	L getLabel(int n);
	void setLabel(int n, L l);
	T *getValues(int n);
	void setValues(int n, T *point);
	Point<L, T>* getPoint(int n);
	
	int getCount();
	void setCount(int count);
	int getDimension();
	void setDimension(int dimension);
};

template<class L, class T>
Points<L, T>::Points(int count, int dim) {
	setCount(count);
	setDimension(dim);

	labels = new L[count];
	values = new T[count * dim];
}

template<class L, class T>
Points<L, T>::~Points() {
        delete[] labels;
        delete[] values;
}

template<class L, class T>
L Points<L, T>::getLabel(int n) {
	if (n < getCount())
		return labels[n];
	else
		return NULL;
}

template<class L, class T>
void Points<L, T>::setLabel(int n, L l) {
	if (n < getCount())
		labels[n] = l;
}

template<class L, class T>
T* Points<L, T>::getValues(int n) {
	if (n < getCount())
		return &values[n * this->getDimension()];
	else
		return NULL;
}

template<class L, class T>
void Points<L, T>::setValues(int n, T *point) {
	if (n < this->getDimension())
		memcpy(getPoint(n), point, this->getDimension());
}

template<class L, class T>
Point<L, T>* Points<L, T>::getPoint(int n) {
	return new Point<L, T>(this->getDimension(), &values[n * this->getDimension()], &labels[n]);
}

template<class L, class T>
int Points<L, T>::getCount() {
	return count;
}

template<class L, class T>
void Points<L, T>::setCount(int count){
	this->count = count;
}

template<class L, class T>
int Points<L, T>::getDimension(){
	return dimension;
}

template<class L, class T>
void Points<L, T>::setDimension(int dimension){
	this->dimension = dimension;
}

#endif /* POINTS_H_ */
