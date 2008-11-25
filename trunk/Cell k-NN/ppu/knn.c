/*
 * This code is inspired by the example in
 * Cell SDK 3.1 Programmers Tutorial, page 103 to 111.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libspe2.h>
#include <libmisc.h>
#include <knn.h>
#include "sortedlist.h"

#define MAX_SPE_THREADS 8

extern spe_program_handle_t knn_spu;

typedef struct {
	spe_context_ptr_t context;
	pthread_t pthread;
	unsigned int entry;
	void *arguments;
} PpuThreadData;

void *runSpeContext(void *arg) {
	PpuThreadData *data = (PpuThreadData *) arg;

	if (spe_context_run(data->context, &data->entry, 0, data->arguments, NULL, NULL) < 0) {
		perror("Failed running context");
		exit(1);
	}

	pthread_exit(NULL);
}

double distance(Point *query, Point *reference) {
	int i, numberOfSpes, offset, count;
	PpuThreadData datas[MAX_SPE_THREADS];
	Parameters parameters[MAX_SPE_THREADS];

	numberOfSpes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);

	if (numberOfSpes > MAX_SPE_THREADS) {
		numberOfSpes = MAX_SPE_THREADS;
	}

	count = query->dimensions / numberOfSpes;

	for (i = 0, offset = 0; i < numberOfSpes; i++, offset += count) {
		// The last SPE calculates the remaining parts.
		parameters[i].count = (i == (numberOfSpes - 1)) ? query->dimensions - offset : count;
		parameters[i].query = &query->vector[offset];
		parameters[i].reference = &reference->vector[offset];
		parameters[i].distance = (double *) malloc_align(sizeof(double), 7);
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

	return sum;
}

int majorityVote(SortedList *list) {
	// TODO: Implement this correctly.
	
	if (list->size > 0) {
		return list->values[0]->point->label;
	}
	
	return -1;
}

int simpleMajorityVote(SortedList *list) {
	// TODO: Implement this correctly.
	
	// TODO better name + move to some header ... 
	int nrClasses = 10;
	
	int i, max, maxVote = 0;
	int votes[nrClasses];
	
	for (i = 0; i < list->size; ++i) {
		++votes[list->values[i]->point->label];
	}
	
	for (i = 0; i < nrClasses; ++i) {
		if (votes[i] > maxVote)
			max = i;
	}
	
	return max;
}

int reciprocalMajorityVote(SortedList *list) {
	// TODO: Implement this correctly.
	
	// TODO better name + move to some header ... 
	int nrClasses = 10;
	
	int i, max;
	double votes[nrClasses], maxVote = 0;
	
	for (i = 0; i < list->size;) {
		votes[list->values[i]->point->label] += 1.0 / (double) (++i); //TODO check functionality
	}
	
	for (i = 0; i < nrClasses; ++i) {
		if (votes[i] > maxVote)
			max = i;
	}
	
	return max;
}

void classify(int k, Point *query, Point **training, int length) {
	// Create sorted list of length k.
	SortedList *list = createSortedList(k);

	int i;
	for (i = 0; i < length; i++) {
		// Calculate distance of sample to training sample.
		double d = distance(query, training[i]);
		
		printf("overall distance = %lf\n", d);

		// Insert training sample into sorted list, discarding if training sample
		// not within k nearest neighbor to query point.
		insert(list, training[i], d);
	}

	// Do majority vote on k nearest neighbours.
	query->label = majorityVote(list);
	
	printf("label = %d\n", query->label);
	
	closeSortedList(list);
}

int main() {
	Point *query, *reference;
	
	query = (Point *) malloc(sizeof(Point));
	reference = (Point *) malloc(sizeof(Point));
	
	query->label = -1;
	query->dimensions = 64;
	query->vector = (int *) malloc_align(query->dimensions * sizeof(int), 7);
	
	int i;
	for (i = 0; i < query->dimensions; i++) {
		query->vector[i] = query->dimensions - i;
	};
	
	reference->label = 1;
	reference->dimensions = query->dimensions;
	reference->vector = (int *) malloc_align(query->dimensions * sizeof(int), 7);
	
	for (i = 0; i < query->dimensions; i++) {
		reference->vector[i] = i + 1;
	};
	
	Point **training = (Point **) malloc(1 * sizeof(Point *));
	training[0] = reference;
	
	classify(1, query, training, 1);
	
	return EXIT_SUCCESS;
}
