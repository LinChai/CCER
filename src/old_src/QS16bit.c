#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "StructPlus.h"
#include "ParseCommandLine.h"
#include <stdint.h>

#define F 1

/**
 * Driver that evaluates test instances using the StructPlus
 * implementation. Use the following command to run this driver:
 *
 * ./StructPlus -ensemble <ensemble-path> -instances <test-instances-path> \
 *              -maxLeaves <max-number-of-leaves> [-print]
 *
 */
typedef struct QSNode QSNode;
typedef uint16_t Byte;

struct QSNode {
  float threshold;
  int fid;
  unsigned int tree_id;
  Byte* bitvector; // how many bytes are determined by maxNumberOfLeaves
};

int nbTrees;
int maxNumberOfLeaves;
int numberOfFeatures;
int numberOfInstances;
StructPlus** trees; 
float** features;

// QS parameters
double* leaves;
StructPlus** leavesContent;
float *thresholds;
unsigned int *tree_ids;
Byte** bitvectors;
unsigned int *offsets;

void read_ensemble(char* configFile);
void read_features(char* featureFile);

int leavesCount;
int innerNodeCount;
QSNode* innerNode;
int b; // number of Bytes(16bit Byte) for the bitvector

Byte** v; // vectors to save results

// Compute scores for instances using QS algorithm
void compute_QS()
{
  int i, j;
  v = (Byte**) malloc(nbTrees * sizeof(Byte*)); 
  for (i=0; i<nbTrees; i++)
    v[i] = (Byte*) malloc(b * sizeof(Byte));

  int k, p, h, begin, end;
  double score, sum;
  struct timeval tstart, tend;
  // for each instance, do QS algorithm
  gettimeofday(&tstart, NULL);
  for (i=0; i<numberOfInstances; i++) {
    // init v to be 11..1
    for (j=0; j<nbTrees; j++)
      for (k=0; k<b; k++)
        v[j][k] = 0xffff;
    // Step 1:
     
    for (j=0; j<numberOfFeatures; j++) {
      begin = offsets[j];
      end = offsets[j+1]; // what we need to test is [begin, end)
      if (begin == end)
        continue;
      p = begin; // pointer
      
      while (p<end && features[i][j] > thresholds[p]) // still false node
      {
        h = tree_ids[p]; // find current tree_id
        for (k=0; k<b; k++) // AND bitvector
          v[h][k] &= bitvectors[p][k];
        p++;
      } // endwhile
        
    }
    // Step 2:
    
    score = 0;
    Byte test;
    for (h=0; h<nbTrees; h++)
    {
      // for each tree, find the left most 1 of v[h] and assign it to j
      j = 0;
      for (k=0; k<b; k++) {
        int y, z;
        for (z=0; z<16; z++) {
          test = 0x8000; // test = 1000 0000 ... 0000
          for (y=0; y<z; y++)
            test = test >> 1;
          if (v[h][k] & test != 0) // found!
          {
            k=b;
            break;
          }
          j++;
        } // loop z
      } // loop k
      int l = h * maxNumberOfLeaves + j;
      score += leaves[l];
    }
    sum += score;
  }
  gettimeofday(&tend, NULL);
  printf("Time per instance per tree(ns): %5.2f\n", 
      (((tend.tv_sec * 1000000 + tend.tv_usec) - 
        (tstart.tv_sec * 1000000 + tstart.tv_usec))*1000/((float) numberOfInstances * nbTrees)));
  printf("Ignore this number: %lf\n", sum);
}


void traverse_tree_for_leaves(StructPlus* tree, int treeId)
{
  if (tree->left == NULL && tree->right == NULL) // leaf node 
  {
    leaves[leavesCount] = tree->threshold;
    leavesContent[leavesCount] = tree;
    leavesCount++;
  }
  else if (tree->left == NULL && tree->right != NULL) // only has right child
  {
    printf("Error in traverse_tree_for_leaves! There is a node in Tree %d only has right child", treeId);
    exit(1);
  }
  else if (tree->left != NULL && tree->right == NULL) // only has left child
  {
    printf("Error in traverse_tree_for_leaves! There is a node in Tree %d only has left child", treeId);
    exit(1);
  }
  else // node has both left and right child
  {
    traverse_tree_for_leaves(tree->left, treeId);
    traverse_tree_for_leaves(tree->right, treeId);
  }
}

