#include "libMnist.h"

#define IMAGE_OFFSET 16
#define LABEL_OFFSET 8

/** 
* @brief Converts from little endian to big endian
* 
* @param value Variable to be changed
*/
void swapBytes(int *value) {
	unsigned char *cptr, tmp;
	
	cptr = (unsigned char *) value;
	tmp = cptr[0];
	cptr[0] = cptr[3];
	cptr[3] = tmp;
	tmp = cptr[1];
	cptr[1] = cptr[2];
	cptr[2] = tmp;
}

/** 
* @brief Read an Integer from the file where fp points to
* 
* @param fp Pointer to a FILE object
* @param number Variable to store the Integer to
*/
void readInt(FILE *fp, int *number) {
	fread(number, sizeof(int), 1, fp);
	swapBytes(number);
}

/** 
* @brief Read a char from the file where fp points to
* 
* @param fp Pointer to a FILE object
* @param number Variable to store the Integer to
*/
void readChar(FILE *fp, unsigned char *number) {
	fread(number, sizeof(unsigned char), 1, fp);
}

/** 
* @brief Open an imagefile provided by the mnist-database to be processed
* 
* @param filePath Path to the image-file
* 
* @return ImageIterator pointing to the first element
*/
ImageIterator *openImages(string filePath) {
	ImageIterator *images = (ImageIterator *) malloc(sizeof(ImageIterator));
	
	images->current = 0;
	
	images->file = fopen(filePath.c_str(), "rb");
	if (images->file == NULL) {
		return NULL;
	}
	
	readInt(images->file, &images->magicNumber);
	readInt(images->file, &images->count);
	readInt(images->file, &images->rows);
	readInt(images->file, &images->columns);
	
	if (images->magicNumber != 2051) {
		fprintf(stderr, "Magic number of images file is not 2051.\n");
	}
	
	return images;
}

/** 
* @brief Close the image-file which is used by the ImageIterator in the argument-list
* 
* @param images ImageIterator pointing to the image-file
*/
void closeImages(ImageIterator *images) {
	fclose(images->file);
	free(images);
}

/** 
* @brief Resets the ImageIterator to the first position
* 
* @param images ImageIterator pointing to the image-file
*/
void resetImageIterator(ImageIterator *images) {
	fseek(images->file, IMAGE_OFFSET, SEEK_SET);
	images->current = 0;
}

/** 
* @brief Iteratorfuction used to see whether there is another element to be used
* 
* @param images ImageIterator pointing to the image-file
* 
* @return 0 if no more elements could be read \n
* 	  1 if there are elements left to read
*/
int hasNextImage(ImageIterator *images) {
	if (images->current >= images->count) {
		return 0;
	}
	
	return 1;
}

/** 
* @brief Function to read the next Image encoded in unsigned chars
* 
* @param images ImageIterator pointing to the image-file
* 
* @return Pointer to unsigned char values, representing pixel values
*/
unsigned char *nextImage(ImageIterator *images) {	
	unsigned char *image = (unsigned char *) malloc(images->columns * images->rows * sizeof(unsigned char));
	
	int i;
	unsigned char buffer;
	for (i = 0; i < (images->columns * images->rows); i++) {
		readChar(images->file, &buffer);
		image[i] = buffer;
	}
	
	images->current++;
	
	return image;
}

/**
* @brief Open an labelfile provided by the mnist-database to be processed
*
* @param filePath Path to the label-file
*
* @return LabelIterator pointing to the first element
*/
LabelIterator *openLabels(string filePath) {
	LabelIterator *labels = (LabelIterator *) malloc(sizeof(LabelIterator));
	
	labels->current = 0;
	
	labels->file = fopen(filePath.c_str(), "r");
	if (labels->file == NULL) {
		return NULL;
	}
	
	readInt(labels->file, &labels->magicNumber);
	readInt(labels->file, &labels->count);

	if (labels->magicNumber != 2049) {
		fprintf(stderr, "Magic number of images file is not 2049.\n");
	}
	
	return labels;
}

/**
* @brief Resets the LabelIterator to the first position
*
* @param labels LabelIterator pointing to the label-file
*/
void resetLabelIterator(LabelIterator *labels) {
	fseek(labels->file, LABEL_OFFSET, SEEK_SET);
	labels->current = 0;
}

/**
* @brief Close the label-file which is used by the ImageIterator in the argument-list
*
* @param labels LabelIterator pointing to the label-file
*/
void closeLabels(LabelIterator *labels) {
	fclose(labels->file);
	free(labels);
}

/**
* @brief Iteratorfuction used to see whether there is another element to be used
*
* @param labels LabelIterator pointing to the label-file
*
* @return 0 if no more elements could be read \n
* 	  1 if there are elements left to read
*/
int hasNextLabel(LabelIterator *labels) {
	if (labels->current >= labels->count) {
		return 0;
	}
	
	return 1;
}

/**
* @brief Function to read the next Label
*
* @param labels ImageIterator pointing to the label-file
*
* @return unsigned char representing the label
*/
unsigned char nextLabel(LabelIterator *labels) {
	unsigned char label;
	readChar(labels->file, &label);
	
	labels->current++;
	
	return label;
}
