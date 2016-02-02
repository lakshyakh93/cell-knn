# Introduction #

This project is part of the [Praktikum für Verteilte und Parallele Systeme](https://orawww.uibk.ac.at/public_prod/owa/lfuonline_lv.details?lvnr_id_in=703701&sem_id_in=08W) course at the [University of Innsbruck](http://uibk.ac.at), Austria.

The team of this project implements a parallel version of the [k-NN algorithm](kNN.md) on IBM's Cell processor. To measure the performance of the implementation we classify hand written images contained in the [MNIST database](http://yann.lecun.com/exdb/mnist/). We run our programs on a Sony Playstation 3, kindly provided by Gregor Burger. On a Sony Playstation 3 the number of SPEs is limited to 6, instead of the possible 8 SPEs.

# Presentations #

  * You can download the introduction presentation (in german) [here](http://cell-knn.googlecode.com/files/IntroductionPresentation.pdf).
  * A presentation showing our schedule can be found [here](http://cell-knn.googlecode.com/files/Schedule.pdf). Alternatively, visit the [schedule](Schedule.md) wiki page.
  * A presentation briefly showing the results of the first parallel implementation [Cell k-NN 1.0](CellKNNv1.md) and a sketch of [Cell k-NN 2.0](CellKNNv2.md) can be found [here](http://cell-knn.googlecode.com/files/Cell%20k-NN%201.0.pdf).

# Results #

Currently, three versions of the k-NN algorithm have been implemented:
  * [Cplusplus\_kNN](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cplusplus_kNN): a _sequential_ implementation written in C++ for both x86 and Cell BE.
  * [Cell\_kNN\_v1](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cell_kNN_v1): a simple _parallel_ implementation written in C/C++ specifically for the Cell BE.
  * [Cell\_kNN\_v2](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cell_kNN_v2): a more complex _parallel_ implementation written in C/C++ specifically for the Cell BE.

See [Cell k-NN 1.0](CellKNNv1#Results.md) for more information.
See [Cell k-NN 2.0](CellKNNv2#Results.md) for more information.