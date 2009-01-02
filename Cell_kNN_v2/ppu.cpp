#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <libspe2.h>
#include <cbe_mfc.h>
#include <pthread.h>

#include <unistd.h>

#include "KNN.h"
#include "Point.h"
#include "Points.h"
#include "libMnist.cpp" // TODO why .cpp?
#define MAX_NUM_SPES 8
#define DEBUG
#define PRETTY_PRINT

extern spe_program_handle_t spu;

// Data structures to work with the SPE
//============================================================================
spe_program_handle_t *program[MAX_NUM_SPES];

// Data structure for running SPE thread
//============================================================================
typedef struct spu_data {
	spe_context_ptr_t spe_ctx;
	pthread_t pthread;
	volatile spe_spu_control_area_t* mfc_ctl;
	volatile void* spu_ls;
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
void imageToPoint(Point<int, int> *point, unsigned char label, unsigned char *image, int length) {
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

	if (spe_context_run(datap->spe_ctx, &entry, 0, NULL, NULL, NULL)<0) {
		perror("Failed running context");
		exit(1);
	}
	pthread_exit(NULL);
}

//============================================================================
// main
//============================================================================
int main() {
	//*******************************************************
#ifdef PRETTY_PRINT
	cout << "---------[Program start]----------" << endl;
#endif

	/*	if (argc < 3) {
	 fprintf(stderr, "Usage: mnist <k> <mnist path> [<nr train images> <nr test images>]\n");
	 return EXIT_FAILURE;
	 }
	 */
	int k = 5; // TODO atoi(argv[1]);

	KNN<int, int> knn(k);

#ifdef PRETTY_PRINT
	cout << "k = " << k << endl;
#endif
	string base = "/tmp/images/"; // TODO string base(argv[2]);

	LabelIterator *trainLabels = openLabels(base + "train-labels-idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images-idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels-idx1-ubyte");
	ImageIterator *testImages = openImages(base + "t10k-images-idx3-ubyte");

	if (trainLabels == 0 || trainImages == 0 || testLabels == 0 || testImages == 0) {
		cout
				<< "One or more files couldn't be found! Make sure that following files are in the directory given as argument:\n"
				<< "\ttrain-labels-idx1-ubyte\n" << "\ttrain-images-idx3-ubyte\n" << "\tt10k-labels-idx1-ubyte\n"
				<< "\tt10k-images-idx3-ubyte\n" << endl;
		exit(-1);
	}

	trainLabels->count = 100;
	trainImages->count = 100;
	testLabels->count = 10;
	testImages->count = 10;

	unsigned char label;
	unsigned char *image;

	Points<int, int> points(trainImages->count, trainImages->rows *trainImages->columns);

#ifdef PRETTY_PRINT
	cout << trainImages->count << " " << trainImages->rows*trainImages->columns << endl;
	cout << "size: " << trainImages->rows*trainImages->columns << "\tcount: " << trainImages->count << endl;
#endif

	int i = 0;
	Point<int, int> *trainPoint;
	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		trainPoint = points.getPoint(i++);
		imageToPoint(trainPoint, label, image, (trainImages->columns *trainImages->rows));

#ifdef PRETTY_PRINT
		if ((i % 1000) == 0)
			cout << i << " Images loaded." << endl;
#endif

		delete trainPoint;
		free(image);
	}
	//********************************************************


	int num, ack, num_spes= MAX_NUM_SPES;
	uint64_t ea, ls;

	// TODO check for number of available SPEs
	num_spes = MAX_NUM_SPES;

	printf("S)PPE:) Start with huge hope\n");

	// create SPE context and load SPE program into the SPE context
	for (num=0; num<num_spes; num++) {
		if ((data[num].spe_ctx = spe_context_create(SPE_MAP_PS, NULL))==NULL) {
			perror("Failed creating context");
			exit(1);
		}
		if (spe_program_load(data[num].spe_ctx, &spu)) {
			perror("Failed loading program");
			exit(1);
		}
	}

	// create SPE pthreads
	for (num=0; num<num_spes; num++) {
		if (pthread_create(&data[num].pthread, NULL, &spu_pthread, &data[num])) {
			perror("Failed creating thread");
			exit(1);
		}
	}

	// STEP 0: map SPE's MFC problem state to main storage (get effective address)
	for (num=0; num<num_spes; num++) {
		if ((data[num].mfc_ctl = (spe_spu_control_area_t*)spe_ps_area_get(data[num].spe_ctx, SPE_CONTROL_AREA))==NULL) {
			perror("Failed mapping MFC control area");
			exit(1);
		}
		if ((data[num].spu_ls = spe_ls_area_get(data[num].spe_ctx))==NULL) {
			perror("Failed mapping SPU local store");
			exit(1);
		}
	}

	int dimension = points.getDimension(); 
	int count = points.getCount();
	
	// STEP 1: send each SPE its number using BLOCKING mailbox write
	for (num=0; num<num_spes; num++) {
		printf("1)PPE->SPE%d: <%u>\n", num, num);

		// write 1 entry to in_mailbox - we don't know if we have availalbe space so use blocking
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&num, 1, SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&num_spes, 1, SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&count, 1, SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&dimension, 1, SPE_MBOX_ALL_BLOCKING);
	}

	// STEP 2: send SPE0 the EA of the points array
	//         send each SPE the effective address of other SPE's MFC area
	//         use NON-BLOCKING mailbox write after first verifying availability of space
	ea = (uint64_t)points.getValues(0);

	// write 2 entries to in_mailbox - blocking
	spe_in_mbox_write(data[0].spe_ctx, (uint32_t*)&ea, 2, SPE_MBOX_ALL_BLOCKING);
	uint64_t ea_next, ea_prev;
	for (num=0; num<num_spes; num++) {
		
		ea_next	= (uint64_t)data[(num==num_spes-1)?0:num+1].mfc_ctl;
		ea_prev	= (uint64_t)data[(num==0)?num_spes-1:num-1].mfc_ctl;
		ls		= (uint64_t)data[(num==0)?num_spes-1:num-1].spu_ls;

		//printf("2)PPE->SPE%d: <%llx>\n", num, ea);		

		// write 4 entries to in_mailbox - blocking
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&ea_next, 2, SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&ea_prev, 2, SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(data[num].spe_ctx, (uint32_t*)&ls, 2, SPE_MBOX_ALL_BLOCKING);
	}

	// STEP 3: 	read acknowledge from each SPE using NON-BLOCKING mailbox read
	for (num=0; num<num_spes; num++) {
		while (!spe_out_mbox_status(data[num].spe_ctx)) {

			// PPE can do other things meanwhile before check status again
		};
		spe_out_mbox_read(data[num].spe_ctx, (uint32_t*)&ack, 1);

		printf("3)PPE<-SPE%u: <%d>\n", num, ack);
	}

	// wait for all SPEs to complete
	for (num=0; num<num_spes; num++) {
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

	printf("E)PPE:) Complete with great success\n");

	return (0);
}

