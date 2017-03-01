#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "StructPlus.h"
#include "ParseCommandLine.h"

/**
 * Driver that evaluates test instances using the StructPlus
 * implementation. Use the following command to run this driver:
 *
 * ./StructPlus -ensemble <ensemble-path> -instances <test-instances-path> \
 *              -maxLeaves <max-number-of-leaves> [-print]
 *
 */

#define QSetNum 4

const int candidateDocNum[QSetNum] = {15000, 8000, 1000, 10};
const int queryNum[QSetNum] = {5, 12, 19, 1000};

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

  gettimeofday(&start, NULL);

  //QSD  
  int q, k;
  q=0;
  k=queryNum[0];
  for (iIndex = 0; iIndex < numberOfInstances; iIndex+=candidateDocNum[q]){
    score = 0;
    for (tindex = 0; tindex < nbTrees; tindex++) 
        for (i=0; i<candidateDocNum[q]; i++) {
          score += getLeaf(trees[tindex], features[iIndex+i])->threshold;
          //if ((iIndex+i) % 1000 == 0)
            //printf("%d ", iIndex+i);
        }
    if (printScores) 
        printf("%f\n", score);
    sum += score;
    k--;
    if (k == 0){
      q++;
      if (q<QSetNum)
        k = queryNum[q];
      else
        break;
    }
  }
  
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
