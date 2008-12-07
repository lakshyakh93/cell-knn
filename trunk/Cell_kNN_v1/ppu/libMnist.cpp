#include "libMnist.h"

#define IMAGE_OFFSET 16
#define LABEL_OFFSET 8
#define SIZE_OF_INT 4
#define SIZE_OF_CHAR 1

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
	fread(number, SIZE_OF_INT, 1, fp);
	//swapBytes(number); // TODO not needed on cell
}

void readChar(FILE *fp, unsigned char *number) {
	fread(number, SIZE_OF_CHAR, 1, fp);
}

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
	
#ifdef DEBUG
	if (images->magicNumber != 2051) {
		fprintf(stderr, "Magic number of images file is not 2051 it's %d.\n", images->magicNumber);
	} else {
		fprintf(stderr, "Magic number of images file is correct.\n");
	}
#endif
	
	return images;
}

void closeImages(ImageIterator *images) {
	fclose(images->file);
	free(images);
}

void resetImageIterator(ImageIterator *images) {
	fseek(images->file, IMAGE_OFFSET, SEEK_SET);
	images->current = 0;
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

LabelIterator *openLabels(string filePath) {
	LabelIterator *labels = (LabelIterator *) malloc(sizeof(LabelIterator));
	
	labels->current = 0;
	
	labels->file = fopen(filePath.c_str(), "r");
	if (labels->file == NULL) {
		return NULL;
	}
	
	readInt(labels->file, &labels->magicNumber);
	readInt(labels->file, &labels->count);

#ifdef DEBUG
	if (labels->magicNumber != 2049) {
		fprintf(stderr, "Magic number of label file is not 2049 it's %d.\n", labels->magicNumber);
	} else {
		fprintf(stderr, "Magic number of label file is correct.\n");
	}
#endif
	
	return labels;
}

void resetLabelIterator(LabelIterator *labels) {
	fseek(labels->file, LABEL_OFFSET, SEEK_SET);
	labels->current = 0;
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
