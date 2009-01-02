#ifndef KNN_H_
#define KNN_H_

#include <map>
#include "SortedList.h"
#include "Points.h"

using namespace std;


//-----------------------------------------------
//----------CELL BE Stuff------------------------
//-----------------------------------------------

#include "cellknn.h"
#include <libspe2.h>
#include <pthread.h>

#define MAX_SPE_THREADS 8

extern spe_program_handle_t knn_spu;

/** 
* @brief Structure used to handle SPE threads
*/
typedef struct {
	spe_context_ptr_t context;
	pthread_t pthread;
	unsigned int entry;
	void *arguments;
} PpuThreadData;

/** 
* @brief Function used to run SPE threads
* 
* @param arg Pointer to PpuThreadData object to invoke spe_context_run
* 
*/
void *runSpeContext(void *arg) {
	PpuThreadData *data = (PpuThreadData *) arg;

	if (spe_context_run(data->context, &data->entry, 0, data->arguments, NULL, NULL) < 0) {
		perror("Failed running context");
		exit(1);
	}

	pthread_exit(NULL);
}

//-----------------------------------------------

// L ... label type
// T ... value type
/**
* @brief Template class to use for the KNN algorithmus. \n
* 	class L stands for the label \n
*	class T stands for the value to classify
*/
template <class L, class T>
class KNN {
	int k;
	L majorityVote(SortedList<double, L> &sortedlist);
	double distance(Point<L, T> &testPoint,	Point<L, T> &trainPoint);
public:
	KNN();
	KNN(int k);
	virtual ~KNN();

	// Updated label of testPoint therefore no return value.
	void classify(Point<L, T> &testPoint, Points<L, T> &trainPoints);
};

/**
* @brief Standard constuctor, sets the k ( representing the considered neighbours ) to 1
*
*/
template <class L, class T> KNN<L, T>::KNN() {
	this(1);
}

/**
* @brief Constructor with k ( representing the considered neighbours ) as argument
*
* @param k value responisble how much neighbours are considered for classification
*
*/
template <class L, class T> KNN<L, T>::KNN(int k) {
	this->k = k;
}

template <class L, class T> KNN<L, T>::~KNN() {
}

/**
* @brief Fuction used to classify the testPoint against the trainPoints \n
* 	testPoints label is set with the calculated label.
*
* @param testPoint Point to be classified
* @param trainPoints Training points used as reference
*
*/
template <class L, class T>
void KNN<L, T>::classify(Point<L, T> &testPoint, Points<L, T> &trainPoints) {
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
	L max = 0;

	for (map<int, int>::iterator it = majorityVote.begin(); it	!= majorityVote.end(); ++it) {
		if (it->second > maxVal) {
			maxVal = it->second;
			max = it->first;
		}
	}
	
	return max;
}

//-----------------------------------------------
//----------CELL BE Stuff------------------------
//-----------------------------------------------
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
	int i, numberOfSpes, offset, count, step, size;
	PpuThreadData datas[MAX_SPE_THREADS];
	Parameters parameters[MAX_SPE_THREADS];

	numberOfSpes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);

	if (numberOfSpes > MAX_SPE_THREADS) {
		numberOfSpes = MAX_SPE_THREADS;
	}

	step = 16 / sizeof(int);
	size = testPoint.getDimension();
	if (size % step)
		size = (static_cast<int>(size / step) + 1) * step;
	count = static_cast<int>(size / numberOfSpes);
	if (count % step)
		count = (static_cast<int>(count / step) + 1) * step;

	for (i = 0, offset = 0; i < numberOfSpes; i++, offset += count) {
		// The last SPE calculates the remaining parts.
		parameters[i].count = (i == (numberOfSpes - 1)) ? size - offset : count;
		parameters[i].testPoint = &testPoint.getValues()[offset];
		parameters[i].trainPoint = &trainPoint.getValues()[offset];
		parameters[i].distance = (double *) _malloc_align(sizeof(double), 4);
		*(parameters[i].distance) = -1.0;

		
		if ((datas[i].context = spe_context_create(0, NULL)) == NULL) {
			perror("Failed creating context");
			exit(1);
		}

		if (spe_program_load(datas[i].context, &knn_spu)) {
			perror("Failed loading programs");
			exit(1);
		}

		datas[i].entry = SPE_DEFAULT_ENTRY;
		datas[i].arguments = &parameters[i];
		//printf("Started SPE %d\n",i);
		if (pthread_create(&datas[i].pthread, NULL, &runSpeContext, &datas[i])) {
			perror("Failed creating thread");
			exit(1);
		}
		
	}

	double sum = 0.0;
	for (i = 0; i < numberOfSpes; i++) {
		if (pthread_join(datas[i].pthread, NULL)) {
			perror("Failed pthread_join");
			exit(1);
		}

		sum += *(parameters[i].distance);
	}
	
	for(i=0; i<numberOfSpes; i++)
	{
		spe_context_destroy(datas[i].context);
	}

	return sum;
}
//-----------------------------------------------

#endif /*KNN_H_*/
