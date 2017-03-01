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
1) Clone the repository as: git clone https://github.com/LinChai/CCER.git

2) Make sure GCC and G++(C++11) compilers are installed in the system.

3) To compile
<pre><code>make</code></pre>
all the executables will be written to target/ directory.

4) To run:
<pre><code>cd scripts/; ./run.sh</code></pre>
the execution results will be written to results/ directory.

Dataset Format:
------------
1. Document dataset has to be in the following format:
<pre><code>100 46
0 qid:7968 1:0.000000 2:0.000000 3:0.000000 4:0.000000 5:0.000000 6:0.000000 7:0.000000 8:0.000000 9:0.000000 10:0.000000 11:0.000000 12:0.000000 13:0.000000 14:0.000000 15:0.000000 16:0.005175 17:0.000000 18:0.181818 19:0.000000 20:0.003106 21:0.000000 22:0.000000 23:0.000000 24:0.000000 25:0.000000 26:0.000000 27:0.000000 28:0.000000 29:0.000000 30:0.000000 31:0.000000 32:0.000000 33:0.000000 34:0.000000 35:0.000000 36:0.000000 37:0.000000 38:0.000000 39:0.000000 40:0.000000 41:0.000000 42:0.000000 43:0.055556 44:0.000000 45:0.000000 46:0.000000</code></pre>
The first line of the document dataset should give the number of documents in total (100 in the example) and the number of features per document (46 in the example). After the 1st line, each line represents one document with format as \<relevence score, query id, feature1:value1, feature2:value2...>
2. Tree model has to be in the following format:
<pre><code>
20000
5
root 0 107 8.667834589086247
node 1 0 133 1 0.0
node 2 0 133 0 0.0
node 3 1 129 1 267.0212212880563
node 4 1 133 0 1.3761520572681523
node 5 2 129 1 547.93404460018
node 6 2 133 0 1.4677659502498392
node 7 3 129 1 -1.2362906645269616
node 8 3 129 0 -0.10613791499626093
node 9 5 129 1 123.99706752340572
node 10 5 129 0 0.4260183857508339
node 11 9 129 1 -1.1016719224496192
node 12 9 18 0 13.607107787401574
node 13 12 18 1 -1.1315252228116115
node 14 12 18 0 -0.5125199927634682
end
</code></pre>
The first line denotes total number of the trees(20000 in this example). The second line is the height of the tree(5 in this example); Then each following line is of format \<node, #nodeid, #parentid, #featureid, #isleft, #threshold>. "end" denotes the end of current tree.

References:
-----------

[1] "Cache-Conscious Runtime Optimization for Ranking Ensembles". X. Tang, X. Jin, T. Yang. SIGIR'14.

[2] "A Comparison of Cache Blocking Methods for Fast Execution of Ensemble-based Score Computation". X. Jin, T. Yang, X. Tang. SIGIR'16.

[3] "Bagging Gradient-Boosted Trees for High Precision". Y. Ganjisaffar, R. Caruana, C. Lopes. SIGIR'11.

[4] "Runtime Optimizations for Tree-Based Machine Learning Models". N. Asadi, J. Lin, and A. P. D. Vries. IEEE TKDE'13.
