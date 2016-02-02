# Basic Idea #

The k-Nearest-Neighbour (k-NN) algorithm is a simple machine learning algorithm.

The idea is to project objects to a vector space of _n_ dimensions. The algorithm compares a _query point_, i.e. an object to classify, to a set of already classified objects, often called the _training set_ (a set of _training points_). The algorithm determines the _k_ nearest neighbours in the vector space by calculating the distance of the query point to all training points according to some distance measure (in our case the quadratic [euclidean distance](http://en.wikipedia.org/wiki/Euclidean_distance)). A query point is classified by a majority vote of its neighbours, with the query point being assigned to the class most common amongst its _k_ nearest neighbors. _k_ is a positive integer, typically small.

See below an implementation of the k-NN algorithm in a Java-like pseudo code.

```
void classify(int k, Point[] querySet, Point[] trainingSet) {
    // Loop 1.
    foreach (query in querySet) {
        // Loop 2.
        foreach (training in trainingSet) {
            // Create a fixed sized sorted map of length k,
            // where we map the distance to a training point.
            SortedMap map = new SortedMap(k);
 
            // Calculate distance of test point to training point.
            double d = distance(query, training);
		
            // Insert training point into sorted list, discarding if training point
            // not within k nearest neighbours to query point.
            map.insert(d, training);
        }
  
        // Do majority vote on k nearest neighbours and
        // assign the corresponding label to the query point.
	query.label = majorityVote(map);
    }
}
 
// We use the quadratic euclidean distance.
double distance(Point query, Point reference) {
    int sum = 0;
 
    // n is the number of dimensions in the vector space.
    int n = query.vector.length;
 
    // Loop 3.
    for (int i = 0; i < n; i++) {
        int difference = reference.vector[i] - query.vector[i];
        sum += difference * difference;
    }
	
    return sum;
}
```

# Parallel implementation variants #

In the code above we have denoted the loops with parallelization potential with _Loop 1_, _Loop 2_ and _Loop 3_. According to this we can implement 3 different variants of parallel implementations.

## Variant 1 ##
Parallelize _Loop 1_, i.e. the loop over the query points. [Cell k-NN 2.0](CellKNNv2.md) follows this approach.

## Variant 2 ##
Parallelize _Loop 2_, i.e. the loop over the training points.

## Variant 3 ##
Parallelize _Loop 3_, i.e. the loop over the dimensions of the vector space to calculate the distance of a query point to a training point. [Cell k-NN 1.0](CellKNNv1.md) follows this approach.