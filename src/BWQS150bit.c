#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "StructPlus.h"
#include "ParseCommandLine.h"
#include <stdint.h>

#define D 1
#define S 100

/**
 * Driver that evaluates test instances using the StructPlus
 * implementation. Use the following command to run this driver:
 *
 * ./StructPlus -ensemble <ensemble-path> -instances <test-instances-path> \
 *              -maxLeaves <max-number-of-leaves> [-print]
 *
 */
typedef struct QSNode QSNode;
typedef uint64_t Byte;

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

// BWQS parameters, comparing QS, there is one more * which is for MetaTrees
double** leaves;
StructPlus*** leavesContent;
float **thresholds;
unsigned int **tree_ids;
Byte*** bitvectors;
unsigned int **offsets;

void read_ensemble(char* configFile);
void read_features(char* featureFile);

int numberOfMetaTree;
int mt; // current metaTree
int mtSize; // current metaTree size
int* leavesCount;
int* innerNodeCount;
QSNode** innerNode;
int b; // number of Bytes(64bit Byte) for the bitvector

Byte** v[D]; // vectors to save results for D documents

// Compute scores for instances using QS algorithm
void compute_QS()
{
  int i, j;
  for (i=0; i<D; i++){
    v[i] = (Byte**) malloc(S * sizeof(Byte *)); 
    for (j=0; j<S; j++)
      v[i][j] = (Byte *) malloc(b * sizeof(Byte));
  }

  int kk, k, p, h, begin, end;
  double score, sum;
  struct timeval tstart, tend;
  // for each instance, do QS algorithm
  int divideInstance = numberOfInstances / D;
  int remainderInstance = numberOfInstances % D;
  gettimeofday(&tstart, NULL);
  for (i=0; i<divideInstance; i++) {
    for (mt=0; mt<numberOfMetaTree; mt++) {
      // each time deal with D documents and a meta tree
      // first numberOfMetaTree-1 metaTrees contain S trees, the last one contains 1~S trees so calculate it separately
      if (mt != numberOfMetaTree - 1) 
        mtSize = S;
      else
        mtSize = nbTrees - S * (numberOfMetaTree-1);
      // init v[][][] to be 11..1
      for (k=0; k<D; k++)
        for (j=0; j<mtSize; j++)
          for (kk = 0; kk<b; kk++)
            v[k][j][kk] = 0xffffffffffffffff;
      // Step 1:
      for (j=0; j<numberOfFeatures; j++) {
        begin = offsets[mt][j];
        end = offsets[mt][j+1]; // what we need to test is [begin, end)
        if (begin == end)
          continue;
        for (k=0; k<D; k++) {
          if (features[i*D+k][j] <= thresholds[mt][begin])
            continue;
          p = begin; // pointer
          while (p<end && features[i*D+k][j] > thresholds[mt][p]) // still false node
          {
            h = tree_ids[mt][p]; // find current tree_id
            for (kk=0; kk<b; kk++)
              v[k][h][kk] &= bitvectors[mt][p][kk];
            p++;
          } // endwhile
        }
      }//end j
      // Step 2:
      Byte test;
      for (k=0; k<D; k++) {
        score = 0;
        for (h=0; h<mtSize; h++)
        {
          // for each tree, find the left most 1 of v[k][h][] and assign it to j
          j = 0;
          int y, z;
          for (kk=0; kk<b; kk++){
            for (z=0; z<64; z++) {
              test = 0x8000000000000000; // test = 1000 0000 ... 0000
              for (y=0; y<z; y++)
                test = test >> 1;
              if (v[k][h][kk] & test != 0) // found!
                break;
              j++;
            } // loop z
          } // loop kk
          int l = h * maxNumberOfLeaves + j;
          score += leaves[mt][l];
        } // end h
        sum += score;
      }
    } // end mt
  }
  
  
  // NEXT, deal with remainder instances, just repeat previous steps
  if (remainderInstance > 0){
    int r = remainderInstance;
    for (mt=0; mt<numberOfMetaTree; mt++) {
      // each time deal with D documents and a meta tree
      // first numberOfMetaTree-1 metaTrees contain S trees, the last one contains 1~S trees so calculate it separately
      if (mt != numberOfMetaTree - 1) 
        mtSize = S;
      else
        mtSize = nbTrees - S * (numberOfMetaTree-1);
      // init v[][][] to be 11..1
      for (k=0; k<r; k++)
        for (j=0; j<mtSize; j++)
          for (kk=0; kk<b; kk++)
            v[k][j][kk] = 0xffffffffffffffff;
      // Step 1:
      for (j=0; j<numberOfFeatures; j++) {
        begin = offsets[mt][j];
        end = offsets[mt][j+1]; // what we need to test is [begin, end)
        if (begin == end)
          continue;
        for (k=0; k<r; k++) {
          if (features[divideInstance*D+k][j] <= thresholds[mt][begin])
            continue;
          p = begin; // pointer
          while (p<end && features[divideInstance*D+k][j] > thresholds[mt][p]) // still false node
          {
            h = tree_ids[mt][p]; // find current tree_id
            for (kk=0; kk<b; kk++)
              v[k][h][kk] &= bitvectors[mt][p][kk];
            p++;
          } // endwhile
        }
      }//end j
      // Step 2:
      Byte test;
      for (k=0; k<r; k++) {
        score = 0;
        for (h=0; h<mtSize; h++)
        {
          // for each tree, find the left most 1 of v[k][h] and assign it to j
          j = 0;
          int y, z;
          for (kk=0; kk<b; kk++) {
            for (z=0; z<64; z++) {
              test = 0x8000000000000000; // test = 1000 0000 ... 0000
              for (y=0; y<z; y++)
                test = test >> 1;
              if (v[k][h][kk] & test != 0) // found!
                break;
              j++;
            } // loop z
          } // loop kk
          int l = h * maxNumberOfLeaves + j;
          score += leaves[mt][l];
        } // end h
        sum += score;
      }
    } // end mt
  } // endif
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
    leaves[mt][leavesCount[mt]] = tree->threshold;
    leavesContent[mt][leavesCount[mt]] = tree;
    leavesCount[mt]++;
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
      if (leavesContent[treeId/S][(treeId%S) * maxNumberOfLeaves + i] == tree)
      {
        //if found
        int divide = i/64;
        int remainder = i%64;
        bv[divide] &= ~((Byte)1 << (63-remainder));
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
    bv[i] = (Byte)0xffffffffffffffff;
  // Then set all leaves of the left child's bit to '0'
  setLeafBit(bv, tree->left, treeId);
}

void traverse_tree_for_inner(StructPlus* tree, int treeId)
{
  // Since traverse_tree_for_leaves has checked there are no nodes with 1 child, we don't need to check here again. Just categorize all nodes into two categories: leaves and inner nodes. Here, we only deal with inner nodes.
  if (tree->left == NULL && tree->right == NULL)
    return;
  // deal with an inner node
  innerNode[mt][innerNodeCount[mt]].threshold = tree->threshold;
  innerNode[mt][innerNodeCount[mt]].fid = tree->fid;
  innerNode[mt][innerNodeCount[mt]].tree_id = treeId % S;
  Byte* bv = (Byte *) malloc(b*sizeof(Byte));
  innerNode[mt][innerNodeCount[mt]].bitvector = bv;
  // calculate bitvector for the node tree using the structure of the tree
  calBitvector(bv, tree, treeId);
  innerNodeCount[mt]++;
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
    featureQSNode[i] = (QSNode **) malloc(maxNumberOfLeaves * mtSize * sizeof(QSNode *));
    featureQSNodeCount[i] = 0;
  }
  for (i=0; i<innerNodeCount[mt]; i++) {
    int currentFid = innerNode[mt][i].fid;
    featureQSNode[currentFid][featureQSNodeCount[currentFid]] = &innerNode[mt][i];
    featureQSNodeCount[currentFid]++;
  }
  // 2. For each array, sort it by thresholds.
  for (i=0; i<numberOfFeatures; i++) {
    if (featureQSNodeCount[i] > 1)
      qsort(featureQSNode[i], featureQSNodeCount[i], sizeof(QSNode *), mycmp);
  }
  // 3. Generate the related data structures: thresholds, tree_ids, bitvectors and offsets.
  thresholds[mt] = (float *) malloc(innerNodeCount[mt] * sizeof(float));
  tree_ids[mt] = (unsigned int*) malloc(innerNodeCount[mt] * sizeof(unsigned int));
  offsets[mt] = (unsigned int*) malloc((numberOfFeatures+1) * sizeof(unsigned int));
  bitvectors[mt] = (Byte **) malloc(innerNodeCount[mt] * sizeof(Byte *));
  for (i=0; i<innerNodeCount[mt]; i++)
    bitvectors[mt][i] = (Byte *) malloc(b*sizeof(Byte));
  int counter=0;
  for (i=0; i<numberOfFeatures; i++) {
    offsets[mt][i] = counter;
    for (j=0; j<featureQSNodeCount[i]; j++) {
      thresholds[mt][counter] = featureQSNode[i][j]->threshold;
      tree_ids[mt][counter] = featureQSNode[i][j]->tree_id;
      int l;
      for (l=0; l<b; l++)
        bitvectors[mt][counter][l] = featureQSNode[i][j]->bitvector[l];
      counter++;
    }
  }
  offsets[mt][numberOfFeatures] = counter; // write the last offsets

  //printf("Finish generating essential QS data structures.\n");
  // 4. free space
  free(featureQSNodeCount);
  for (i=0; i<numberOfFeatures; i++)
    free(featureQSNode[i]);
  free(featureQSNode);
  
  // print as testing
  /*printf("counter=%d\noffsets:", counter);
  for (i=0; i<numberOfFeatures; i++)
    printf("%d\t", offsets[mt][i]);
  printf("\n");
  for (i=0; i<counter; i++)
  {
    printf("i=%d", i);
    printf(" thresholds[i]=%f tree_ids[i]=%d bitvector[i]:", thresholds[mt][i], tree_ids[mt][i]);
    for (j=0; j<b; j++)
      printf("%d ", bitvectors[mt][i][j]);
    printf("\n");
  }
  printf("**************\n");
  */
}

