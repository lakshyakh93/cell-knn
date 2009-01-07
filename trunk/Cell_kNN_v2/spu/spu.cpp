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

uint32_t my_num, num_spes;
volatile char buffer0[BUFFER_MAX_SIZE] __attribute__((aligned(128)));
volatile char buffer1[BUFFER_MAX_SIZE] __attribute__((aligned(128)));

uint32_t fillBuffer(volatile char *buffer, uint32_t tagId, uint64_t address, uint32_t transfer) {
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

uint32_t calculate(volatile char *buffer) {
	int i, temp=0;

	for (i = 0; i < 768*4; i++)
		temp+=buffer[i];

	printf("%d\n", temp);

	return 0;
}

int main() {
	uint32_t ea_h, ea_l, tagId[2];
	uint64_t ea_mfc_next, ea_sig2_prev, ea_ls_prev, ea_points;

	char **buffer;

	// STEP 1: read from PPE my number using BLOCKING mailbox read	
	while (spu_stat_in_mbox()<=0)
		;
	my_num = spu_read_in_mbox();
	num_spes = spu_read_in_mbox();
	uint32_t size_data = spu_read_in_mbox(); // Total number of Bytes to read
	const uint32_t size_buffer = spu_read_in_mbox(); // Size of Buffers

#ifdef PRETTY_PRINT
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

	//	printf("%d bla %d\n",buffer0,buffer1);

	// STEP 2: receive from PPE the effective address of previous SPE's MFC area and previous SPE's LS
	//             use BLOCKING mailbox

	// SPE0 gets address of points from PPE
	if (my_num == 0) {
		ea_h = spu_read_in_mbox(); // read EA lower bits 
		ea_l = spu_read_in_mbox(); // read EA higher bits 

		ea_points = mfc_hl2ea(ea_h, ea_l);
	}

	// TODO get data by mfc command => faster ?
	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_mfc_next = mfc_hl2ea(ea_h, ea_l);

	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_sig2_prev = mfc_hl2ea(ea_h, ea_l);

	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_ls_prev = mfc_hl2ea(ea_h, ea_l);

#ifdef PRETTY_PRINT
	printf("SPE%d:\tSetup complete\n", my_num);
	fflush(stdout);
#endif

	//*******new stuff****************
	// SPEs (except SPE0) need to get address for buffer
	if (my_num > 0) {
		// adds local address of buffer to global address of SPU
		ea_points = ea_ls_prev + spu_read_in_mbox();
	}

	uint32_t transfer[2], iteration = 0;
	while (size_data > 0) {

		// check if setup iteration is already done
		if (iteration > 0) {
			// check if dma0 finished
			if (size_data > 0) //TODO check condition
				waittag(tagId[0]);
#ifdef PRETTY_PRINT
			printf("SPE%d:\tTransfer %d on Buffer 0 complete\n", my_num, iteration-1);
			fflush(stdout);
#endif
			// adjust remaining amount of data
			size_data -= transfer[0];
			// send next SPU the address of filled buffer
			if (my_num != num_spes-1) {
				if (write_in_mbox((uint32_t)buffer0, ea_mfc_next, tagId[0])!=1) {
					printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// send copy confirmation to previous SPU
			if (my_num != 0) {
				if (write_signal2(0, ea_sig2_prev, tagId[0])!=1) {
					printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}

			// do calcualtions on buffer
			calculate(buffer0);

			// check if dma1 finished
			if (size_data > 0) //TODO check condition
				waittag(tagId[1]);
#ifdef PRETTY_PRINT
			printf("SPE%d:\tTransfer %d on Buffer 1 complete\n", my_num, iteration-1);
			fflush(stdout);
#endif
			// adjust remaining amount of data
			size_data -= transfer[1];
			// send next SPU the address of filled buffer
			if (my_num != num_spes-1) {
				if (write_in_mbox((uint32_t)buffer1, ea_mfc_next, tagId[1])!=1) {
					printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// send copy confirmation to previous SPU
			if (my_num != 0) {
				if (write_signal2(1, ea_sig2_prev, tagId[1])!=1) {
					printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}

			// check if buffer is already copied (may be overwritten before it's copied otherwise)
			if (my_num != num_spes-1) {
				uint32_t temp;
				if ((temp = spu_read_signal2())!=0) {
					printf("SPE%d:\tconfirmation failed, received %d, wanted 0\n", my_num, temp);
					fflush(stdout);
					return -1;
				}
			}
			// decide amount to transfer
			transfer[0] = (size_buffer < size_data) ? size_buffer : size_data;
			//if there is something to transfer then
			if (transfer[0] > 0) {
				// adjust data source
				if (my_num == 0) {
					ea_points += transfer[1];
				} else {
					// adds local address of buffer to global address of SPU
					ea_points = ea_ls_prev + spu_read_in_mbox();
				}
				// transfer data
				if (transfer[0] != fillBuffer(buffer0, tagId[0], ea_points, transfer[0])) {
					printf("Error occured at transfer");
					return -1;
				}
#ifdef PRETTY_PRINT
				printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
				fflush(stdout);
#endif
			}

			// do calcualtions on buffer
			calculate(buffer1);

			// check if buffer is already copied (may be overwritten before it's copied otherwise)
			if (my_num != num_spes-1) {
				uint32_t temp;
				if ((temp = spu_read_signal2())!=1) {
					printf("SPE%d:\tconfirmation failed, received %d, wanted 1\n", my_num, temp);
					fflush(stdout);
					return -1;
				}
			}
			// decide amount to transfer
			transfer[1] = (size_buffer < size_data-transfer[0]) ? size_buffer : size_data-transfer[0];
			//if there is something to transfer then
			if (transfer[1] > 0) {
				// adjust data source
				if (my_num == 0) {
					ea_points += transfer[0];
				} else {
					// adds local address of buffer to global address of SPU
					ea_points = ea_ls_prev + spu_read_in_mbox();
				}
				// transfer data
				if (transfer[1] != fillBuffer(buffer1, tagId[1], ea_points, transfer[1])) {
					printf("Error occured at transfer");
					return -1;
				}
#ifdef PRETTY_PRINT
				printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
				fflush(stdout);
#endif
			}
		} else {
			// decide amount to transfer
			transfer[0] = (size_buffer < size_data) ? size_buffer : size_data;

			// transfer data
			if (transfer[0] != fillBuffer(buffer0, tagId[0], ea_points, transfer[0])) {
				printf("Error occured at transfer");
				return -1;
			}
#ifdef PRETTY_PRINT
			printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
			fflush(stdout);
#endif

			// adjust data source
			if (my_num == 0) {
				ea_points += transfer[0];
			} else {
				// adds local address of buffer to global address of SPU
				ea_points = ea_ls_prev + spu_read_in_mbox();
			}

			// decide amount to transfer
			transfer[1] = (size_buffer < size_data-transfer[0]) ? size_buffer : size_data-transfer[0];

			// transfer data
			if (transfer[1] != fillBuffer(buffer1, tagId[1], ea_points, transfer[1])) {
				printf("Error occured at transfer");
				return -1;
			}
#ifdef PRETTY_PRINT
			printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
			fflush(stdout);
#endif
		}

		// increase iteration counter
		iteration++; // TODO check for overflow
	}
#ifdef PRETTY_PRINT
	printf("SPE%d:\tComputations finished\n", my_num);
	fflush(stdout);
#endif

	printf("SPE%d:\tended\n", my_num);

	return 0;
}
