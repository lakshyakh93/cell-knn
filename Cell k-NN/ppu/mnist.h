#ifndef MNIST_H_
#define MNIST_H_

typedef struct {
	int magicNumber;
	int count;
	int current;
	int rows;
	int columns;
	FILE *file;
} ImageIterator;

typedef struct {
	int magicNumber;
	int count;
	int current;
	FILE *file;
} LabelIterator;

ImageIterator *openImages(char *filePath);
void closeImages(ImageIterator *images);
int hasNextImage(ImageIterator *images);
unsigned char *nextImage(ImageIterator *images);

LabelIterator *openLabels(char *filePath);
void closeLabels(LabelIterator *labels);
int hasNextLabel(LabelIterator *labels);
unsigned char nextLabel(LabelIterator *labels);

#endif
