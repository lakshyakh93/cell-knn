The goal of this project is to implement an efficient implementation of the k-NN algorithm on IBM's Cell processor.

The _k-nearest neighbor algorithm_ (k-NN) is a simple algorithm for machine learning. The algorithm projects objects to a multidimensional feature space. It then classifies objects based on the closest (according to a distance metric) training examples in the feature space. An object is classified by a majority vote of its neighbors, with the object being assigned to the class most common amongst its _k_ nearest neighbors.

To evaluate the performance of our implementation(s) we classify elements in the [MNIST database](http://yann.lecun.com/exdb/mnist/), which consists of up to 60000 images of handwritten digits.