using namespace std;

#include "KNN.h"
#include "Point.h"
#include "Points.h"
#include "libMnist.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG

void imageToPoint(Point<int, int> *point, unsigned char label,
		unsigned char *image, int length) {
	for (int i = 0; i < length; ++i) {
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

	LabelIterator *trainLabels = openLabels(base + "train-labels-idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images-idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels-idx1-ubyte");
        ImageIterator *testImages = openImages(base + "t10k-images-idx3-ubyte");

        if ( trainLabels == 0 || trainImages == 0 || testLabels == 0 || testImages == 0 )
        {
            cout << "One or more files couldn't be found! Make shure that following files are in the directory given as argument:\n"
                    << "train-labels-idx1-ubyte\n"
                    << "train-images-idx3-ubyte\n"
                    << "t10k-labels-idx1-ubyte\n"
                    << "t10k-images-idx3-ubyte\n"
                    << endl;
            exit(-1);
        }

	unsigned char label;
	unsigned char *image;
	Points<int, int> points(trainImages->count, trainImages->rows
			*trainImages->columns);
	cout << "size: " << trainImages->rows*trainImages->columns << "\tcount: "
			<< trainImages->count << endl;

	int i = 0;
	Point<int, int> *trainPoint;
	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		trainPoint = points.getPoint(i++);
		imageToPoint(trainPoint, label, image,
				(trainImages->columns*trainImages->rows));

		delete trainPoint;
		free(image);
	}

#ifdef DEBUG
	int temp, all = 0, good = 0;
#endif
	while (hasNextLabel(testLabels) && hasNextImage(testImages)) {
		image = nextImage(testImages);
		label = nextLabel(testLabels);

		Point <int, int> testPoint(testImages->columns*testImages->rows);
		
		imageToPoint(&testPoint, label, image, (testImages->columns*testImages->rows));

		free(image);
		
#ifdef DEBUG
		temp = testPoint.getLabel();
		cout << "label:\t" << testPoint.getLabel();
#endif
		
		knn.classify(testPoint, points);
		
#ifdef DEBUG
		cout << "\tcalculated:\t" << testPoint.getLabel();
		++all;
		if (temp == testPoint.getLabel()) ++good;
		cout << "\t#:\t" << all << "\tgood:\t" << (100.0 * good / all) << endl;
#endif
	}

	//cout << points.getPoint(333).getValues()[33];

	closeLabels(trainLabels);
	closeImages(trainImages);
	closeLabels(testLabels);
	closeImages(testImages);

	cout << "----------[Program end]-----------" << endl;
	return EXIT_SUCCESS;
}
