#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main()
{
  ifstream fin;
  ofstream fout;
  // open files
  fin.open("test.txt");
  fout.open("test_new.txt");
  int numberOfInstances = 0;
  int numberOfFeatures = 0;
  // read the first line and write the first lin
  fin >> numberOfInstances >> numberOfFeatures;
  fout << numberOfInstances << " " << numberOfFeatures << endl;
  float *feature = new float[numberOfFeatures+1];
  char line[100000];
  char trash[10];
  // after reading the first line, the first '\n' is still there, read it out
  fin.getline(trash, 10, '\n');
  // get the 2nd line
  fin.getline(line, 100000, '\n');
  while(strlen(line)>10)
  {
    char *pch = strtok(line, " \n");
    // write 1st token
    fout << pch << " ";
    pch = strtok(NULL, " \n");
    // write 2nd token
    fout << pch;
    // clean feature[]
    for (int i=1; i<=numberOfFeatures; i++)
      feature[i] = 0;
    // deal with the following tokens
    while((pch = strtok(NULL, " \n")) != NULL)
    {
      int fid;
      float fvalue;
      sscanf(pch, "%d:%f", &fid, &fvalue);
      feature[fid] = fvalue;
    }
    for (int i=1; i<=numberOfFeatures; i++)
      fout << " " << i << ":" << feature[i];
    fout << endl;
    fin.getline(line, 100000, '\n');
  }
  delete []feature;
  fin.close();
  fout.close();
  return 0;
}
