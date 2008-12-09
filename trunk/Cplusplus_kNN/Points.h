#ifndef POINTS_H_
#define POINTS_H_

#include "Point.h"
#include <stdio.h>

// L ... label type
// T ... value type
template <class L, class T>
/** 
* @brief Class representing a collection of images (Points)
*/
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

/**
* @brief Constructor for the Points class
*
* @param count Amount of elements
* @param dim Datafield dimension of each element
*
*/
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

/**
* @brief Getter function to get the label
*
* @param n Position of the Element
*
* @return Label of element on n-th position
*/
template<class L, class T>
L Points<L, T>::getLabel(int n) {
	if (n < getCount())
		return labels[n];
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
template<class L, class T>
void Points<L, T>::setLabel(int n, L l) {
	if (n < getCount())
		labels[n] = l;
}

/**
* @brief Getter function to return a pointer to a datafield of an element in position n
*
* @param n Position of the element
*
* @return Pointer to the datafield
*/
template<class L, class T>
T* Points<L, T>::getValues(int n) {
	if (n < getCount())
		return &values[n * this->getDimension()];
	else
		return NULL;
}

/**
* @brief Setter function to set a Datafield of the n-th element
*
* @param n Position of the element
* @param point Datafield to be assigned to the element
*
*/
template<class L, class T>
void Points<L, T>::setValues(int n, T *point) {
	if (n < this->getDimension())
		memcpy(getPoint(n), point, this->getDimension());
}

/**
* @brief Getter funtion to return a Pointer to Point
*
* @param n Position of the element
*
* @return Pointer to a Point-object
*/
template<class L, class T>
Point<L, T>* Points<L, T>::getPoint(int n) {
	return new Point<L, T>(this->getDimension(), &values[n * this->getDimension()], &labels[n]);
}

/**
* @brief Getter function to get the count of elements
*
* @return Count of the elements
*/
template<class L, class T>
int Points<L, T>::getCount() {
	return count;
}

/**
* @brief Setter function to set the count of elements
*
* @param count Amount of elements
*
*/
template<class L, class T>
void Points<L, T>::setCount(int count){
	this->count = count;
}

/**
* @brief Getter function to get the Dimension of the datafields
*
* @return Dimension of the datafields (images)
*/
template<class L, class T>
int Points<L, T>::getDimension(){
	return dimension;
}

/**
* @brief Setter function to set the dimension of the datafields
*
* @param dimension Dimension of the datafields
*
*/
template<class L, class T>
void Points<L, T>::setDimension(int dimension){
	this->dimension = dimension;
}

#endif /* POINTS_H_ */
