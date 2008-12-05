#ifndef POINTS_H_
#define POINTS_H_

#include "Point.h"
#include "cellknn.h"
#include <stdio.h>

// L ... label type
// T ... value type
template <class L, class T> class Points {
	int count; //60000
	int dimension; //28*28
	int lsize; //28*28*sizeof(int) + padding
	int vsize; //sizeof(int) + padding

	L *labels; //unsigned char
	T *values; //int
	
	int getVSize();
	int getLSize();
public:
	Points(int count, int dim);
	virtual ~Points();

	L *getLabel(int n);
	void setLabel(int n, L l);
	T *getValues(int n);
	void setValues(int n, T *point);
	Point<L, T>* getPoint(int n);

	int getCount();
	void setCount(int count);
	int getDimension();
	void setDimension(int dimension);
};

template<class L, class T> Points<L, T>::Points(int count, int dim) {
	setCount(count);
	setDimension(dim);

	//-----------------------------------------------
	//----------CELL BE Stuff------------------------
	//-----------------------------------------------
	lsize = sizeof(L);
	if (lsize % ALIGNMOD)
		lsize = (static_cast<int>(lsize / ALIGNMOD) + 1) * ALIGNMOD;
	labels = (L *) malloc_align(lsize * count, 7);

	vsize = dim * sizeof(T);
	if (vsize % ALIGNMOD)
		vsize = (static_cast<int>(vsize / ALIGNMOD) + 1) * ALIGNMOD;
	values = (T *) malloc_align(vsize * count, 7);
	//-----------------------------------------------
}

template<class L, class T> Points<L, T>::~Points() {
	free(labels);
	free(values);
}

template<class L, class T> L* Points<L, T>::getLabel(int n) {
	if (n < getCount())
		return (labels + n * getLSize());
	else
		return NULL;
}

template<class L, class T> void Points<L, T>::setLabel(int n, L l) {
	if (n < getCount())
		*(labels + n * getLSize()) = l;
}

template<class L, class T> T* Points<L, T>::getValues(int n) {
	if (n < getCount())
		return (values + n * getVSize());
	else
		return NULL;
}

template<class L, class T> void Points<L, T>::setValues(int n, T *values) {
	if (n < getCount())
		memcpy(getValues(n), values, getDimension());
}

template<class L, class T> Point<L, T>* Points<L, T>::getPoint(int n) {
	return new Point<L, T>(getDimension(), getValues(n), getLabel(n));
}

template<class L, class T> int Points<L, T>::getCount() {
	return count;
}

template<class L, class T> void Points<L, T>::setCount(int count) {
	this->count = count;
}

template<class L, class T> int Points<L, T>::getDimension() {
	return dimension;
}

template<class L, class T> void Points<L, T>::setDimension(int dimension) {
	this->dimension = dimension;
}

template<class L, class T> int Points<L, T>::getVSize() {
	return vsize;
}

template<class L, class T> int Points<L, T>::getLSize() {
	return lsize;
}

#endif /* POINTS_H_ */