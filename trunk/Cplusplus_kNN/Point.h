#ifndef POINT_H_
#define POINT_H_

// L ... label type
// T ... value type
/** 
* @brief Class Point used to represent an image with label and data-field
*/
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

/**
* @brief Standardconstructor with the (image)dimesion as argument
*
* @param dim Dimension of the values (amount of pixels in an image)
*/
template <class L, class T> 
Point<L,T>::Point (int dim) {
	deleteValues = 1;
	setDimension(dim);
	label = new L;
	values = new T[dim];
}

/**
* @brief Constructor to set dimension, the data and the label
*
* @param dim Dimension of the datafield
* @param v Pointer to the datafield
* @param l Label to be set
*/
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

/**
* @brief Getter function to return the dimension of the datafield
*
* @return Dimension of the datafield
*/
template <class L, class T> 
int Point<L,T>::getDimension() {
	return this->dimension;
}

/**
* @brief Setter function to set the dimension of the datafield
*
* @param dim Dimension to be set
*/
template <class L, class T>
void Point<L,T>::setDimension(int dim) {
	this->dimension = dim;
}

/**
* @brief Getter function, which returns a Pointer to the Label
*
* @return Pointer to the Label
*/
template <class L, class T>
L Point<L,T>::getLabel() {
	return *label;
}

/**
* @brief Setter function to set the label to a new value
*
* @param l Value for the new label
*/
template <class L, class T> 
void Point<L,T>::setLabel (L l) {
	*label = l;
}

/**
* @brief Setter function to set the label to a new value (pointing to the parameter l )
*
* @param l Pointer to the new label
*/
template <class L, class T> 
void Point<L,T>::setLabel (L *l) {
	label = l;
}

/**
* @brief Getter function to return the values of the datafield
*
* @return Pointer to the datafield
*/
template <class L, class T> 
T *Point<L,T>::getValues() {
	return values;
}

/**
* @brief Setter function to set the new Datafield ( pointing to parameter v )
*
* @param v Pointer to the new values
*/
template <class L, class T> 
void Point<L,T>::setValues (T *v) {
	values = v;
}

#endif /*POINT_H_*/
