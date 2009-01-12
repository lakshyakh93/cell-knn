#define SPU

#include <libmisc.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

//#include "KNN.h"
#include "spu_mfcio_ext.h"
#include "cellknn.h"
#include "Points.h"

CONTROL_BLOCK cb __attribute__((aligned(16)));

 char trainingValues[2][TRAINING_VALUES_MAX_SIZE] __attribute__((aligned(128)));
 char testValues[TEST_VALUES_MAX_SIZE] __attribute__((aligned(128)));
 char *trainingLabels[2];
 char *testLabels;

uint32_t my_num;
uint32_t tagId[2];

uint32_t fillBuffer(char *buffer, uint32_t tagId, uint64_t address, uint32_t transfer) {
	uint32_t offset = 0;

	if (transfer <= 0)
		return 0;

	while (DMA_MAX_SIZE < transfer) {
		mfc_get((void *) (buffer + offset), address, DMA_MAX_SIZE, tagId, 0, 0);
		offset += DMA_MAX_SIZE;
		transfer -= DMA_MAX_SIZE;
	}

	if (transfer > 0) {
		mfc_get((void *) (buffer + offset), address, transfer, tagId, 0, 0);
		offset += transfer;
		transfer = 0;
	}

	return offset;
}

uint32_t calculate(Points<int, int> &test_points, Points<int, int> &training_points) {
	//iterate over test_points
	//calculate temporary knn over training points (~30 points)
	//store a sorted list for each testpoint sortedlists (=> created in streamdata or globally)
	// => cb.test_points_per_transfer sorted lists
	
	return 0;
}

