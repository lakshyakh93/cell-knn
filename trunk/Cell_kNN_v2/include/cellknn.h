#ifndef CELLKNN_H_
#define CELLKNN_H_

#define ALIGNMOD 16
#define TRAINING_VALUES_MAX_SIZE (64 * 1024)   // Size of Buffer (in Bytes)
#define TEST_VALUES_MAX_SIZE (8 * 1024)   // Size of Buffer (in Bytes)
#define DMA_MAX_SIZE 16384 // Maximum Size (in Bytes)
#define MAX_NUM_SPES 8

#define DEBUG
#define PRINT


#define waittag(tag_id) mfc_write_tag_mask(1<<tag_id);	mfc_read_tag_status_all();

typedef struct {
	uint32_t k; 
	uint32_t num_spes;
	uint32_t values_size;
	uint32_t label_size;
	// 16
	
	uint32_t training_dimension;
	uint32_t training_count;
	uint32_t training_data_size;
	uint32_t training_points_per_transfer;
	// 16
	
	uint32_t test_dimension;
	uint32_t test_count;
	uint32_t test_data_size;
	uint32_t test_points_per_transfer;
	// 16
	
	uint64_t ea_training_points;
	uint64_t ea_training_labels;	
	uint64_t ea_test_points;
	uint64_t ea_test_labels;
	// 32
	
	uint64_t spu_mfc_ctl[MAX_NUM_SPES];
	uint64_t spu_ls[MAX_NUM_SPES];
	uint64_t spu_sig1[MAX_NUM_SPES];
	uint64_t spu_sig2[MAX_NUM_SPES];
	// 4 * 8 * 8 = 256
	
	char padding[128 - 16 - 2 * 16 - 32];
} CONTROL_BLOCK; // => sizeof(CONTROL_BLOCK) == 3 * 128!!!


#endif /*CELLKNN_H_*/
