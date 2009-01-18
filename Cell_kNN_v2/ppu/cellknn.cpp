#define PPU

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <libspe2.h>
#include <cbe_mfc.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "mnist.h"
#include "cellknn.h"
#include "KNN.h"

ControlBlock cb __attribute__((aligned(16)));

extern spe_program_handle_t cellknn_spu;

// Data structures to work with the SPE
//============================================================================
spe_program_handle_t *program[MAX_NUM_SPES];

// Data structure for running SPE thread
//============================================================================
typedef struct spu_data {
	spe_context_ptr_t spe_ctx;
	pthread_t pthread;
} spu_data_t;

spu_data_t data[MAX_NUM_SPES];

/**
 * @brief Function to write the data of an image to a Point-class
 *
 * @param point Point to write to
 * @param label Label to set for the point
 * @param image Image from which the data comes
 * @param length Size of an image
 */
void imageToPoint(Point<int, int> *point, unsigned char label,
		unsigned char *image, int length) {
	for (int i = 0; i < length; ++i) {
		point->getValues()[i] = static_cast<int> (image[i]);
	}
	point->setLabel(static_cast<int> (label));
}

//============================================================================
// create and run one SPE thread
//============================================================================
void *spu_pthread(void *arg) {

	spu_data_t *datap = (spu_data_t *)arg;
	uint32_t entry = SPE_DEFAULT_ENTRY;

	// TODO check &cb
	if (spe_context_run(datap->spe_ctx, &entry, 0, &cb, NULL, NULL)<0) {
		perror("Failed running context");
		exit(1);
	}
	pthread_exit(NULL);
}

/**
 * @brief Classifies a set of test points using a set of training points.
 *
 * @param k The number of k nearest neighbours.
 * @param test_points The set of test points.
 * @param training_points The set of training points.
 * 
 * @return An array of calculated labels for the set of test points. 
 *         The element at the first position represents the calculated 
 *         label of the first test points. 
 */
int *classify(int k, Points<int, int> &test_points,
		Points<int, int> &training_points) {
	time_t start_time, end_time;

	time(&start_time);

	cb.k = k;
	cb.values_size = training_points.getVSize();
	cb.label_size = training_points.getLSize();

	cb.training_dimension = training_points.getDimension();
	cb.training_count = training_points.getCount();
	cb.training_data_size = training_points.getCount()
			* training_points.getVSize();
	cb.training_points_per_transfer = TRAINING_VALUES_MAX_SIZE
			/ training_points.getVSize();

	cb.test_dimension = test_points.getDimension();
	cb.test_count = test_points.getCount();
	cb.test_data_size = test_points.getCount() * test_points.getVSize();
	cb.test_points_per_transfer = TEST_VALUES_MAX_SIZE / test_points.getVSize();

	cb.ea_training_points = (uint64_t) training_points.getValues(0);
	cb.ea_training_labels = (uint64_t) training_points.getLabel(0);
	cb.ea_test_points = (uint64_t) test_points.getValues(0);
	cb.ea_test_labels = (uint64_t) test_points.getLabel(0);

	cb.num_spes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
	if (cb.num_spes > MAX_NUM_SPES) {
		cb.num_spes = MAX_NUM_SPES;
	}

	#ifdef PRINT
	printf("PPE:\t Num spes = %d\n", cb.num_spes);
	#endif

	uint32_t num;

	printf("PPE:\t Start calculating\n");
	fflush(stdout);

	// create SPE context and load SPE program into the SPE context
	for (num=0; num<cb.num_spes; num++) {
		if ((data[num].spe_ctx = spe_context_create(SPE_MAP_PS
				|SPE_CFG_SIGNOTIFY1_OR|SPE_CFG_SIGNOTIFY2_OR, NULL))==NULL) {
			perror("Failed creating context");
			exit(1);
		}
		if (spe_program_load(data[num].spe_ctx, &cellknn_spu)) {
			perror("Failed loading program");
			exit(1);
		}
	}

	// create SPE pthreads
	for (num=0; num<cb.num_spes; num++) {
		if (pthread_create(&data[num].pthread, NULL, &spu_pthread, &data[num])) {
			perror("Failed creating thread");
			exit(1);
		}
	}

	// map SPE's MFC problem state to main storage (get effective address)
	for (num=0; num<cb.num_spes; num++) {
		if ((cb.spu_mfc_ctl[num] = (uint64_t)spe_ps_area_get(data[num].spe_ctx,
				SPE_CONTROL_AREA))==0) {
			perror("Failed mapping MFC control area");
			exit(1);
		}
		if ((cb.spu_ls[num] = (uint64_t)spe_ls_area_get(data[num].spe_ctx))==0) {
			perror("Failed mapping SPU local store");
			exit(1);
		}
		if ((cb.spu_sig1[num] = (uint64_t)spe_ps_area_get(data[num].spe_ctx,
				SPE_SIG_NOTIFY_1_AREA))==0) {
			perror("Failed mapping Signal1 area");
			exit(1);
		}
		if ((cb.spu_sig2[num] = (uint64_t)spe_ps_area_get(data[num].spe_ctx,
				SPE_SIG_NOTIFY_2_AREA))==0) {
			perror("Failed mapping Signal2 area");
			exit(1);
		}
	}

	// send each SPE its number using BLOCKING mailbox write
	for (num=0; num<cb.num_spes; num++) {
		// write 1 entry to in_mailbox - we don't know if we have availalbe space so use blocking
		// cb parameter have to be loaded after receiving local id!!!
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&num, 1,
				SPE_MBOX_ALL_BLOCKING);
	}

	// wait for all SPEs to complete
	for (num=0; num<cb.num_spes; num++) {
		// wait for all the SPE pthread to complete
		if (pthread_join(data[num].pthread, NULL)) {
			perror("Failed joining thread");
			exit(1);
		}

		// destroy the SPE contexts
		if (spe_context_destroy(data[num].spe_ctx)) {
			perror("Failed spe_context_destroy");
			exit(1);
		}
	}

	time(&end_time);

	double difference = difftime(end_time, start_time);
	printf("It took %.2lf seconds to calculate %d test points and %d training points\n",
			difference, cb.test_count, cb.training_count);

	return 0;
}