void gen_QS()
{
  if (nbTrees % S == 0)
    numberOfMetaTree = nbTrees / S;
  else
    numberOfMetaTree = nbTrees / S + 1;
  leavesCount = (int *) malloc(numberOfMetaTree * sizeof(int));
  innerNodeCount = (int *) malloc(numberOfMetaTree * sizeof(int));
  leaves = (double **) malloc(numberOfMetaTree * sizeof(double *));
  leavesContent = (StructPlus ***) malloc(numberOfMetaTree * sizeof(StructPlus **));
  innerNode = (QSNode **) malloc(numberOfMetaTree * sizeof(QSNode *));
  
  thresholds = (float **) malloc(numberOfMetaTree * sizeof(float *));
  tree_ids = (unsigned int**) malloc(numberOfMetaTree * sizeof(unsigned int *));
  offsets = (unsigned int**) malloc(numberOfMetaTree * sizeof(unsigned int *));
  bitvectors = (Byte ***) malloc(numberOfMetaTree * sizeof(Byte **));
  // deal with one MetaTree at a time
  for (mt=0; mt<numberOfMetaTree; mt++)   
  {
    // first numberOfMetaTree-1 metaTrees contain S trees, the last contain 1~S trees so calculate it separately
    if (mt != numberOfMetaTree - 1) 
      mtSize = S;
    else
      mtSize = nbTrees - S * (numberOfMetaTree-1);

    // deal with leaves data structure
    leaves[mt] = (double *) malloc(maxNumberOfLeaves * mtSize * sizeof(double));
    leavesContent[mt] = (StructPlus **) malloc(maxNumberOfLeaves * mtSize * sizeof(StructPlus *));

    int i;
    leavesCount[mt] = 0;
    for (i=0; i<mtSize; i++){
      traverse_tree_for_leaves(trees[mt*S+i], mt*S+i);
    }
    //printf("Finish saving leaves. There are %d leaves.\n", leavesCount[mt]);

    // deal with inner node, generate QSNode data structure
    innerNodeCount[mt] = 0;
    innerNode[mt] = (QSNode *) malloc (maxNumberOfLeaves * mtSize * sizeof(QSNode));
    // calculate bitvector size needed
    if (maxNumberOfLeaves % 64 == 0)
      b = maxNumberOfLeaves / 64;
    else
      b = maxNumberOfLeaves / 64 + 1;
    for (i=0; i<mtSize; i++)
      traverse_tree_for_inner(trees[mt*S+i], mt*S+i);
    //printf("Finish saving innerNodes. There are %d innerNode.\n", innerNodeCount[mt]);
    // sort by features and generate QS data structures
    sort_and_gen();
  }
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
  int tindex, i, j;
  for(tindex = 0; tindex < nbTrees; tindex++) {
    destroyTree(trees[tindex]);
  }
  free(trees);
  for(i = 0; i < numberOfInstances; i++) {
    free(features[i]);
  }
  free(features);
  
  // free QS data structures
  for (mt = 0; mt<numberOfMetaTree; mt++){
    free(leaves[mt]); 
    free(leavesContent[mt]);
    for (i=0; i<innerNodeCount[mt]; i++)
      free(innerNode[mt][i].bitvector);
    for (i=0; i<innerNodeCount[mt]; i++)
      free(bitvectors[mt][i]);
    free(innerNode[mt]);
    free(bitvectors[mt]);
    free(thresholds[mt]); free(tree_ids[mt]); free(offsets[mt]);
  }
  
  for (i=0; i<D; i++)
  {
    for (j=0; j<S; j++)
      free(v[i][j]);
    free(v[i]);
  }
  free(leavesCount); free(leaves); free(leavesContent);
  free(innerNodeCount); free(innerNode);
  free(bitvectors); free(thresholds); free(tree_ids); free(offsets);
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


