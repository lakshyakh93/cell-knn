#ifndef POINT_H_
#define POINT_H_

#include <vector>

template <class L, class T> class Point {
	L label __attribute__ ((aligned (16)));
	std::vector<T> values __attribute__ ((aligned (16)));
public:
	Point(L label, std::vector<T> values);
	Point(std::vector<T> values);
	virtual ~Point();

	L getLabel();
	void setLabel(L label);
	std::vector<T> getValues();
};

#endif /*POINT_H_*/
