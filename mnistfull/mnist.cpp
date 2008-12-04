#include "KNN.h"
#include "Point.h"
#include "Points.h"
#include "libMnist.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void imageToPoint(Point<int, int> *point, unsigned char label,
		unsigned char *image) {
	for (int i = 0; i < point->getDimension(); ++i) {
		point->getValues()[i] = static_cast<int> (image[i]);
	}
	point->setLabel(static_cast<int> (label));
}

int main(int argc, char **argv) {
	cout << "---------[Program start]----------" << endl;

	if (argc < 3) {
		fprintf(stderr, "Usage: mnist <k> <mnist path>\n");
		return EXIT_FAILURE;
	}

	int k = atoi(argv[1]);

	KNN<int, int> knn(k);

	cout << "k = " << k << endl;

	string base(argv[2]);
	//base = "../"; // TODO delete

	LabelIterator *trainLabels = openLabels(base + "train-labels.idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images.idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels.idx1-ubyte");
	ImageIterator *testImages = openImages(base + "t10k-images.idx3-ubyte");

	unsigned char label;
	unsigned char *image;
	Points<int, int> points(trainImages->count, trainImages->rows
			* trainImages->columns);
	cout << "size: " << trainImages->rows * trainImages->columns << "\tcount: "
			<< trainImages->count << endl;

	int i = 0;

	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		imageToPoint(points.getPointObject(i++), label, image);

		free(image);
	}

	int all = 0, good = 0, temp;
	while (hasNextLabel(testLabels) && hasNextImage(testImages)) {
		image = nextImage(testImages);
		label = nextLabel(testLabels);

		Point<int, int> testPoint(testImages->columns * testImages->rows);

		imageToPoint(&testPoint, label, image);

		free(image);

		//cout << "label:\t\t" << testPoint.getLabel() << endl;
		temp = testPoint.getLabel();

		knn.classify(testPoint, points);

		//cout << "calculated label:\t" << testPoint.getLabel() << endl << endl;
		++all;
		if (temp == testPoint.getLabel())
			++good;
		cout << "Calculated :\t" << all << "\tGood (%) :\t" << (100.0 * good
				/ all) << endl;
	}

	//cout << points.getPointObject(333).getValues()[33];

	closeLabels(trainLabels);
	closeImages(trainImages);
	closeLabels(testLabels);
	closeImages(testImages);

	cout << "----------[Program end]-----------" << endl;
	return EXIT_SUCCESS;
}