// set the node's all leaves' bit to 0
void setLeafBit(Byte* bv, StructPlus* tree, int treeId)
{
  int i;
  // if the node is a leaf, check leavesContent to find its position
  if (tree->left == NULL && tree->right == NULL)
  {
    for (i=0; i<maxNumberOfLeaves; i++)
      // check all the maxNumberOfLeaves leaves of the tree to find which the leaf id
      if (leavesContent[treeId * maxNumberOfLeaves + i] == tree)
      {
        //if found
        int divide = i/16;
        int remainder = i%16;
        bv[divide] &= ~((Byte)1 << (15-remainder));
        break;
      }
  }
  else
  {
    setLeafBit(bv, tree->left, treeId);
    setLeafBit(bv, tree->right, treeId);
  }
}

void calBitvector(Byte* bv, StructPlus* tree, int treeId)
{
  // Initialize the bitvector of this node to all '1'
  int i;
  for (i=0; i<b; i++)
    bv[i] = (Byte)0xffff;
  // Then set all leaves of the left child's bit to '0'
  setLeafBit(bv, tree->left, treeId);
}

void traverse_tree_for_inner(StructPlus* tree, int treeId)
{
  // Since traverse_tree_for_leaves has checked there are no nodes with 1 child, we don't need to check here again. Just categorize all nodes into two categories: leaves and inner nodes. Here, we only deal with inner nodes.
  if (tree->left == NULL && tree->right == NULL)
    return;
  // deal with an inner node
  innerNode[innerNodeCount].threshold = tree->threshold;
  innerNode[innerNodeCount].fid = tree->fid;
  innerNode[innerNodeCount].tree_id = treeId;
  Byte* bv = (Byte *) malloc(b*sizeof(Byte));
  innerNode[innerNodeCount].bitvector = bv;
  // calculate bitvector for the node tree using the structure of the tree
  calBitvector(bv, tree, treeId);
  innerNodeCount++;
  // recursion for left and right
  traverse_tree_for_inner(tree->left, treeId);
  traverse_tree_for_inner(tree->right, treeId);
}

// compare function for qsort
// sort according to threshold from small to large
int mycmp(const void *a, const void *b)
{
  if ((*(QSNode **)a)->threshold <= (*(QSNode **)b)->threshold) 
    return -1;
  else 
    return 1;
}


// sort each feature array by its elements' thresholds and then generate required data structures
void sort_and_gen()
{
  // 1. Use an array to save QSNodes of each feature.
  QSNode ***featureQSNode = (QSNode ***) malloc(numberOfFeatures * sizeof(QSNode **));
  int i, j;
  int *featureQSNodeCount = (int *) malloc(numberOfFeatures * sizeof(int));
  for (i=0; i<numberOfFeatures; i++) {
    featureQSNode[i] = (QSNode **) malloc(maxNumberOfLeaves * nbTrees * sizeof(QSNode *));
    featureQSNodeCount[i] = 0;
  }
  for (i=0; i<innerNodeCount; i++) {
    int currentFid = innerNode[i].fid;
    featureQSNode[currentFid][featureQSNodeCount[currentFid]] = &innerNode[i];
    featureQSNodeCount[currentFid]++;
  }
  // 2. For each array, sort it by thresholds.
  for (i=0; i<numberOfFeatures; i++) {
    if (featureQSNodeCount[i] > 1)
      qsort(featureQSNode[i], featureQSNodeCount[i], sizeof(QSNode *), mycmp);
  }
  // 3. Generate the related data structures: thresholds, tree_ids, bitvectors and offsets.
  thresholds = (float *) malloc(innerNodeCount * sizeof(float));
  tree_ids = (unsigned int*) malloc(innerNodeCount * sizeof(unsigned int));
  offsets = (unsigned int*) malloc((numberOfFeatures+1) * sizeof(unsigned int));
  bitvectors = (Byte **) malloc(innerNodeCount * sizeof(Byte *));
  for (i=0; i<innerNodeCount; i++)
    bitvectors[i] = (Byte *) malloc(b*sizeof(Byte));
  int counter=0;
  for (i=0; i<numberOfFeatures; i++) {
    offsets[i] = counter;
    for (j=0; j<featureQSNodeCount[i]; j++) {
      thresholds[counter] = featureQSNode[i][j]->threshold;
      tree_ids[counter] = featureQSNode[i][j]->tree_id;
      int l;
      for (l=0; l<b; l++)
        bitvectors[counter][l] = featureQSNode[i][j]->bitvector[l];
      counter++;
    }
  }
  offsets[numberOfFeatures] = counter; // write the last offsets

  printf("Finish generating essential QS data structures.\n");
  // 4. free space
  free(featureQSNodeCount);
  for (i=0; i<numberOfFeatures; i++)
    free(featureQSNode[i]);
  free(featureQSNode);
  
  // print as testing
  /*
  printf("counter=%d\noffsets:", counter);
  for (i=0; i<numberOfFeatures; i++)
    printf("%d\t", offsets[i]);
  printf("\n");
  for (i=0; i<counter; i++)
  {
    printf("i=%d", i);
    printf(" thresholds[i]=%f tree_ids[i]=%d bitvector[i]:", thresholds[i], tree_ids[i]);
    for (j=0; j<b; j++)
      printf("%d ", bitvectors[i][j]);
    printf("\n");
  }
  */
}

