#define SPU

#include <libmisc.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

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

double distance(Point<int, int> &testPoint,	Point<int, int> &trainPoint) {
	double result = 0.0;
	
	#ifndef SIMD
	double sum, temp;
	for (int i = 0; i < testPoint.getDimension(); ++i) {
		temp = static_cast<double>(testPoint.getValues()[i] - trainPoint.getValues()[i]);
		sum += temp * temp;
	}
	result = sum;
	#endif
	
	#ifdef SIMD
	float sum;
	for (int i = 0; i < testPoint.getDimension() / 4; i += 4) {
		vector signed int distVec;
		vector float distVecF;
		
		vector signed int trainVec = (vector signed int) {trainPoint.getValues()[i], trainPoint.getValues()[i + 1], trainPoint.getValues()[i + 2], trainPoint.getValues()[i + 3]};
		vector signed int testVec = (vector signed int) {testPoint.getValues()[i], testPoint.getValues()[i + 1], testPoint.getValues()[i + 2], testPoint.getValues()[i + 3]};
		
		distVec = spu_sub(trainVec, testVec);
		distVecF = spu_convtf(distVec, 0);
		distVecF = spu_mul(distVecF, distVecF);
		
		for (int k = 0; k < 4; k++) {
			sum += spu_extract(distVecF, k);
		}
	}
	result = (double) sum;
	#endif

	return result;
}

uint32_t calculate(Points<int, int> &test_points, Points<int, int> &training_points,
		double *distances, int *labels) {
	//iterate over test_points
	//calculate temporary knn over training points (~30 points)
	//store a sorted list for each testpoint sortedlists (=> created in streamdata or globally)
	// => cb.test_points_per_transfer sorted lists
	//int label = KNN<int, int>::classify(test_points, training_points, *list);
	
	//printf("SPE%d: label = %d", my_num, label);
	
	for (int j = 0; j < test_points.getCount(); j++) {
		Point<int, int> *testPoint =  test_points.getPoint(j);
		Point<int, int> *trainPoint;
		
		for (int i = 0; i < training_points.getCount(); ++i) {
			trainPoint = training_points.getPoint(i);
			double d = distance(*testPoint, *trainPoint);
			//printf("SPE%d: distance = %lf\n", my_num, d);
			//sortedlist.insert(d, trainPoint->getLabel());
			
			if (distances[j] == -1.0 || distances[j] > d) {
				distances[j] = d;
				labels[j] = trainPoint->getLabel();
			}
			
			delete trainPoint;
		}
		
		delete testPoint;
	}
	
	//return KNN<L, T>::majorityVote(sortedlist);
	
	return 0;
}

uint32_t streamData(Points<int, int> &test_points) {
	double *distances = (double *) malloc_align(test_points.getCount() * sizeof(double), 7);
	int *labels = (int *) malloc_align(test_points.getCount() * sizeof(int), 7);
	
	for (int i = 0; i < test_points.getCount(); i++) {
		distances[i] = -1.0;
		labels[i] = -1;
	}
	
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
		printf("SPE%d:\tTransfer %d on Buffer 0 complete\n", my_num, iteration);
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
			calculate(test_points,training_points0, distances, labels);

		// check if dma1 finished
		if (size_data > 0) //TODO check condition
			waittag(tagId[1]);

#ifdef PRINT
		printf("SPE%d:\tTransfer %d on Buffer 1 complete\n", my_num, iteration);
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
			printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration+1);
			fflush(stdout);
#endif
		}

		// TrainingPoints0 loaded, create Object
		Points<int, int> training_points1(cb.training_points_per_transfer, cb.training_dimension, trainingLabels[1], trainingValues[1]);
		
			// do calcualtions on buffer
			calculate(test_points, training_points1, distances, labels);

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
			printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration+1);
			fflush(stdout);
#endif
		}

		// increase iteration counter
		iteration++;
	}
	
	for (int i = 0; i < test_points.getCount(); i++) {
		int bad = labels[i] != test_points.getPoint(i)->getLabel();
		
		printf("SPE%d: given label = %d, resulting label = %d (bad = %d)\n",
				my_num, labels[i], test_points.getPoint(i)->getLabel(), bad);
		fflush(stdout);
	}
	
	free_align(distances);
	free_align(labels);
	
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
	
	uint32_t range = cb.test_count/cb.num_spes;	
	uint32_t start = my_num * range;
	uint32_t end = start+range-1;
	
	uint32_t rest = cb.test_count%cb.num_spes;
	if(rest!=0)
	{
		if (my_num > 0) {
			if(my_num<rest)
			{
				start+=my_num;
				end+=my_num;
				
				if (my_num < cb.test_count%cb.num_spes) {
					range++;
					end++;
				}
			}
			else
			{
				start+=rest;
				end+=rest;
			}
		} else {
			range++;
			end++;
		}
	}
	
	bool skip = false;
	
	for (uint32_t test_point_transfer = 0, index = start; test_point_transfer < test_point_transfers_per_spu; 
			test_point_transfer++, index += cb.test_points_per_transfer) {
#ifdef PRINT
		printf("SPE%d:\ttest iteration %d started\n", my_num, test_point_transfer);
		fflush(stdout);
#endif
		
#ifdef DEBUG
		printf("%llu \t %llu \n",ea_test_points,cb.ea_test_points);
#endif
		
		// Check if we would exceed end index.
		if (index + cb.test_points_per_transfer >= end) {
			cb.test_points_per_transfer = end - index;
		}
		
		Points<int, int> *test_points = 0;
		
		if (index <= end) 
		{
			// fill test point and label buffer
			if (cb.test_points_per_transfer * cb.values_size != fillBuffer(testValues, tagId[0], 
					cb.ea_test_points + index * cb.values_size,
					cb.test_points_per_transfer * cb.values_size)) {
				printf("Error occured at transfering test points");
				fflush(stdout);
				return -1;
			}
	
			if (cb.test_points_per_transfer * cb.label_size != fillBuffer(testLabels, tagId[0], 
					cb.ea_test_labels + index * cb.label_size,
					cb.test_points_per_transfer * cb.label_size)) {
				printf("Error occured at transfering test labels");
				fflush(stdout);
				return -1;
			}
	
			waittag(tagId[0]);
	
	#ifdef DEBUG
			printf("Adresses = %d \t %d \n",((int *)testValues)[0], ((int *)testLabels)[0]);
	#endif
			// TestPoints loaded, create Objects
			test_points = new Points<int, int>(cb.test_points_per_transfer, cb.test_dimension, testLabels, testValues);
			
			//Point<int, int> *fu = test_points->getPoint(0);
			//fu->print();		
			//delete fu;
		
		// now stream data and calculate all buffered test points
		}
		else
		{
			test_points = new Points<int, int>(0, 0, testLabels, testValues);
			printf("%d SKIP\n", my_num);
		}
		
		streamData(*test_points);
		
		delete test_points;
		
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