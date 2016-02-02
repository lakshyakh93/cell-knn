# Cell k-NN 2.0 #

Considering the results obtained from analyzing Cell k-NN 1.0, we want to realize the following optimizations:
  * Extend program to use SIMD (Single Instruction Multiple Data) features, i.e. calculate distance of 4 dimensions in one cycle.
  * Implement pipelining approach as described in section [Pipelining](#Pipelining.md) to avoid bottleneck when accessing data.
  * Implement different distribution paradigm (parallelization over test points).

## Pipelining ##

![http://cell-knn.googlecode.com/files/Pipelining.png](http://cell-knn.googlecode.com/files/Pipelining.png)


## Results ##

In the code repository you can find the final version of our parallel k-NN implementation:
  * [Cell\_kNN\_v2](http://code.google.com/p/cell-knn/source/browse/#svn/trunk/Cell_kNN_v2): a _parallel_ implementation using the pipelining approach as described above.

Compared to version 1.0 and the sequential implementation we now have a drastic performance increase. See the table below for the time needed classifying 10000 test points using 60000 training points with both the sequential and the parallel (version 2.0) k-NN implementation.

| **Version** | **Overall Time** | **Time Per Test Point** | Speedup compared to Seq. V. |
|:------------|:-----------------|:------------------------|:----------------------------|
| Cell k-NN Sequential | 12 000s          | 1.2s                    | 1                           |
| Cell k-NN 1.0 | 5 200 000s       | 520s                    | 0.0023                      |
| Cell k-NN 2.0 | 634s             | 0.0634s                 | 18.93                       |

As you can see, we have a speedup of 18.93 comparing the parallel to the sequential implementation.