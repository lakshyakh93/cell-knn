using namespace std;

#include "KNN.h"
#include "Point.h"
#include "Points.h"
#include "libMnist.h"
#include <stdio.h>
#include <stdlib.h>

#define PRETTY_PRINT
#define DEBUG

/**
 * @brief Function to write the data of an image to a Point-class
 *
 * @param point Point to write to
 * @param label Label to set for the point
 * @param image Image from which the data comes
 * @param length Size of an image
 */
void imageToPoint(Point<int, int> *point, unsigned char label,
		unsigned char *image, int length) {
	for (int i = 0; i < length; ++i) {
		point->getValues()[i] = static_cast<int> (image[i]);
	}
	point->setLabel(static_cast<int> (label));
}

/**
 * @brief Entryfunction for this program parameters are: \n
 *       int k: Amount of neighbours used to classify the Image \n
 *       string testpath: Directory in which the mnist-files are
 *
 * @return 0 if the program terminated normaly \n
 *       -1 else
 */
int main(int argc, char **argv) {
#ifdef PRETTY_PRINT
	cout << "---------[Program start]----------" << endl;
#endif

	if (argc < 3) {
		fprintf(stderr, "Usage: mnist <k> <mnist path> [<nr train images> <nr test images>]\n");
		return EXIT_FAILURE;
	}

	int k = atoi(argv[1]);

	KNN<int, int> knn(k);

#ifdef PRETTY_PRINT
	cout << "k = " << k << endl;
#endif

	string base(argv[2]);

	LabelIterator *trainLabels = openLabels(base + "train-labels-idx1-ubyte");
	ImageIterator *trainImages = openImages(base + "train-images-idx3-ubyte");
	LabelIterator *testLabels = openLabels(base + "t10k-labels-idx1-ubyte");
	ImageIterator *testImages = openImages(base + "t10k-images-idx3-ubyte");

	if (trainLabels == 0 || trainImages == 0 || testLabels == 0 || testImages
			== 0) {
		cout
				<< "One or more files couldn't be found! Make shure that following files are in the directory given as argument:\n"
				<< "train-labels-idx1-ubyte\n" << "train-images-idx3-ubyte\n"
				<< "t10k-labels-idx1-ubyte\n" << "t10k-images-idx3-ubyte\n"
				<< endl;
		exit(-1);
	}

	if (argc >= 5) {
		trainLabels->count = atoi(argv[3]);
		trainImages->count = atoi(argv[3]);
		testLabels->count = atoi(argv[4]);
		testImages->count = atoi(argv[4]);
	}

	unsigned char label;
	unsigned char *image;

	Points<int, int> points(trainImages->count, trainImages->rows
			*trainImages->columns);

#ifdef PRETTY_PRINT
	cout << trainImages->count << " " << trainImages->rows*trainImages->columns
			<< endl;
	cout << "size: " << trainImages->rows*trainImages->columns << "\tcount: "
			<< trainImages->count << endl;
#endif

	int i = 0;
	Point<int, int> *trainPoint;
	while (hasNextLabel(trainLabels) && hasNextImage(trainImages)) {
		image = nextImage(trainImages);
		label = nextLabel(trainLabels);

		trainPoint = points.getPoint(i++);
		imageToPoint(trainPoint, label, image, (trainImages->columns
				*trainImages->rows));

#ifdef PRETTY_PRINT
		if ((i % 1000) == 0)
			cout << i << " Images loaded." << endl;
#endif

		delete trainPoint;
		free(image);
	}

#ifdef PRETTY_PRINT
	int temp, all = 0, good = 0;
#endif
	while (hasNextLabel(testLabels) && hasNextImage(testImages)) {
		image = nextImage(testImages);
		label = nextLabel(testLabels);

		Point <int, int> testPoint(testImages->columns*testImages->rows);

		imageToPoint(&testPoint, label, image, (testImages->columns
				*testImages->rows));

		free(image);

#ifdef PRETTY_PRINT
		temp = testPoint.getLabel();
#endif

		knn.classify(testPoint, points);

#ifdef PRETTY_PRINT
		cout << "label:\t" << temp << "\tcalculated:\t" << testPoint.getLabel();
		++all;
		if (temp == testPoint.getLabel())
			++good;
		cout << "\t#:\t" << all << "\tgood:\t" << (100.0 * good / all) << endl;
#endif
	}

	closeLabels(trainLabels);
	closeImages(trainImages);
	closeLabels(testLabels);
	closeImages(testImages);

	cout << "----------[Program end]-----------" << endl;
	return EXIT_SUCCESS;
}
