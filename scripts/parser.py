#!/usr/bin/python

import sys
import re
import numpy as np

def parseResult(fileName):
    f = open(fileName, 'r')
    lines = f.readlines()
    time = []
    for line in lines:
        words = line.strip().split()
        if words[0] == 'Time':
            time.append(int(float(words[4]))) # time per instance in ns
    return time

def parseLog(fileName):
    f = open(fileName, 'r')
    lines = f.readlines()
    dRef = []
    d1Miss = []
    ddlMiss = []
    bRef = []
    bMiss = []
    for line in lines:
        words = line.strip().split()
        if len(words) > 2 and words[1] == 'D' and words[2] == 'refs:':
            dRef.append(int(words[3].replace(',','')))
        if len(words) > 2 and words[1] == 'D1' and words[2] == 'misses:':
            d1Miss.append(int(words[3].replace(',','')))
        if len(words) > 2 and words[1] == 'LLd' and words[2] == 'misses:':
            ddlMiss.append(int(words[3].replace(',','')))
        if len(words) > 2 and words[1] == 'Branches:':
            bRef.append(int(words[2].replace(',','')))
        if len(words) > 2 and words[1] == 'Mispredicts:':
            bMiss.append(int(words[2].replace(',','')))

    if len(dRef) != len(d1Miss) or len(dRef) != len(ddlMiss) or len(dRef) != len(bRef) or len(dRef) != len(bMiss):
        sys.exit(-1)
    
    d1MissRate = np.array(d1Miss, dtype=np.float)/np.array(dRef, dtype=np.float)
    ddlMissRate = np.array(ddlMiss, dtype=np.float)/np.array(d1Miss, dtype=np.float)
    ddlMissRate2 = np.array(ddlMiss, dtype=np.float)/np.array(dRef, dtype=np.float)
    bMissRate = np.array(bMiss, dtype=np.float)/np.array(bRef, dtype=np.float)
    
    return [dRef, d1Miss, ddlMiss, bRef, bMiss, d1MissRate, ddlMissRate, ddlMissRate2, bMissRate]
    
def main():
    treeNum = '100'
    node = '50'
    dir = 'perf/'+treeNum+'_'+node+'_variFT_FTFT/'
    
    #alg = ['DOT', 'SOT', 'FTFT', 'FTTF', 'TFFT', 'TFTF']
    #print alg
    
    time = parseResult(dir+'log_'+treeNum+'_'+node+'.txt')
    print time
'''
    [dRef, d1Miss, ddlMiss, bRef, bMiss, d1MissRate, ddlMissRate, ddlMissRate2, bMissRate] = parseLog(dir+'log_'+treeNum+'_'+node+'.txt')
    print dRef
    print d1Miss
    print ddlMiss
    print d1MissRate
    #print ddlMissRate
    print ddlMissRate2
    print '\n'
    print bRef
    print bMiss
    print bMissRate
'''

if __name__ == '__main__':
    main()
