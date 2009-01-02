#ifndef INCLUDE_H_
#define INCLUDE_H_

#define BUFFER_MAX_SIZE (64 * 1024)   // Size of Buffer (in Bytes)
#define DMA_MAX_SIZE 16384 // Maximum Size (in Bytes)

#define waittag(tag_id) mfc_write_tag_mask(1<<tag_id);	mfc_read_tag_status_all();

#endif /*INCLUDE_H_*/
