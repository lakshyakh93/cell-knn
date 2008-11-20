#include "../knn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <libspe2.h>

#define MAX_SPU_THREADS 8

extern spe_program_handle_t knn_spu;

typedef struct {
	spe_context_ptr_t context;
	pthread_t pthread;
	unsigned int entry;
	void *arguments;
} PpuThreadData;

typedef struct {
	Point *point;
	double distance;
} ListItem;

typedef struct {
	int size;
	int length;
	ListItem **values;
} SortedList;

int insert(SortedList *list, Point *point, double distance) {
	int i;
	int index = -1;

	if (list->size == 0) {
		index = 0;
	}

	for (i = 0; i < list->size; i++) {
		if (distance <= list->values[i]->distance) {
			index = i;
			break;
		}
	}

	// Shift all elements.

	if (list->size < list->length) {
		list->values[list->size] = list->values[list->size - 1];
	}

	for (i = (list->size - 2); i >= index; i--) {
		values[i + 1] = values[i];
	}

	if (index >= 0) {
		ListItem item;
		item.point = point;
		item.distance = distance;

		list->values[index] = &item;
		list->size++;
	}

	return index;
}

void *runSpeContext(void *arg) {
	PpuThreadData *data = (PpuThreadData *) arg;

	if (spe_context_run(data->context, &data->entry, 0, data->arguments, NULL, NULL) < 0) {
		perror("Failed running context");
		exit(1);
	}

	pthread_exit(NULL);
}

double distance(Point *query, Point *reference) {
	int n = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
	int len = query->dimensions / n;

	// Let each SPE calculate distance of a part of the sample and sum up distances.
	int sum = 0;
	int i;
	for (i = 0; i < n; i++) {
		int offset = i * len;

		// Spawn SPE process.
		sum += euclidean(query, reference, offset, len);
	}

	return sqrt(sum);

	int i, numberOfSpes, offset, count;
	PpuThreadData datas[MAX_SPE_THREADS];
	Parameters parameters[MAX_SPE_THREADS];

	numberOfSpes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);

	if (numberOfSpes > MAX_SPU_THREADS) {
		numberOfSpes = MAX_SPU_THREADS;
	}

	count = query->dimensions / numberOfSpes;

	for (i = 0, offset = 0; i < numberOfSpes; i++, offset += count) {
		// The last SPE calculates the remaining parts.
		parameters[i].count = (i == (numberOfSpes - 1)) ? query->dimensions - offset : count;
		parameters[i].query = query->vector;
		parameters[i].reference = reference->vector;
		parameters[i].distance = -1.0;

		if ((datas[i].context[i] = spe_context_create(0, NULL)) = NULL) {
			perror("Failed creating context");
			exit(1);
		}

		if (spe_program_load(datas[i].context[i], &knn_spu)) {
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

	for (i = 0; i < numberOfSpes; i++) {
		if (pthread_join(datas[i].pthread, NULL)) {
			perror("Failed pthread_join");
			exit(1);
		}
	}

	return EXIT_SUCCESS;
}

int majorityVote(SortedList list) {
	return 0;
}

void classify(int k, Point *query, Point **training, int length) {
	// Create sorted list of length k.
	SortedList list;
	list.size = 0;
	list.length = k;
	list.values = (ListItem *) malloc(list.length * sizeof(ListItem));

	int i;
	for (i = 0; i < length; i++) {
		// Calculate distance of sample to training sample.
		double distance = distance(query, training[i]);

		// Insert training sample into sorted list, discarding if training sample
		// not within k nearest neighbor to query point.
		insert(&list, &training[i], distance);
	}

	// Do majority vote on k nearest neighbours.
	query->label = majorityVote(list);
}