uint32_t streamData(Points<int, int> &test_points) {
	uint32_t transfer[2], count[2], iteration = 0;

	// Reset size data.
	uint32_t size_data = cb.training_data_size;
	uint32_t size_buffer = cb.values_size * cb.training_points_per_transfer;

	// effective address to training points.
	uint64_t ea_training_points, ea_training_labels;
	// SPEs (except SPE0) need to get address for buffer
	if (my_num == 0) {
		ea_training_points = cb.ea_training_points;
		ea_training_labels = cb.ea_training_labels;
	} else {
		// adds local address of buffer to global address of SPU
		ea_training_points = cb.spu_ls[my_num-1] + spu_read_in_mbox();
		ea_training_labels = cb.spu_ls[my_num-1] + spu_read_in_mbox();
	}

	// decide amount to transfer
	transfer[0] = (size_buffer < size_data) ? size_buffer : size_data;
	count[0] = transfer[0] / cb.values_size;

	// transfer data
	if (transfer[0] != fillBuffer(trainingValues[0], tagId[0], ea_training_points, transfer[0])) {
		printf("Error occured at transfering training points 0");
		return -1;
	}
	
	if (count[0] * cb.label_size != fillBuffer(trainingLabels[0], tagId[0], ea_training_labels,
			count[0] * cb.label_size)) {
		printf("Error occured at transfering training labels 0");
		fflush(stdout);
		return -1;
	}


#ifdef PRINT
	printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
	fflush(stdout);
#endif

	// adjust data source
	if (my_num == 0) {
		ea_training_points += transfer[0];
		ea_training_labels += cb.label_size * count[0];
	} else {
		// adds local address of buffer to global address of SPU
		ea_training_points = cb.spu_ls[my_num-1] + spu_read_in_mbox();
		ea_training_labels = cb.spu_ls[my_num-1] + spu_read_in_mbox();
	}

	// decide amount to transfer
	transfer[1] = (size_buffer < size_data-transfer[0]) ? size_buffer : size_data-transfer[0];
	count[1] = transfer[1] / cb.values_size;

	// transfer data
	if (transfer[1] != fillBuffer(trainingValues[1], tagId[1], ea_training_points, transfer[1])) {
		printf("Error occured at transfering training points 0");
		return -1;
	}

	if (count[1] * cb.label_size != fillBuffer(trainingLabels[1], tagId[1], ea_training_labels,
			count[1] * cb.label_size)) {
		printf("Error occured at transfering training labels 0");
		fflush(stdout);
		return -1;
	}

#ifdef PRINT
	printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
	fflush(stdout);
#endif

	while (size_data > 0) {
		// check if dma0 finished
		waittag(tagId[0]);

#ifdef PRINT
		printf("SPE%d:\tTransfer %d on Buffer 0 complete\n", my_num, iteration-1);
		fflush(stdout);
#endif
		
		// adjust remaining amount of data
		size_data -= transfer[0];

		// send next SPU the address of filled buffer
		if (my_num != cb.num_spes-1) {
			if (write_in_mbox((uint32_t)trainingValues[0], cb.spu_mfc_ctl[my_num+1], tagId[0])!=1) {
				printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// send next SPU the address of filled labels
		if (my_num != cb.num_spes-1) {
			if (write_in_mbox((uint32_t)trainingLabels[0], cb.spu_mfc_ctl[my_num+1], tagId[0])!=1) {
				printf("SPE%d:\tfail to send label address to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// send copy confirmation to previous SPU
		if (my_num != 0) {
			if (write_signal2(0, cb.spu_sig2[my_num-1], tagId[0])!=1) {
				printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// TrainingPoints0 loaded, create Object
		Points<int, int> training_points0(cb.training_points_per_transfer, cb.training_dimension, trainingLabels[0], trainingValues[0]);
		// do calcualtions on buffer
		calculate(test_points,training_points0);

		// check if dma1 finished
		if (size_data > 0) //TODO check condition
			waittag(tagId[1]);

#ifdef PRINT
		printf("SPE%d:\tTransfer %d on Buffer 1 complete\n", my_num, iteration-1);
		fflush(stdout);
#endif

		// adjust remaining amount of data
		size_data -= transfer[1];

		// send next SPU the address of filled buffer
		if (my_num != cb.num_spes-1) {
			if (write_in_mbox((uint32_t)trainingValues[1], cb.spu_mfc_ctl[my_num+1], tagId[1])!=1) {
				printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// send next SPU the address of filled buffer
		if (my_num != cb.num_spes-1) {
			if (write_in_mbox((uint32_t)trainingLabels[1], cb.spu_mfc_ctl[my_num+1], tagId[1])!=1) {
				printf("SPE%d:\tfail to send label address to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// send copy confirmation to previous SPU
		if (my_num != 0) {
			if (write_signal2(1, cb.spu_sig2[my_num-1], tagId[1])!=1) {
				printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
				fflush(stdout);
				return -1;
			}
		}

		// check if buffer is already copied (may be overwritten before it's copied otherwise)
		if (my_num != cb.num_spes-1) {
			uint32_t temp;
			if ((temp = spu_read_signal2())!=0) {
				printf("SPE%d:\tconfirmation failed, received %d, wanted 0\n", my_num, temp);
				fflush(stdout);
				return -1;
			}
		}

		// decide amount to transfer
		transfer[0] = (size_buffer < size_data) ? size_buffer : size_data;
		count[0] = transfer[0] / cb.values_size;

		//if there is something to transfer then
		if (transfer[0] > 0) {
			// adjust data source
			if (my_num == 0) {
				ea_training_points += transfer[1];
				ea_training_labels += cb.label_size * count[1];
			} else {
				// adds local address of buffer to global address of SPU
				ea_training_points = cb.spu_ls[my_num-1] + spu_read_in_mbox();
				ea_training_labels = cb.spu_ls[my_num-1] + spu_read_in_mbox();
			}
			// transfer data
			if (transfer[0] != fillBuffer(trainingValues[0], tagId[0], ea_training_points, transfer[0])) {
				printf("Error occured at transfering training points 0");
				return -1;
			}

			
			if (count[0] * cb.label_size != fillBuffer(trainingLabels[0], tagId[0], ea_training_labels,
					count[0] * cb.label_size)) {
				printf("Error occured at transfering training labels 0");
				fflush(stdout);
				return -1;
			}

			
#ifdef PRINT
			printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
			fflush(stdout);
#endif
		}

		// TrainingPoints0 loaded, create Object
		Points<int, int> training_points1(cb.training_points_per_transfer, cb.training_dimension, trainingLabels[1], trainingValues[1]);
		// do calcualtions on buffer
		calculate(test_points, training_points1);

		// check if buffer is already copied (may be overwritten before it's copied otherwise)
		if (my_num != cb.num_spes-1) {
			uint32_t temp;
			if ((temp = spu_read_signal2())!=1) {
				printf("SPE%d:\tconfirmation failed, received %d, wanted 1\n", my_num, temp);
				fflush(stdout);
				return -1;
			}
		}

		// decide amount to transfer
		transfer[1] = (size_buffer < size_data-transfer[0]) ? size_buffer : size_data-transfer[0];
		count[1] = transfer[1] / cb.values_size;

		//if there is something to transfer then
		if (transfer[1] > 0) {
			// adjust data source
			if (my_num == 0) {
				ea_training_points += transfer[0];
				ea_training_labels += cb.label_size * count[0];
			} else {
				// adds local address of buffer to global address of SPU
				ea_training_points = cb.spu_ls[my_num-1] + spu_read_in_mbox();
				ea_training_labels = cb.spu_ls[my_num-1] + spu_read_in_mbox();
			}
			// transfer data
			if (transfer[1] != fillBuffer(trainingValues[1], tagId[1], ea_training_points, transfer[1])) {
				printf("Error occured at transfering training values 0");
				return -1;
			}

			if (count[1] * cb.label_size != fillBuffer(trainingLabels[1], tagId[1], ea_training_labels,
					count[1] * cb.label_size)) {
				printf("Error occured at transfering training labels 0");
				fflush(stdout);
				return -1;
			}


			
#ifdef PRINT
			printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
			fflush(stdout);
#endif
		}

		// increase iteration counter
		iteration++;
	}
	//print sorted lists (see calculate(...) for more information on sorted lists)
return 0;
}

int main(unsigned long long speid, unsigned long long argp, unsigned long long envp) {
	uint64_t ea_test_points, ea_test_labels;
	//uint32_t training_count, dimensions, test_count, test_data_size, point_size, size_data_start;
	//uint64_t ea_test_points, ea_test_labels, ea_training_points_start;

#ifdef PRINT
	printf("SPE%d:\tstarted\n", my_num);
	fflush(stdout);
#endif

	if ((tagId[0]= mfc_tag_reserve())==MFC_TAG_INVALID) {
		printf("SPE: ERROR can't allocate tag ID\n");
		return -1;
	}
	if ((tagId[1]= mfc_tag_reserve())==MFC_TAG_INVALID) {
		printf("SPE: ERROR can't allocate tag ID\n");
		return -1;
	}

	// read from PPE my number using BLOCKING mailbox read	
	my_num = spu_read_in_mbox();

	// now it's safe to load parameters 
	mfc_get(&cb, argp, sizeof(CONTROL_BLOCK), tagId[0], 0, 0);
	waittag(tagId[0]);
	
	if (my_num == 0) {
		print_control_block(&cb);
	}

	testLabels = (char *) malloc_align(cb.label_size * cb.test_points_per_transfer, 7);
	trainingLabels[0] = (char *) malloc_align(cb.label_size * cb.training_points_per_transfer, 7);
	trainingLabels[1] = (char *) malloc_align(cb.label_size * cb.training_points_per_transfer, 7);

#ifdef PRINT
	printf("SPE%d:\tSetup complete\n", my_num);
	fflush(stdout);
#endif

	uint32_t test_point_transfers = cb.test_count / cb.test_points_per_transfer;
	if (cb.test_count % cb.test_points_per_transfer != 0) {
		test_point_transfers++;
	}
	
	uint32_t test_point_transfers_per_spu = test_point_transfers / cb.num_spes;
	if (test_point_transfers % cb.num_spes != 0) {
		test_point_transfers_per_spu++;
	}
	
	uint32_t test_points_per_spu = test_point_transfers_per_spu 
		* cb.test_points_per_transfer;

	// calculate offset in testpoint array
	ea_test_points = cb.ea_test_points + my_num * test_points_per_spu * cb.values_size;
	ea_test_labels = cb.ea_test_labels + my_num * test_points_per_spu * cb.label_size;

	// adjust count of last spu
	if (my_num == cb.num_spes-1) {
		test_point_transfers_per_spu = test_point_transfers - (cb.num_spes-1)*test_point_transfers_per_spu;
		test_points_per_spu = test_point_transfers_per_spu * cb.test_points_per_transfer;
	}
	
	for (uint32_t test_point_transfer = 0; test_point_transfer < test_point_transfers_per_spu; test_point_transfer++) {
#ifdef PRINT
		printf("SPE%d:\ttest iteration %d started\n", my_num, test_point_transfer);
		fflush(stdout);
#endif
		
#ifdef DEBUG
		printf("%llu \t %llu \n",ea_test_points,cb.ea_test_points);
#endif
		// fill test point and label buffer
		if (cb.test_points_per_transfer * cb.values_size != fillBuffer(testValues, tagId[0], ea_test_points,
				cb.test_points_per_transfer * cb.values_size)) {
			printf("Error occured at transfering test points");
			fflush(stdout);
			return -1;
		}

		if (cb.test_points_per_transfer * cb.label_size != fillBuffer(testLabels, tagId[0], ea_test_labels,
				cb.test_points_per_transfer * cb.label_size)) {
			printf("Error occured at transfering test labels");
			fflush(stdout);
			return -1;
		}

		ea_test_points += cb.test_points_per_transfer * cb.values_size;
		ea_test_labels += cb.test_points_per_transfer * cb.label_size;

		waittag(tagId[0]);

#ifdef DEBUG
		printf("%d \t %d \n",((int *)testValues)[0], ((int *)testLabels)[0]);
#endif
		// TestPoints loaded, create Objects
		Points<int, int> test_points(cb.test_points_per_transfer, cb.test_dimension, testLabels, testValues);
		
		Point<int, int> *fu = test_points.getPoint(0);
		fu->print();		
		delete fu;
		
		// now stream data and calculate all buffered test points
		streamData(test_points);
		
#ifdef PRINT
		printf("SPE%d:\ttest iteration %d finished\n", my_num, test_point_transfer);
		fflush(stdout);
#endif
	} // End loop over test points.

	// Free buffer for test point.
	free_align(testLabels);
	free_align(trainingLabels[0]);
	free_align(trainingLabels[1]);

#ifdef PRINT
	printf("SPE%d:\tended\n", my_num);
	fflush(stdout);
#endif

	return 0;
}


/*
 * 	while ((test_index = my_num + test_counter * num_spes) < max_test_count) {

		uint32_t skip = (test_index >= test_count);

		if (!skip) {

		}

		


	}*/