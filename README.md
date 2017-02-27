Cache-Conscious Ensemble Ranking (CCER)
=======================================
Release note: [March, 2017]
-----------------

Project overview:
-----------------
A Cache-Conscious Ensemble Ranking as described in [1][2].  The package takes popular learning-to-rank datasets like Yahoo! dataset, MSLR-30K dataset and MQ2007 as the dataset input. Tree model input can be obtained from these datasets by using jforests[3] package with LambdaMART algorithm. The package can be used to evaluate and compare the efficiency of different cache blocking methods and some well known ensemble ranking methods like VPred[4].

Package overview:
-----------------
src/: source code files;

scripts/: scripts to compile codes, run executables and evaluate results

target/: compiled executables

results/: execution results

data/: a short version of the document dataset and tree model

Quick start:
-----------------


References:
-----------

[1] "Cache-Conscious Runtime Optimization for Ranking Ensembles". X. Tang, X. Jin, T. Yang. SIGIR'14.

[2] "A Comparison of Cache Blocking Methods for Fast Execution of Ensemble-based Score Computation". X. Jin, T. Yang, X. Tang. SIGIR'16.

[3] "Bagging Gradient-Boosted Trees for High Precision". Y. Ganjisaffar, R. Caruana, C. Lopes. SIGIR'11.

[4] "Runtime Optimizations for Tree-Based Machine Learning Models". N. Asadi, J. Lin, and A. P. D. Vries. IEEE TKDE'13.
