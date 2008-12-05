#ifndef LIBMNIST_H_
#define LIBMNIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

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

ImageIterator *openImages(string filePath);
void resetImageIterator(ImageIterator *images);
void closeImages(ImageIterator *images);
int hasNextImage(ImageIterator *images);
unsigned char *nextImage(ImageIterator *images);

LabelIterator *openLabels(string filePath);
void resetLabelIterator(LabelIterator *images);
void closeLabels(LabelIterator *labels);
int hasNextLabel(LabelIterator *labels);
unsigned char nextLabel(LabelIterator *labels);

#endif
