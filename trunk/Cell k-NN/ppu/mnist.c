#include <stdio.h>
#include <stdlib.h>
#include "mnist.h"

// Converts from little endian to big endian.
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

void readInt(FILE *fp, int *number) {
	fread(number, sizeof(int), 1, fp);
	swapBytes(number);
}

void readChar(FILE *fp, unsigned char *number) {
	fread(number, sizeof(unsigned char), 1, fp);
}

ImageIterator *openImages(char *filePath) {
	ImageIterator *images = (ImageIterator *) malloc(sizeof(ImageIterator));
	
	images->current = 0;
	
	images->file = fopen(filePath, "rb");
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

void closeImages(ImageIterator *images) {
	fclose(images->file);
	free(images);
}

int hasNextImage(ImageIterator *images) {
	if (images->current >= images->count) {
		return 0;
	}
	
	return 1;
}

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

LabelIterator *openLabels(char *filePath) {
	LabelIterator *labels = (LabelIterator *) malloc(sizeof(LabelIterator));
	
	labels->current = 0;
	
	labels->file = fopen(filePath, "r");
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

void closeLabels(LabelIterator *labels) {
	fclose(labels->file);
	free(labels);
}

int hasNextLabel(LabelIterator *labels) {
	if (labels->current >= labels->count) {
		return 0;
	}
	
	return 1;
}

unsigned char nextLabel(LabelIterator *labels) {
	unsigned char label;
	readChar(labels->file, &label);
	
	labels->current++;
	
	return label;
}

/*
int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: mnist <label file> <images file>\n");
		return EXIT_FAILURE;
	}
	
	LabelIterator *labels = openLabels(argv[1]);
	
	printf("magicNumber =  %d\n", labels->magicNumber);
	printf("count =  %d\n", labels->count);
	
	unsigned char label;
	while (hasNextLabel(labels)) {
		label = nextLabel(labels);
		//printf("label = %d\n", label);
	}
	
	closeLabels(labels);
	
	ImageIterator *images = openImages(argv[2]);
	
	printf("magicNumber =  %d\n", images->magicNumber);
	printf("count =  %d\n", images->count);
	printf("columns =  %d\n", images->columns);
	printf("rows =  %d\n", images->rows);
	
	unsigned char *image;
	while (hasNextImage(images)) {
		image = nextImage(images);
		
		printf("image = ");
		
		int i;
		for (i = 0; i < (images->rows * images->columns); i++) {
			printf(" %d", image[i]);
		}
		
		printf("\n");
		
		
		free(image);
	}
	
	closeImages(images);
	
	return EXIT_SUCCESS;	
}
*/
