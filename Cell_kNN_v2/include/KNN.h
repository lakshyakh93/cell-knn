#ifndef KNN_H_
#define KNN_H_

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
private:
	static L majorityVote(SortedList<double, L> &sortedlist);
	static double distance(Point<L, T> &testPoint,	Point<L, T> &trainPoint);
public:
	// Updated label of testPoint therefore no return value.
	void classify(Point<L, T> &testPoint, Points<L, T> &trainPoints, int k);
	static L classify(Point<L, T> &testPoint, Points<L, T> &trainPoints, SortedList<double, L> &list);
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
		double d = KNN<L, T>::distance(testPoint, *trainPoint);
		sortedlist.insert(d, trainPoint->getLabel());
		delete trainPoint;
	}
	
	testPoint.setLabel(KNN<L, T>::majorityVote(sortedlist));
}

/** 
* @brief Function used to classify the testPoint against the trainPoints.
* 
* @param testPoint Point to be classified
* @param trainPoints Training points used as reference
* 
* @return Label of testPoint.
* 
*/
template <class L, class T>
static L classify(Point<L, T> &testPoint, Points<L, T> &trainPoints, SortedList<double, L> &sortedlist) {
	Point<L,T> *trainPoint;
	
	for (int i = 0; i < trainPoints.getCount(); ++i) {
		trainPoint = trainPoints.getPoint(i);
		double d = KNN<L, T>::distance(testPoint, *trainPoint);
		sortedlist.insert(d, trainPoint->getLabel());
		delete trainPoint;
	}
	
	return KNN<L, T>::majorityVote(sortedlist);
}

/** 
* @brief Fuction used to classify the testPoint against the trainPoints \n
* 	testPoints label is set with the calculated label.
* 
* @param testPoint Point to be classified
* @param trainPoints Training points used as reference
* 
*/

/** 
* @brief Function used to compute the most occurrencies of same distance values (sum of sqare the difference)
* 
* @param sortedlist SortedList filled with distance values and labels
* 
* @return class L The classification of the image
*/
template <class L, class T>
static L majorityVote(SortedList<double, L> &sortedlist) {
        L  *labels = new L[sortedlist.getCurrSize()];
        int *count = new int[sortedlist.getCurrSize()];
        int size = 0, i = 0, j = 0;
        typename SortedList<double, L>::node *head = sortedlist.getHead();

        for (i = 0; i < sortedlist.getCurrSize(); i ++) {
            //labels[i] =  0;
            count[i] = 0;
        }


        for (i = 0; i < sortedlist.getCurrSize(); i++) {
            for (j = 0; j <= size; j++) {
                if (j == size) { // create new labels entry
                    labels[j] = head->value;
                    size++;
                    break;
                } else if (head->value == labels[j]) { // key found
                    break;
                }
            }
                count[j]++;
                head = head->next;
        }


        int maxCount = 0;
        L max = (L) 0;

        for (i = 0; i < size; i++) {
            if (count[i] > maxCount) {
                maxCount = count[i];
                max = labels[i];
            }
        }

        delete [] labels;
        delete [] count;
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
static double distance(Point<L, T> &testPoint,	Point<L, T> &trainPoint) {
	double sum = 0.0, temp;

	for (int i = 0; i < testPoint.getDimension(); ++i) {
		temp = static_cast<double>(testPoint.getValues()[i] - trainPoint.getValues()[i]);
		sum += temp * temp;
	}

	return sum;
}

#endif /*KNN_H_*/
