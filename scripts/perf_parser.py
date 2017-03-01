#!/usr/bin/python

import sys
import os
import re
import numpy as np
import itertools

def parseLog(fileName, treeNum):
    f = open(fileName, 'r')
    lines = f.readlines()

    time_per_inst = []
    Alg = []
    F = []
    T = []
    d1Load = []
    d1Miss = []
    ddlLoad = []
    ddlMiss = []
    bLoad = []
    bMiss = []
    time = []

    for line in lines:
        words = line.strip().split()
        if len(words) > 1 and words[1] == 'Segmentation' and words[2] == 'fault':
            time_per_inst.append(0)
        if len(words) > 1 and words[0] == 'Time':
            time_per_inst.append(float(words[4])/treeNum) # time per instance in ns/number of trees
            #time_per_inst.append(float(words[4]))
            #time_per_inst.append(float(words[4])*8/1000000)
        '''
        if len(words) == 3 and (words[0] == 'TFTF' or words[0] == 'FTFT' or words[0] == 'TFFT' or words[0] == 'FTTF' or words[0] == 'SOT' or words[0] == 'DOT'):
            Alg.append(words[0])
            F.append(words[1])
            T.append(words[2])
        '''
        if len(words) == 2 and words[1] == 'L1-dcache-loads':
            d1Load.append(int(words[0].replace(',','')))
        if len(words) == 2 and words[1] == 'L1-dcache-misses':
            d1Miss.append(int(words[0].replace(',','')))
        if len(words) == 2 and words[1] == 'LLC-loads':
            ddlLoad.append(int(words[0].replace(',','')))
        if len(words) == 2 and words[1] == 'LLC-misses':
            ddlMiss.append(int(words[0].replace(',','')))
        if len(words) == 2 and words[1] == 'branch-loads':
            bLoad.append(int(words[0].replace(',','')))
        if len(words) == 2 and words[1] == 'branch-misses':
            bMiss.append(int(words[0].replace(',','')))
        if len(words) > 1 and words[1] == 'seconds':
            time.append(int(float(words[0])))
    
    if len(d1Load) != len(d1Miss) or len(d1Load) != len(ddlMiss) or len(d1Load) != len(bLoad) or len(d1Load) != len(bMiss) or len(d1Load) != len(ddlLoad):
        print len(time_per_inst), len(d1Load), len(d1Miss), len(ddlMiss), len(bLoad), len(bMiss), len(ddlLoad)
        
        print 'size unmatch'
        sys.exit(-1)
    
    print len(time_per_inst), len(d1Load), len(d1Miss), len(ddlMiss), len(bLoad), len(bMiss), len(ddlLoad)
    
    d1MissRate = np.array(d1Miss, dtype=np.float)*100/np.array(d1Load, dtype=np.float)
    ddlMissRate = np.array(ddlMiss, dtype=np.float)*100/np.array(ddlLoad, dtype=np.float)
    ddlMissRate2 = np.array(ddlMiss, dtype=np.float)*100/np.array(d1Load, dtype=np.float)
    bMissRate = np.array(bMiss, dtype=np.float)*100/np.array(bLoad, dtype=np.float)
   
    #return [Alg, F, T, d1Load, d1Miss, ddlMiss, bLoad, bMiss, d1MissRate, ddlMissRate, ddlMissRate2, bMissRate]
    x=1
    y=1
    z=7
    return [np.array(time_per_inst, dtype=np.float).reshape(x,y,z), np.array(time, dtype=np.int).reshape(x,y,z), d1MissRate.reshape(x,y,z), \
        ddlMissRate.reshape(x,y,z), ddlMissRate2.reshape(x,y,z), bMissRate.reshape(x,y,z)]
    '''
    x=1
    y=5
    return [np.array(time_per_inst, dtype=np.int).reshape(x,y), np.array(time, dtype=np.int).reshape(x,y), d1MissRate.reshape(x,y), \
        ddlMissRate.reshape(x,y), ddlMissRate2.reshape(x,y), bMissRate.reshape(x,y)]
    '''

def print2D(chain):
    for i in range(0, len(chain[0])):
        for j in range(0, len(chain[0][0])):
            for m in chain:
                print m[i][j],
            print '\n',

def print3D(chain):       
    for i in range(0, len(chain)):
        print chain[i],
        print '\n',
    
    #print chain
def printAvg(chain):
    for i in range(0, len(chain)):
        m = chain[i]
        for j in range(0, len(m)):
            avg = sum(m[j])/len(m[j])
            print avg,
        print '\n'

def main():
    treeNum = '8000'
    node = '150'
    dir = 'perf/yahoo/'+treeNum+'_'+node+'_variN_VPred_combined/'
    scriptFile = 'variN_run.sh'
    logFile = 'log_'+treeNum+'_'+node+'.txt'
    
    print dir+'perf'
    print dir+scriptFile

    
    # make dir if needed
    os.system('mkdir -p '+dir)
    # cp script file
    os.system('cp '+scriptFile+' '+dir)
    # cp tmp file (prog output)
    os.system('cp tmp '+dir)    
    # mv log file
    os.system('mv '+logFile+' '+dir)
    

    matrix = parseLog(dir+logFile, int(treeNum))
    
    #redirect stdout to file
    sys.stdout = open(dir+'perf', 'a')

    print matrix
    
    chain = itertools.chain(*matrix)
    chain = (list(chain))   
    print2D(chain)

    #print3D(chain)
    #printAvg(chain)
    

if __name__ == '__main__':
    main()
