#ifndef KNN_H_
#define KNN_H_

#include <map>
#include "SortedList.h"
#include "Points.h"

using namespace std;

// L ... label type
// T ... value type
/** 
* @brief Template class to use for the KNN algorithmus. \n
* 	class L stands for the label \n
*	class T stands for the value to classify
*/
template <class L, class T>
class KNN {
	L majorityVote(SortedList<double, L> &sortedlist);
	double distance(Point<L, T> &testPoint,	Point<L, T> &trainPoint);
public:
	// Updated label of testPoint therefore no return value.
	void classify(Point<L, T> &testPoint, Points<L, T> &trainPoints, int k);
};

/** 
* @brief Fuction used to classify the testPoint against the trainPoints \n
* 	testPoints label is set with the calculated label.
* 
* @param testPoint Point to be classified
* @param trainPoints Training points used as reference
* 
*/
template <class L, class T>
void KNN<L, T>::classify(Point<L, T> &testPoint, Points<L, T> &trainPoints, int k) {
	SortedList<double, L> sortedlist(k);
	Point<L,T> *trainPoint;
	for (int i = 0; i < trainPoints.getCount(); ++i) {
		trainPoint = trainPoints.getPoint(i);
		double d = distance(testPoint, *trainPoint);
		sortedlist.insert(d, trainPoint->getLabel());
		delete trainPoint;
	}
	
	testPoint.setLabel(majorityVote(sortedlist));
}

/** 
* @brief Function used to compute the most occurrencies of same distance values (sum of sqare the difference)
* 
* @param sortedlist SortedList filled with distance values and labels
* 
* @return class L The classification of the image
*/
template <class L, class T>
L KNN<L, T>::majorityVote(SortedList<double, L> &sortedlist) {
	map<L, int> majorityVote;

	for (typename SortedList<double, L>::Iterator it = sortedlist.begin(); it != sortedlist.end(); ++it) {
		majorityVote[it->value]++;
	}

	int maxVal = 0;
	L max;

	for (map<int, int>::iterator it = majorityVote.begin(); it	!= majorityVote.end(); ++it) {
		if (it->second > maxVal) {
			maxVal = it->second;
			max = it->first;
		}
	}
	
	return max;
}

/** 
* @brief Function to calculate the sum of the distances sqare over all Points 
* 
* @param testPoint Point to calulate the distance
* @param trainPoint Training Point as counterpart
* 
* @return Sum of the distances squared
*/
template <class L, class T>
double KNN<L, T>::distance(Point<L, T> &testPoint,	Point<L, T> &trainPoint) {
	double sum = 0.0, temp;

	for (int i = 0; i < testPoint.getDimension(); ++i) {
		temp = static_cast<double>(testPoint.getValues()[i] - trainPoint.getValues()[i]);
		sum += temp * temp;
	}

	return sum;
}

#endif /*KNN_H_*/