void gen_QS()
{
  // deal with leaves data structure
  leaves = (double *) malloc(maxNumberOfLeaves * nbTrees * sizeof(double));
  leavesContent = (StructPlus **) malloc(maxNumberOfLeaves * nbTrees * sizeof(StructPlus *));

  int i;
  leavesCount = 0;
  for (i=0; i<nbTrees; i++)
    traverse_tree_for_leaves(trees[i], i);
  printf("Finish saving leaves. There are %d leaves.\n", leavesCount);
  if (leavesCount != maxNumberOfLeaves * nbTrees)
  {
    printf ("Error in gen_QS! leavesCount != maxNumberOfLeaves * nbTrees, we cannot align leaves correctly in the leaveContent array\n");
    exit(1);
  }

  // deal with inner node, generate QSNode data structure
  innerNodeCount = 0;
  innerNode = (QSNode *) malloc (maxNumberOfLeaves * nbTrees * sizeof(QSNode));
  // calculate bitvector size needed
  if (maxNumberOfLeaves % 16 == 0)
    b = maxNumberOfLeaves / 16;
  else
    b = maxNumberOfLeaves / 16 + 1;
  for (i=0; i<nbTrees; i++)
    traverse_tree_for_inner(trees[i], i);
  printf("Finish saving innerNodes. There are %d innerNode.\n", innerNodeCount);
  // sort by features and generate QS data structures
  sort_and_gen();
}

int main(int argc, char** args) {
  if(!isPresentCL(argc, args, (char*) "-ensemble") ||
     !isPresentCL(argc, args, (char*) "-instances") ||
     !isPresentCL(argc, args, (char*) "-maxLeaves")) {
    return -1;
  }

  char* configFile = getValueCL(argc, args, (char*) "-ensemble");
  char* featureFile = getValueCL(argc, args, (char*) "-instances");
  maxNumberOfLeaves = atoi(getValueCL(argc, args, (char*) "-maxLeaves"));
  int printScores = isPresentCL(argc, args, (char*) "-print");

  // Read ensemble
  read_ensemble(configFile);
  // Read instances (SVM Light format)
  read_features(featureFile);
  // Generate essential QS data structures
  gen_QS();
  // Compute scores for instances using QS algorithm
  compute_QS();
  
  // Free used memory
  int tindex, i;
  for(tindex = 0; tindex < nbTrees; tindex++) {
    destroyTree(trees[tindex]);
  }
  free(trees);
  for(i = 0; i < numberOfInstances; i++) {
    free(features[i]);
  }
  free(features);free(leaves); free(leavesContent);
  for (i=0; i<innerNodeCount; i++)
    free(innerNode[i].bitvector);
  free(innerNode);
  free(thresholds); free(tree_ids); free(offsets);
  for (i=0; i<innerNodeCount; i++)
    free(bitvectors[i]);
  free(bitvectors);
  for (i=0; i<nbTrees; i++)
    free(v[i]);
  free(v);
  return 0;
}

// Read ensemble
void read_ensemble(char* configFile)
{
  FILE *fp = fopen(configFile, "r");
  fscanf(fp, "%d", &nbTrees);

  // Array of pointers to tree roots, one per tree in the ensemble
  trees = (StructPlus**) malloc(nbTrees * sizeof(StructPlus*));
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
}

// Read instances (SVM Light format)
void read_features(char* featureFile)
{
  FILE* fp = fopen(featureFile, "r");
  fscanf(fp, "%d %d", &numberOfInstances, &numberOfFeatures);
  int divisibleNumberOfInstances = numberOfInstances;
  while(divisibleNumberOfInstances % F !=0)
    divisibleNumberOfInstances++;
  features = (float**) malloc(divisibleNumberOfInstances * sizeof(float*));
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
  fclose(fp);
}


