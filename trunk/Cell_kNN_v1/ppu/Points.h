#ifndef POINTS_H_
#define POINTS_H_

#include "Point.h"
#include "cellknn.h"
#include <stdio.h>
#include <malloc_align.h>
#include <free_align.h>

// L ... label type
// T ... value type
/**
* @brief Class representing a collection of images (Point)
*/
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

/**
* @brief Constructor for the Points class
*
* @param count Amount of elements
* @param dim Datafield dimension of each element
*
*/
template<class L, class T> Points<L, T>::Points(int count, int dim) {
	setCount(count);
	setDimension(dim);
	
	//-----------------------------------------------
	//----------CELL BE Stuff------------------------
	//-----------------------------------------------
	lsize = sizeof(L);
	if (lsize % ALIGNMOD)
		lsize = (static_cast<int>(lsize / ALIGNMOD) + 1) * ALIGNMOD;
	labels = (L *) _malloc_align(lsize * count, 7);
	lsize /= sizeof(L); //TODO !!!

	vsize = dim * sizeof(T);
	if (vsize % ALIGNMOD)
		vsize = (static_cast<int>(vsize / ALIGNMOD) + 1) * ALIGNMOD;
	values = (T *) _malloc_align(vsize * count, 7);
	vsize /= sizeof(T); //TODO !!!
	//-----------------------------------------------
	
}

template<class L, class T> Points<L, T>::~Points() {
	_free_align(labels);
	_free_align(values);
}

/**
* @brief Getter function to get the label
*
* @param n Position of the Element
*
* @return Label of element on n-th position
*/
template<class L, class T> L* Points<L, T>::getLabel(int n) {
	if (n < getCount())
		return (labels + n * getLSize());
	else
		return NULL;
}

/**
* @brief Setter function to set the label of the n-th element
*
* @param n Position of the element
* @param l Label to be set
*
*/
template<class L, class T> void Points<L, T>::setLabel(int n, L l) {
	if (n < getCount())
		*(labels + n * getLSize()) = l;
}

/**
* @brief Getter function to return a pointer to a datafield of an element in position n
*
* @param n Position of the element
*
* @return Pointer to the datafield
*/
template<class L, class T> T* Points<L, T>::getValues(int n) {
	if (n < getCount())
		return (values + n * getVSize());
	else
		return NULL;
}

/**
* @brief Setter function to set a Datafield of the n-th element
*
* @param n Position of the element
* @param values Datafield to be assigned to the element
*
*/
template<class L, class T> void Points<L, T>::setValues(int n, T *values) {
	if (n < getCount())
		memcpy(getValues(n), values, getDimension());
}

/**
* @brief Getter funtion to return a Pointer to Point
*
* @param n Position of the element
*
* @return Pointer to a Point-object
*/
template<class L, class T> Point<L, T>* Points<L, T>::getPoint(int n) {
	return new Point<L, T>(getDimension(), getValues(n), getLabel(n));
}

/**
* @brief Getter function to get the count of elements
*
* @return Count of the elements
*/
template<class L, class T> int Points<L, T>::getCount() {
	return count;
}

/**
* @brief Setter function to set the count of elements
*
* @param count Amount of elements
*
*/
template<class L, class T> void Points<L, T>::setCount(int count) {
	this->count = count;
}

/**
* @brief Getter function to get the Dimension of the datafields
*
* @return Dimension of the datafields (images)
*/
template<class L, class T> int Points<L, T>::getDimension() {
	return dimension;
}

/**
* @brief Setter function to set the dimension of the datafields
*
* @param dimension Dimension of the datafields
*
*/
template<class L, class T> void Points<L, T>::setDimension(int dimension) {
	this->dimension = dimension;
}

/** 
* @brief Getter function used to get the size of the data (pixel) (alligned to 16)
* 
* @return Size of the data (pixel)
*/
template<class L, class T> int Points<L, T>::getVSize() {
	return vsize;
}

/** 
* @brief Getter function used to get the data for one image (alligned to 16)
* 
* @return Size of the data (image)
*/
template<class L, class T> int Points<L, T>::getLSize() {
	return lsize;
}

#endif /* POINTS_H_ */
