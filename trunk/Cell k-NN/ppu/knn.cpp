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
#include <mnist.h>
#include <map>
#include <iostream>
#include <sortedlist.h>

#define MAX_SPE_THREADS 8
#define ARRAY_ALIGNMENT 7
#define POINTER_ALIGNMENT 4

extern spe_program_handle_t knn_spu;

Point *imageToPoint(unsigned char *image, int length, unsigned char label) {
	Point *point = (Point *) malloc(sizeof(Point));

	point->dimensions = length;
	point->label = (int) label;
	point->vector = (int *) malloc_align(length * sizeof(int), ARRAY_ALIGNMENT);

	int i;
	for (i = 0; i < length; ++i) {
		point->vector[i] = (int) image[i];
	}

	return point;
}


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
		parameters[i].distance = (double *) malloc_align(sizeof(double), POINTER_ALIGNMENT);
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
		free_align(parameters[i].distance);
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
		if (votes[i] > maxVote) {
			max = i;
			maxVote = votes[i];
		}
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
		if (votes[i] > maxVote) {
			max = i;
			maxVote = votes[i];
		}
	}

	return max;
}

int *expandToMultiple(int *oldVector, int oldLength, int *newLength, int multiple) {	
	if ((oldLength % multiple) != 0) {
		*newLength = ((int) (oldLength / multiple) + 1) * multiple;
		int *newVector = (int *) realloc_align(oldVector, 
				*newLength * sizeof(int), ARRAY_ALIGNMENT);
		
		if (newVector != NULL) {
			if (*newLength > oldLength) {
				memset(newVector + oldLength, 0,
						(*newLength - oldLength) * sizeof(int));
			}
			
			return newVector;
		}
	}
	
	return oldVector;
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
	//std::map<int,int> *stringCounts = new std::map<int,int>();
	std::cout << "===== Cell KNN ======" << std::endl;
	Point *query, *reference;
	
	query = (Point *) malloc(sizeof(Point));
	reference = (Point *) malloc(sizeof(Point));
	
	query->label = -1;	
	
	int numberOfSpes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);

	query->dimensions = 1 * numberOfSpes;
	query->vector = (int *) malloc_align(query->dimensions * sizeof(int), ARRAY_ALIGNMENT);

	int i;
	for (i = 0; i < query->dimensions; i++) {
		query->vector[i] = query->dimensions - i;
	}

	reference->label = 1;
	reference->dimensions = query->dimensions;
	reference->vector = (int *) malloc_align(query->dimensions * sizeof(int), ARRAY_ALIGNMENT);
	
	for (i = 0; i < query->dimensions; i++) {
		reference->vector[i] = i + 1;
	}
	
	query->vector = expandToMultiple(query->vector, query->dimensions,
			&query->dimensions, 4 * numberOfSpes);
	reference->vector = expandToMultiple(reference->vector, reference->dimensions, 
			&reference->dimensions, 4 * numberOfSpes);
	
	Point **training = (Point **) malloc(1 * sizeof(Point *));
	training[0] = reference;

	classify(1, query, training, 1);
	
	free_align(query->vector);
	free_align(reference->vector);
	free(query);
	free(reference);
	free(training);
	
	std::cout << "===== DONE ======" << std::endl;
	return EXIT_SUCCESS;
}

//*************************************
// TODO TESTING:
/*
void classify2(int k, Point *testpoint, ImageIterator *trainimages, LabelIterator *trainlabels) {

	// Create sorted list of length k.
	SortedList *list = createSortedList(k);

	unsigned char trainlabel;
    unsigned char *trainimage;
	
    Point *trainpoint;
	
    while (hasNextImage(trainimages) && hasNextLabel(trainlabels)) {
            trainimage = nextImage(trainimages);
            trainlabel = nextLabel(trainlabels);

            trainpoint = imageToPoint(trainimage, (trainimages->rows
                            * trainimages->columns), trainlabel);

            free(trainimage);

            
            // Calculate distance of sample to training sample.
    		double d = distance(trainpoint, trainpoint);

    		printf("overall distance = %lf\n", d);

    		// Insert training sample into sorted list, discarding if training sample
    		// not within k nearest neighbor to query point.
    		// TODO free trainpoint just if not inserted!?!
    		if (insert(list, trainpoint, d) < 0)
    			free(trainpoint);
    }
    
	// Do majority vote on k nearest neighbours.
    testpoint->label = majorityVote(list);

	printf("label = %d\n", testpoint->label);

	closeSortedList(list);    
    resetImageIterator(trainimages);
    resetLabelIterator(trainlabels);
}


int main2(int argc, char **argv) {
        if (argc < 5) {
                fprintf(
                                stderr,
                                "Usage: mnist <train label file> <train images file> <test label file> <test images file> \n");
                return EXIT_FAILURE;
        }

        //TODO set as arg
        int k = 10;
        
        Point *testpoint;

        LabelIterator *trainlabels = openLabels(argv[1]);
        ImageIterator *trainimages = openImages(argv[2]);
        LabelIterator *testlabels = openLabels(argv[3]);
        ImageIterator *testimages = openImages(argv[4]);

        unsigned char testlabel;
        unsigned char *testimage;
        
        while (hasNextImage(testimages) && hasNextLabel(testlabels)) {
                
                testimage = nextImage(testimages);
                testlabel = nextLabel(testlabels);
                
                testpoint = imageToPoint(testimage, (testimages->rows
                                * testimages->columns), testlabel);

                free(testimage);
                
                classify2(k, testpoint, trainimages, trainlabels);
        }

        closeLabels(trainlabels);
        closeImages(trainimages);
        closeLabels(testlabels);
        closeImages(testimages);

        return EXIT_SUCCESS;
}
*/