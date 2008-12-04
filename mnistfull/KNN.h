#ifndef KNN_H_
#define KNN_H_

#include <map>
#include "SortedList.h"
#include "Points.h"

using namespace std;

// L ... label type
// T ... value type
template<class L, class T>
class KNN {
	int k;
	L majorityVote(SortedList<double, L> &sortedlist);
	double distance(Point<L, T> &testPoint, Point<L, T> &trainPoint);
public:
	KNN();
	KNN(int k);
	virtual ~KNN();

	// Updated label of testPoint therefore no return value.
	void classify(Point<L, T> &testPoint, Points<L, T> &trainPoints);
};

template<class L, class T> KNN<L, T>::KNN() {
	this(1);
}

template<class L, class T> KNN<L, T>::KNN(int k) {
	this->k = k;
}

template<class L, class T> KNN<L, T>::~KNN() {
}

template<class L, class T>
void KNN<L, T>::classify(Point<L, T> &testPoint, Points<L, T> &trainPoints) {
	SortedList<double, L> sortedlist(k);
	Point<L, T> *trainPoint;
	for (int i = 0; i < trainPoints.getCount(); ++i) {
		trainPoint = trainPoints.getPointObject(i);
		double d = distance(testPoint, *trainPoint);
		sortedlist.insert(d, trainPoint->getLabel());
	}

	testPoint.setLabel(majorityVote(sortedlist));
}

template<class L, class T>
L KNN<L, T>::majorityVote(SortedList<double, L> &sortedlist) {
	map<L, int> majorityVote;

	for (typename SortedList<double, L>::Iterator it = sortedlist.begin(); it
			!= sortedlist.end(); ++it) {
		majorityVote[it->value]++;
	}

	int maxVal = 0;
	L max;

	for (map<L, int>::iterator it = majorityVote.begin(); it
			!= majorityVote.end(); ++it) {
		if (it->second > maxVal) {
			maxVal = it->second;
			max = it->first;
		}
	}

	return max;
}

template<class L, class T>
double KNN<L, T>::distance(Point<L, T> &testPoint, Point<L, T> &trainPoint) {
	double sum = 0.0, temp;

	for (int i = 0; i < testPoint.getDimension(); ++i) {
		temp = static_cast<double> (testPoint.getValues()[i]
				- trainPoint.getValues()[i]); //TODO trainpoints no more available after 1st run
		sum += temp * temp;
	}

	return sum;
}

#endif /*KNN_H_*/
