#include <libmisc.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#include "spu_mfcio_ext.h"
#include "include.h"

uint32_t my_num, num_spes;

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

uint32_t calculate(char *buffer) {
	/*if (my_num == 0) {
		buffer[3] = 'c';
	} else {
		if (buffer[3] == 'c') {
			printf("SPE%d:\tBuffer[3] == c\n", my_num);
			fflush(stdout);
		} else {
			printf("SPE%d:\tBuffer[3] != c\n", my_num);
			fflush(stdout);
		}
	}*/
	return 0;
}

int main() {
	uint32_t ea_h, ea_l, tagId[2];
	uint64_t ea_mfc_next, ea_mfc_prev, ea_ls_prev, ea_points;

	char **buffer;

	// STEP 1: read from PPE my number using BLOCKING mailbox read	
	while (spu_stat_in_mbox()<=0)
		;
	my_num = spu_read_in_mbox();
	num_spes = spu_read_in_mbox();
	uint32_t size_data = spu_read_in_mbox(); // Total number of Bytes to read
	const uint32_t size_buffer = spu_read_in_mbox(); // Size of Buffers

	printf("SPE%d:\tstarted\n", my_num);
	fflush(stdout);

	if ((tagId[0]= mfc_tag_reserve())==MFC_TAG_INVALID) {
		printf("SPE: ERROR can't allocate tag ID\n");
		return -1;
	}
	if ((tagId[1]= mfc_tag_reserve())==MFC_TAG_INVALID) {
		printf("SPE: ERROR can't allocate tag ID\n");
		return -1;
	}


	buffer = (char **) malloc_align(2 * sizeof(char *), 7);
	buffer[0] = (char *) malloc_align(size_buffer, 7);
	buffer[1] = (char *) malloc_align(size_buffer, 7);

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

	ea_mfc_prev = mfc_hl2ea(ea_h, ea_l);

	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_ls_prev = mfc_hl2ea(ea_h, ea_l);

	printf("SPE%d:\tSetup complete\n", my_num);
	fflush(stdout);

	//*******new stuff****************
	// SPEs (except SPE0) need to get address for buffer
	if (my_num > 0) {
		// adds local address of buffer to global address of SPU
		ea_points = ea_ls_prev + spu_read_in_mbox();
	}

	uint32_t transfer, iteration = 0;
	while (size_data > 0) {

		// check if setup iterations are already done
		// TODO check "iteration" conditions
		// TODO error if size_data < buffer (no switching required)

		if (iteration > 0) {
			// check if dma0 finished
			waittag(tagId[0]);
			printf("SPE%d:\tTransfer %d on Buffer 0 complete\n", my_num, iteration);
			fflush(stdout);
			// send next SPU the address of filled buffer
			if (my_num != num_spes-1) {
				if (write_in_mbox((uint32_t)buffer[0], ea_mfc_next, tagId[0])!=1) {
					printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// send copy confirmation to previous SPU
			if (my_num != 0) {
				if (write_in_mbox(0, ea_mfc_prev, tagId[0])!=1) {
					printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}

			// do calcualtions on buffer
			calculate(buffer[0]);

			// check if dma1 finished
			waittag(tagId[1]);
			printf("SPE%d:\tTransfer %d on Buffer 1 complete\n", my_num, iteration);
			fflush(stdout);
			// send next SPU the address of filled buffer
			if (my_num != num_spes-1) {
				if (write_in_mbox((uint32_t)buffer[1], ea_mfc_next, tagId[1])!=1) {
					printf("SPE%d:\tfail to send buffer address to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// send copy confirmation to previous SPU
			if (my_num != 0) {
				if (write_in_mbox(1, ea_mfc_prev, tagId[1])!=1) {
					printf("SPE%d:\tfail to send confirmation to other SPE\n", my_num);
					fflush(stdout);
					return -1;
				}
			}

			// check if buffer is already copied (may be overwritten before it's copied otherwise)
			if (my_num != num_spes-1) {
				if (0 != spu_read_in_mbox()) {
					printf("SPE%d:\tconfirmation failed\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// adjust data source
			if (my_num == 0) {
				ea_points += transfer;
			} else {
				// adds local address of buffer to global address of SPU
				ea_points = ea_ls_prev + spu_read_in_mbox();
			}
			// decide amount to transfer
			transfer = (size_buffer < size_data) ? size_buffer : size_data;
			// adjust remaining amount of data
			size_data -= transfer;
			// transfer data
			if (transfer != fillBuffer(buffer[0], tagId[0], ea_points, transfer)) {
				printf("Error occured at transfer");
				return -1;
			}
			printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
			fflush(stdout);

			// do calcualtions on buffer
			calculate(buffer[1]);

			// check if buffer is already copied (may be overwritten before it's copied otherwise)
			if (my_num != num_spes-1) {
				if (1 != spu_read_in_mbox()) {
					printf("SPE%d:\tconfirmation failed\n", my_num);
					fflush(stdout);
					return -1;
				}
			}
			// adjust data source
			if (my_num == 0) {
				ea_points += transfer;
			} else {
				// adds local address of buffer to global address of SPU
				ea_points = ea_ls_prev + spu_read_in_mbox();
			}
			// decide amount to transfer
			transfer = (size_buffer < size_data) ? size_buffer : size_data;
			// adjust remaining amount of data
			size_data -= transfer;
			// transfer data
			if (transfer != fillBuffer(buffer[1], tagId[1], ea_points, transfer)) {
				printf("Error occured at transfer");
				return -1;
			}
			printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
			fflush(stdout);

		} else {
			// decide amount to transfer
			transfer = (size_buffer < size_data) ? size_buffer : size_data;
			// adjust remaining amount of data
			size_data -= transfer;

			// transfer data
			if (transfer != fillBuffer(buffer[0], tagId[0], ea_points, transfer)) {
				printf("Error occured at transfer");
				return -1;
			}
			printf("SPE%d:\tTransfer %d on Buffer 0 initiated\n", my_num, iteration);
			fflush(stdout);

			// adjust data source
			if (my_num == 0) {
				ea_points += transfer;
			} else {
				// adds local address of buffer to global address of SPU
				ea_points = ea_ls_prev + spu_read_in_mbox();
			}

			// decide amount to transfer
			transfer = (size_buffer < size_data) ? size_buffer : size_data;
			// adjust remaining amount of data
			size_data -= transfer;

			// transfer data
			if (transfer != fillBuffer(buffer[1], tagId[1], ea_points, transfer)) {
				printf("Error occured at transfer");
				return -1;
			}
			printf("SPE%d:\tTransfer %d on Buffer 1 initiated\n", my_num, iteration);
			fflush(stdout);

		}

		// increase iteration counter
		iteration++; // TODO check for overflow
	}

	free_align(buffer[0]);
	free_align(buffer[1]);
	free_align(buffer);

	printf("SPE%d:\tended\n", my_num);

	return 0;
}
