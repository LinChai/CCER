#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "StructPlus.h"
#include "ParseCommandLine.h"

#define F 100000 // d
#define T 20000 // s
#define NUM_THREADS 1

/**
 * Driver that evaluates test instances using the StructPlus
 * implementation. Use the following command to run this driver:
 *
 * ./StructPlus -ensemble <ensemble-path> -instances <test-instances-path> \
 *              -maxLeaves <max-number-of-leaves> [-print]
 *
 */

int main(int argc, char** args) {
  if(!isPresentCL(argc, args, (char*) "-ensemble") ||
     !isPresentCL(argc, args, (char*) "-instances") ||
     !isPresentCL(argc, args, (char*) "-maxLeaves")) {
    return -1;
  }

  char* configFile = getValueCL(argc, args, (char*) "-ensemble");
  char* featureFile = getValueCL(argc, args, (char*) "-instances");
  int maxNumberOfLeaves = atoi(getValueCL(argc, args, (char*) "-maxLeaves"));
  int printScores = isPresentCL(argc, args, (char*) "-print");

  // Read ensemble
  FILE *fp = fopen(configFile, "r");
  int nbTrees;
  fscanf(fp, "%d", &nbTrees);

  // Array of pointers to tree roots, one per tree in the ensemble
  StructPlus** trees = (StructPlus**) malloc(nbTrees * sizeof(StructPlus*));
  int tindex = 0;

  // Number of nodes in a tree does not exceed (maxLeaves * 2)
  int maxTreeSize = 2 * maxNumberOfLeaves;
  long treeSize;

  for(tindex = 0; tindex < nbTrees; tindex++) {
    fscanf(fp, "%ld", &treeSize);

    trees[tindex] = createNodes(maxTreeSize);

    char text[20];
    long line = 0;
    fscanf(fp, "%s", text);
    while(strcmp(text, "end") != 0) {
      long id;
      fscanf(fp, "%ld", &id);

      // A "root" node contains a feature id and a threshold
      if(strcmp(text, "root") == 0) {
        int fid;
        float threshold;
        fscanf(fp, "%d %f", &fid, &threshold);
        setRoot(trees[tindex], id, fid, threshold);
      } else if(strcmp(text, "node") == 0) {
        int fid;
        long pid;
        float threshold;
        int leftChild = 0;
        // Read Id of the parent node, feature id, subtree (left or right),
        // and threshold
        fscanf(fp, "%ld %d %d %f", &pid, &fid, &leftChild, &threshold);

        // Find the parent node, based in parent id
        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < maxTreeSize; parentIndex++) {
          if(trees[tindex][parentIndex].id == pid) {
            break;
          }
        }
        // Add the new node
        if(trees[tindex][parentIndex].fid >= 0) {
          addNode(trees[tindex], parentIndex, line, id, leftChild, fid, threshold);
        }
      } else if(strcmp(text, "leaf") == 0) {
        long pid;
        int leftChild = 0;
        float value;
        fscanf(fp, "%ld %d %f", &pid, &leftChild, &value);

        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < maxTreeSize; parentIndex++) {
          if(trees[tindex][parentIndex].id == pid) {
            break;
          }
        }
        if(trees[tindex][parentIndex].fid >= 0) {
          addNode(trees[tindex], parentIndex, line, id, leftChild, 0, value);
        }
      }
      line++;
      fscanf(fp, "%s", text);
    }
    // Re-organize tree memory layout
    trees[tindex] = compress(trees[tindex]);
  }
  fclose(fp);

  // Read instances (SVM Light format
  int numberOfFeatures = 0;
  int numberOfInstances = 0;

  fp = fopen(featureFile, "r");
  fscanf(fp, "%d %d", &numberOfInstances, &numberOfFeatures);
  int divisibleNumberOfInstances = numberOfInstances;
  while(divisibleNumberOfInstances % F !=0)
    divisibleNumberOfInstances++;
  float** features = (float**) malloc(divisibleNumberOfInstances * sizeof(float*));
  int i = 0; int j=0;
  for(i = 0; i < divisibleNumberOfInstances; i++) {
    features[i] = (float*) malloc(numberOfFeatures * sizeof(float));
  }

  float fvalue;
  int fIndex = 0, iIndex = 0;
  int ignore;
  char text[20];
  char comment[1000];
  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {
    fscanf(fp, "%d %[^:]:%d", &ignore, text, &ignore);
    for(fIndex = 0; fIndex < numberOfFeatures; fIndex++) {
      fscanf(fp, "%[^:]:%f", text, &fvalue);
      features[iIndex][fIndex] = fvalue;
    }
    fscanf(fp, "%[^\n]", comment);
  }
  // Compute scores for instances using the ensemble and
  // measure elapsed time
  float sum = 0; // Dummy value just so gcc wouldn't optimize the loop out
  float score;
  struct timeval start, end;

  int remainder, remainder_F;
  remainder = nbTrees % T;
  remainder_F = numberOfInstances % F;
  gettimeofday(&start, NULL);
  
  //SDSD  
  for (tindex = 0; tindex < nbTrees - remainder; tindex+=T) {
    score = 0;
    for (iIndex = 0; iIndex < numberOfInstances - remainder_F; iIndex+=F)
      for (j=0; j<T; j++)
        for (i=0; i<F; i++)
          score += getLeaf(trees[tindex+j], features[iIndex+i])->threshold;
    for (j=0; j<T; j++)
        for (i=0; i<remainder_F; i++)
          score += getLeaf(trees[tindex+j], features[numberOfInstances - remainder_F + i])->threshold;
    if (printScores) 
      printf("%f\n", score);
    sum += score;
  }
  
  //deal with remain trees
  score = 0;
  for (iIndex = 0; iIndex < numberOfInstances - remainder_F; iIndex+=F)
    for (j=0; j<remainder; j++)
      for (i=0; i<F; i++)
        score += getLeaf(trees[nbTrees - remainder+j], features[iIndex+i])->threshold;
  for (j=0; j<remainder; j++)
    for (i=0; i<remainder_F; i++)
        score += getLeaf(trees[nbTrees - remainder+j], features[numberOfInstances - remainder_F + i])->threshold;
  
  gettimeofday(&end, NULL);
  
  if(printScores)
    printf("%f\n", score);
  sum += score;
  

  printf("Time per instance per tree(ns): %5.2f\n",
         (((end.tv_sec * 1000000 + end.tv_usec) -
           (start.tv_sec * 1000000 + start.tv_usec))*1000/((float) numberOfInstances * nbTrees)));
  printf("Ignore this number: %f\n", sum);

  // Free used memory
  for(tindex = 0; tindex < nbTrees; tindex++) {
    destroyTree(trees[tindex]);
  }
  free(trees);
  for(i = 0; i < numberOfInstances; i++) {
    free(features[i]);
  }
  free(features);
  fclose(fp);
  return 0;
}
