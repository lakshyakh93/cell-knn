using namespace std;

#include "KNN.h"
#include "Point.h"
#include "Points.h"
#include "libMnist.h"
#include <stdio.h>
#include <stdlib.h>

void imageToPoint(Point<unsigned char, int> *point, unsigned char label,
		unsigned char *image, int length) {
	for (int i = 0; i < length; ++i) {
		point->getValues()[i] = static_cast<int> (image[i]);
	}
	point->setLabel((unsigned char)label);
}

int main(int argc, char **argv) {
	cout << "---------[Program start]----------" << endl;

	if (argc < 3) {
		fprintf(stderr, "Usage: mnist <k> <mnist path>\n");
		return EXIT_FAILURE;
	}

	int k = atoi(argv[1]);

	KNN<unsigned char, int> knn(k);

	cout << "k = " << k << endl;

	string base(argv[2]);

	LabelIterator *trainLabels = openLabels(base + "train-labels-idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images-idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels-idx1-ubyte");
	ImageIterator *testImages = openImages(base + "t10k-images-idx3-ubyte");

	unsigned char label;
	unsigned char *image;
	Points<unsigned char, int> points(trainImages->count, trainImages->rows
			*trainImages->columns);
	cout << "size: " << trainImages->rows*trainImages->columns << "\tcount: "
			<< trainImages->count << endl;

	int i = 0;
	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		imageToPoint(points.getPointObject(i++), label, image,
				(trainImages->columns*trainImages->rows));

		free(image);
	}

	while (hasNextLabel(testLabels) && hasNextImage(testImages)) {
		image = nextImage(testImages);
		label = nextLabel(testLabels);

		Point <unsigned char, int> testPoint(testImages->columns*testImages->rows);
		
		imageToPoint(&testPoint, label, image, (testImages->columns*testImages->rows));

		free(image);
		
		cout << "label:\t\t" << testPoint.getLabel() << endl;
		
		knn.classify(testPoint, points);
		
		cout << "calculated label:\t" << testPoint.getLabel() << endl << endl;
	}

	//cout << points.getPointObject(333).getValues()[33];

	closeLabels(trainLabels);
	closeImages(trainImages);
	closeLabels(testLabels);
	closeImages(testImages);

	cout << "----------[Program end]-----------" << endl;
	return EXIT_SUCCESS;
}