//============================================================================
// main
//============================================================================
int main() {
	//*******************************************************
	cout << "---------[Program start]----------" << endl;

	/*	if (argc < 3) {
	 fprintf(stderr, "Usage: mnist <k> <mnist path> [<nr train images> <nr test images>]\n");
	 return EXIT_FAILURE;
	 }
	 */
	int k = 5; // TODO atoi(argv[1]);

	#ifdef PRINT
	cout << "k = " << k << endl;
	#endif
	
	string base = "/tmp/images/"; // TODO string base(argv[2]);

	LabelIterator *trainLabels = openLabels(base + "train-labels-idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images-idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels-idx1-ubyte");
	ImageIterator *testImages = openImages(base + "t10k-images-idx3-ubyte");

	if (trainLabels == 0 || trainImages == 0 || testLabels == 0 || testImages
			== 0) {
		cout
				<< "One or more files couldn't be found! Make sure that following files are in the directory given as argument:\n"
				<< "\ttrain-labels-idx1-ubyte\n"
				<< "\ttrain-images-idx3-ubyte\n"
				<< "\tt10k-labels-idx1-ubyte\n" << "\tt10k-images-idx3-ubyte\n"
				<< endl;
		exit(-1);
	}

	trainLabels->count = 1000;
	trainImages->count = 1000;
	testLabels->count = 10;
	testImages->count = 10;

	unsigned char label;
	unsigned char *image;

	Points<int, int> training_points(trainImages->count, trainImages->rows
			* trainImages->columns);
	Points<int, int> test_points(testImages->count, testImages->rows
			* testImages->columns);

	#ifdef PRINT
	cout << trainImages->count << " " << trainImages->rows*trainImages->columns << endl;
	cout << "size: " << trainImages->rows*trainImages->columns << "\tcount: " << trainImages->count << endl;
	#endif

	int i = 0;
	Point<int, int> *trainPoint;
	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		trainPoint = training_points.getPoint(i++);
		imageToPoint(trainPoint, label, image, (trainImages->columns
				*trainImages->rows));

		#ifdef PRINT
		if ((i % 1000) == 0)
			cout << i << " training images loaded." << endl;
		#endif

		delete trainPoint;
		free(image);
	}

	i = 0;
	Point<int, int> *testPoint;
	while (hasNextLabel(testLabels) && hasNextImage(testImages)) {
		image = nextImage(testImages);
		label = nextLabel(testLabels);

		testPoint = test_points.getPoint(i++);
		imageToPoint(testPoint, label, image, (testImages->columns
				* testImages->rows));

		#ifdef PRINT
		if ((i % 1000) == 0)
			cout << i << " test images loaded." << endl;
		#endif

		delete testPoint;
		free(image);
	}
	//********************************************************
	
	int *result = classify(k, test_points, training_points);
	
	// TODO: Analyze.
	
	if (result != 0) {
		free(result);
	}

	printf("PPE:\t Program finished\n");

	return (0);
}

