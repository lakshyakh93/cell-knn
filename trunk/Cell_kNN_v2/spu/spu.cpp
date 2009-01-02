#include <libmisc.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#include "spu_mfcio_ext.h"

#define BUFFER_MAX_SIZE 65536   // Size of Buffer (in Bytes)
#define DMA_MAX_SIZE 16384 // Maximum Size (in Bytes)

#define waittag(tag_id) mfc_write_tag_mask(1<<tag_id);	mfc_read_tag_status_all();

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
	return 0;
}

int main( )
{
	uint32_t ea_h, ea_l, tagId[2];
	uint64_t ea_mfc_next, ea_mfc_prev, ea_ls_prev, ea_points;
	
	char **buffer;

	printf("S<SPE %d: starts\n",my_num); fflush(stdout);
	
	if ((tagId[0]= mfc_tag_reserve())==MFC_TAG_INVALID){
		printf("SPE: ERROR can't allocate tag ID\n"); return -1;
	}
	if ((tagId[1]= mfc_tag_reserve())==MFC_TAG_INVALID){
		printf("SPE: ERROR can't allocate tag ID\n"); return -1;
	}
	
	// STEP 1: read from PPE my number using BLOCKING mailbox read	
	while( spu_stat_in_mbox()<=0 );
	my_num   = spu_read_in_mbox(); //printf("1}SPE%d<-PPE: <%u>\n",my_num,my_num); fflush(stdout);
	num_spes = spu_read_in_mbox();
	uint32_t size_data	= spu_read_in_mbox(); // Total number of Bytes to read
	const uint32_t size_buffer	= spu_read_in_mbox(); // Size of Buffers
	

	buffer[0] = (char *) malloc_align(size_buffer, 7);
	buffer[1] = (char *) malloc_align(size_buffer, 7);

	// STEP 2: receive from PPE the effective address of previous SPE's MFC area and previous SPE's LS
	//             use BLOCKING mailbox
	
	// SPE0 gets address of points from PPE
	if (my_num == 0) {
		ea_h = spu_read_in_mbox(); // read EA lower bits 
		ea_l = spu_read_in_mbox(); // read EA higher bits 

		ea_points = mfc_hl2ea( ea_h, ea_l);
	}
	
	// TODO get data by mfc command => faster ?
	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_mfc_next = mfc_hl2ea( ea_h, ea_l);

	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_mfc_prev = mfc_hl2ea( ea_h, ea_l);

	ea_h = spu_read_in_mbox(); // read EA lower bits 
	ea_l = spu_read_in_mbox(); // read EA higher bits 

	ea_ls_prev = mfc_hl2ea( ea_h, ea_l);

	//*******new stuff****************
	// SPEs (except SPE0) need to get address for buffer
	if (my_num > 0) {
		// adds local address of buffer to global address of SPU
		ea_points = ea_ls_prev + spu_read_in_mbox();
	}

	uint32_t transfer, iteration = 0, i = 0;
	while (size_data > 0 || iteration < 2) {
	
		// decide amount to transfer
		transfer = (size_buffer < size_data) ? size_buffer : size_data;
		// transfer data
		if (transfer != fillBuffer(buffer[i], tagId[i], ea_points, transfer)) {
			printf("Error occured at transfer");
			return -1;
		}
		// adjust remaining amount of data
		size_data -= transfer;
		
		// adjust data source
		if (my_num == 0) {
			ea_points += transfer;
		} else {
			// adds local address of buffer to global address of SPU
			ea_points = ea_ls_prev + spu_read_in_mbox();
		}

		// switch buffer
		i = (i == 0) ? 1 : 0;
		
		// check if setup iterations are already done
		// TODO check "iteration" conditions
		// TODO error if size_data < buffer (no switching required)
		if (iteration > 0) {
			// check if other dma finished ("i" is already switched)
			waittag(tagId[i]);
			// send next SPU the address of filled buffer
			if (my_num != num_spes-1) {
				if (write_in_mbox((uint32_t)buffer[i], ea_mfc_next, tagId[i])!=1){
					printf("SPE %d: fail to send buffer address to other SPE\n",my_num);fflush(stdout); return -1;
				}
			}
			// send copy confirmation to previous SPU
			if (my_num != 0) {
				//printf("----2}SPE%d->SPE%d: <%x>\n",my_num,my_num+1,i);fflush(stdout);
				if (write_in_mbox( i, ea_mfc_prev, tagId[i])!=1){
					printf("SPE %d: fail to send MBOX to other SPE\n",my_num);fflush(stdout); return -1;
				}
			}
			// do calcualtions on buffer
			calculate(buffer[i]);
			// check if buffer is already copied (may be overwritten before it's copied otherwise)
			if (my_num != num_spes-1) {
				if (i != spu_read_in_mbox()) {
					printf("Error");
					return -1;
				}
			}
		} else {
			iteration++;
		}
	}

	free_align(buffer[0]);
	free_align(buffer[1]);

	printf("E<SPE %d: ended\n",my_num);	
	
	return 0;
}
