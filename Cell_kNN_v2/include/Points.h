#ifndef POINTS_H_
#define POINTS_H_

#include "Point.h"
#include "cellknn.h"
#include <stdio.h>

// L ... label type
// T ... value type
/**
* @brief Class representing a collection of images (Point)
*/
template <class L, class T> class Points {
	int deleteValues;
	int count; //60000
	int dimension; //28*28
    int lsize; //sizeof(L) + padding
    int vsize; //28*28*sizeof(T) + padding

	char *labels; //unsigned char
	char *values; //int
	
	// TODO check offset calculations
public:
	Points(int count, int dim);
	Points(int count, int dim, char *labels, char* values);
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

	int getVSize();
	int getLSize();
};

/**
* @brief Constructor for the Points class
*
* @param count Amount of elements
* @param dim Datafield dimension of each element
*
*/
template<class L, class T> Points<L, T>::Points(int count, int dim) {
	deleteValues = 1;
	setCount(count);
	setDimension(dim);
	
	//-----------------------------------------------
	//----------CELL BE Stuff------------------------
	//-----------------------------------------------
	lsize = sizeof(L);
	if (lsize % ALIGNMOD)
		lsize = (static_cast<int>(lsize / ALIGNMOD) + 1) * ALIGNMOD;
#ifdef PPU
	labels = (char *) _malloc_align(lsize * count, 7);
#endif
#ifdef SPU
	labels = (char *) malloc_align(lsize * count, 7);
#endif

	vsize = dim * sizeof(T);
	if (vsize % ALIGNMOD)
		vsize = (static_cast<int>(vsize / ALIGNMOD) + 1) * ALIGNMOD;
#ifdef PPU
	values = (char *) _malloc_align(vsize * count, 7);
#endif
#ifdef SPU
	values = (char *) malloc_align(vsize * count, 7);
#endif
	//-----------------------------------------------
	
}

/**
* @brief Constructor for the Points class.
* Does not allocate any space, but sets references to existing arrays.
*/
template<class L, class T> Points<L, T>::Points(int count, int dim, char *labels, char* values) {
	deleteValues = 0;
	setCount(count);
	setDimension(dim);
	
	//-----------------------------------------------
	//----------CELL BE Stuff------------------------
	//-----------------------------------------------
	lsize = sizeof(L);
	if (lsize % ALIGNMOD)
		lsize = (static_cast<int>(lsize / ALIGNMOD) + 1) * ALIGNMOD;
	this->labels = labels;// TODO check

	vsize = dim * sizeof(T);
	if (vsize % ALIGNMOD)
		vsize = (static_cast<int>(vsize / ALIGNMOD) + 1) * ALIGNMOD;
	this->values = values;// TODO check
	//-----------------------------------------------
	
}

template<class L, class T> Points<L, T>::~Points() {
	if (deleteValues) {
#ifdef PPU
		_free_align(labels);
		_free_align(values);
#endif
#ifdef SPU
		free_align(labels);
		free_align(values);
#endif
	}
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
		return (L*) (labels + n * getLSize());
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
		return (T*) (values + n * getVSize());
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
* @brief Getter function used to get the size of the data (image) (alligned to 16)
* 
* @return Size of the data (image)
*/
template<class L, class T> int Points<L, T>::getVSize() {
	return vsize;
}

/** 
* @brief Getter function used to get the data for one label (alligned to 16)
* 
* @return Size of the data (label)
*/
template<class L, class T> int Points<L, T>::getLSize() {
	return lsize;
}

#endif /* POINTS_H_ */
