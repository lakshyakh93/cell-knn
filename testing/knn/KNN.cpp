#include "KNN.h"
#include <map>
#include <vector>
#include <math.h>

template <class L, class T> KNN<L, T>::KNN() {
	this(1);
}

template <class L, class T> KNN<L, T>::KNN(int k) {
	this->k = k;
}

template <class L, class T> KNN<L, T>::~KNN() {
}

template <class L, class T>
void KNN<L, T>::classify(Point<L, T> testPoint, list<Point<L, T> > trainPoints) {
	SortedList<double, Point<L, T> > sortedlist(k);
	
	for (typename list<Point<L, T> >::iterator trainPoint = trainPoints.begin(); trainPoint != trainPoints.end(); trainPoint++) {
		double d = distance(testPoint, *trainPoint);
		sortedlist.insert(d, *trainPoint);
	}
	
	testPoint->setLabel(majorityVote(sortedlist));
}

template <class L, class T>
L KNN<L, T>::majorityVote(SortedList<double, Point<L, T> > sortedlist) {
	map<L, int> majorityVote;

	for (typename SortedList<double, Point<L, T> >::Iterator it = sortedlist.begin(); it != sortedlist.end(); ++it) {
		majorityVote[it->value->getLabel()]++;
	}

	int maxVal = 0;
	L max;

	for (map<char, int>::iterator it = majorityVote.begin(); it	!= majorityVote.end(); ++it) {
		if (it->second > maxVal) {
			maxVal = it->second;
			max = it->first;
		}
	}
}

template <class L, class T>
double KNN<L, T>::distance(Point<L, T> testPoint,	Point<L, T> trainPoint) {
	double sum = 0.0, temp;

	for (int i = 0; i < (int) testPoint->values->size(); ++i) {
		temp = static_cast<double>(testPoint->values[i] - trainPoint->values[i]);
		sum += temp * temp;
	}

	return sum;
}

int main() {
	return 0;
}
