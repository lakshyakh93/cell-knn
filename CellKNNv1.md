# Cell k-NN 1.0 #

In the first version of the Cell k-NN program we are going to implement a runnable program which distributes the work among the SPEs. This includes
  * A program to preprocess the MNIST data, i.e. extract the "interesting" values from a data set and transform the data as required
  * A generic implementation of the k-NN algorithm.
As far as the k-NN algorithm is concerned, we are planning to implement it "as generic as possible", that means, we do not specifically adapt our implementation to the MNIST data sets. For interexchangeability we provide a generic interface consisting of header files, which we are planning to develop in cooperation with the [Cuda k-NN Team](http://homepage.uibk.ac.at/~csaf3296/pvps08w/), if possible.

The general idea is illustrated in the following image:
![http://cell-knn.googlecode.com/files/Distribution%20Simple.png](http://cell-knn.googlecode.com/files/Distribution%20Simple.png)

As you can see in the image above, an MNIST image is represented as an array of pixels with brightness values (0-255). The transformation of the original MNIST representation to a simple array representation is done in the preprocessing of the image. Then, the array is split into _n_ parts (_n_ is the number of SPEs available), for which each SPE calculates the distance of the specific part of the image to corresponding part of all other reference images. The distance measure where are going to use is the _(quadratic) euclidean distance_. The PPE then sums up all the _n_ distances returned by the SPEs and keeps track of the _k_ nearest images in a sorted list. After all distances between the reference images and the given image have been calculated, the PPE classifies the given image using the _k_ nearest reference images.

We suspect, that this approach works well for high-dimensional data sets, however, for small-dimensional sets it may be more performant if each SPE calculates the distance of a _single_ image rather than a relatively small part of a "small" image, since
  * The PPE has to compute the memory addresses for each SPE per image
  * The PPE has to sum up the distances gathered from the SPEs
  * The SPEs have to compute the distance for each dimension and calculate the part distances

Since the effort for these operations is unknown up to now, version 2.0 of the program will deal with optimization and load balancing, maybe resulting in a different implementation where each SPE calculates the total distance of a single image to a reference image.

Another problem of the first approach may occur when using other distance metrics than the euclidean distance which do not calculate the distance locally but globally.

See the following listing for a prototype of our implementation in a C-like pseudo code:

```
typedef struct {
	int *vector;
	int label;
} Point;

// SPE
int euclidean(Point *query, Point *reference, int offset, int dimension) {
	int sum = 0;
	int i;
	for (i = 0; i< dimension; i++) {
		int difference = reference->vector[i] - query->vector[i] 
		sum += difference * difference;
	}
	
	return sum;
}

// PPE
double distance(Point *query, Point *reference, int dimension) {
	int n = get_number_of_available_spe();
	int len = dimension / n;
	
	// Let each SPE calculate distance of a part of the sample and sum up distances.
	int sum = 0;
	int i;
	for (i = 0; i < n; i++) {
		int offset = i * len;
		
		// Spawn SPE process.
		sum += euclidean(query, reference, offset, len);
	}
	
	return sqrt(sum);
}

void classify(int k, Point *query, Point **training, int length, int dimension) {
	// Create sorted list of length k.
	List sorted_list;
	
	int i;
	for (i = 0; i < length; i++) {
		// Calculate distance of sample to training sample.
		double distance = distance(query, &training[i], dimension);
		
		// Insert training sample into sorted list, discarding if training sample
		// not within k nearest neighbour to query sample.
		insert(sorted_list, &training[i], distance);
	}
	
	// Do majority vote on k nearest neighbours.
	query->label = majority_vote(sorted_list);
}
```

## Results ##

In the code repository you can find various implementations of the k-NN algorithm:
  * [Cplusplu\_kNN](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cell_kNN_v1): a _sequential_ implementation written in standard C++.
  * [Cell\_kNN\_v1](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cell_kNN_v1): a simple _parallel_ implementation written for the Cell BE.

Executing the MNIST tests has shown, that the parallel version peforms drastically worse than the sequential implementation (**speedup of 0.002!**). We have identified the following possible reasons:
  * A single SPE transfers only about 0.5 kByte compared the possible 16 kByte per comparison of a single reference point to the query point. This comes from the fact that each SPE calculates the distance between a _part_ of the query and a _part_ of the reference point. The number of elements each SPE considers is given by number\_of\_dimensions / number\_of\_SPEs, which is in our case (28 x 28) / 6 = 130 = 522 Byte.
  * 60000 x number\_of\_SPEs threads are created, destroyed and joined when classifying a single query point (1 MNIST image). When classyfing all 10000 MNIST images in the test set, an overall of 60000 x number\_of\_SPEs x 10000 threads are spawned.
  * At each comparison, the query point data is loaded from the main memory into the local store of the SPE, although it would be sufficient to load the query point data once and compare it to all reference points.
  * Since all threads are started more or less at the same time, there is a simultaneous access to the main memory from all SPEs, which leads to a bottleneck at the EIB through simultaneous DMA requests on the same data.

Documentation is available at:

http://www.dps.uibk.ac.at/~csae6550/documents/seq/ for the sequential (Cplusplus\_kNN/) version

http://www.dps.uibk.ac.at/~csae6550/documents/par/ for the parallel (Cell\_kNN\_v1/) version