Cache-Conscious Ensemble Ranking (CCER)
=======================================
Release note: [March, 2017]
-----------------

Project overview:
-----------------
A Cache-Conscious Ensemble Ranking as described in [1][2].  The package takes popular learning-to-rank datasets like Yahoo! dataset, MSLR-30K dataset and MQ2007 as the dataset input. Tree model input can be obtained from these datasets by using jforests package with LambdaMART algorithm. The package can be used to evaluate and compare the efficiency of different cache blocking methods.

Package overview:
-----------------
src/: source code files;

target/: compiled executables

results/: execution results

data/: a short version of the document dataset and tree model

Quick start:
-----------------


References:
-----------

[1] "Cache-Conscious Runtime Optimization for Ranking Ensembles". X. Tang, X. Jin, T. Yang. SIGIR'14.

[2] "A Comparison of Cache Blocking Methods for Fast Execution of Ensemble-based Score Computation". X. Jin, T. Yang, X. Tang. SIGIR'16.
