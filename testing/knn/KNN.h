#ifndef KNN_H_
#define KNN_H_

#include <list>
#include <SortedList.cpp>
#include <Point.cpp>

using namespace std;
template <class L, class T>
class KNN {
	int k;
	L majorityVote(SortedList<double, Point<L, T> > sortedlist);
	double distance(Point<L, T> testPoint,	Point<L, T> trainPoint);
public:
	KNN();
	KNN(int k);
	virtual ~KNN();

	// Updated label of testPoint therefore no return value.
	void classify(Point<L, T> testPoint, list<Point<L, T> > trainPoints);
};

#endif /*KNN_H_*/
