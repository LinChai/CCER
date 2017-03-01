#include <iostream>
#include <fstream>
#include <string>

using namespace std;

ofstream fout;

#define V 32
#define D 150
void writeFunction(int depth)
{
  string function1, function2;
  function1 = "void findLeafDepth";
  function2 = "(int* leaves, float* featureVector, int numberOfFeatures, Node* nodes) {";
  // print function head
  fout << function1 << depth << function2 << endl;
  int i;
  // print the first block section
  for(i=0; i<V; i++)
    fout << "\tleaves[" << i << "] = (*nodes).children[(*(featureVector+numberOfFeatures*" << i << "+(*nodes).fid) > (*nodes).theta)];" << endl;
  fout << endl;
  int d;
  // print the remaining depth-1 block section
  for (d=1; d<=depth-1; d++)
  {
    for (i=0; i<V; i++)
      fout << "\tleaves[" << i << "] = (*(nodes+leaves[" << i << "])).children[(*(featureVector+numberOfFeatures*" << i << "+(*(nodes+leaves[" << i << "])).fid) > (*(nodes+leaves[" << i << "])).theta)];" << endl;
    fout << endl;
  }
  fout << "}" << endl;
}
int main()
{
  fout.open("src/32_VPred_151.h");
  fout << "#define V "<< V << endl;
  fout << "typedef struct Node Node;" << endl;
  fout << "struct Node{" << endl;
  fout << "\tint fid;" << endl;
  fout << "\tfloat theta;" << endl;
  fout << "\tint children[2];" << endl;
  fout << "};" << endl;
  for (int i=1; i<=D; i++)
    writeFunction(i);
  fout.close();
  return 0;
}
