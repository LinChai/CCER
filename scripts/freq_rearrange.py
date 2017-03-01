#!/usr/bin/python

import sys
import re
import numpy as np
from array import *

#tree_file = 'util/tree-ensemble-file_1083_50'
tree_file = 'tree_model/tree-ensemble-file_8000_50'
inst_file = 'test.txt'

def cal_feature_freq():
    f = open(inst_file, 'r')
    lines = f.readlines()
    first_line = lines[0].strip().split()
    inst_cnt = int(first_line[0])
    feature_cnt = int(first_line[1])
    #print inst_cnt, feature_cnt

    feature_freq = array('f', [0.0]*(feature_cnt+1))
    '''
    feature_freq = []
    for i in range(0, feature_cnt+1):
        feature_freq.append(0.0)
    '''
    lines = lines[1:]
    for line in lines:
        feature_freq_lst = line.strip().split()[2:]
        for item in feature_freq_lst:
            pair = item.split(':')
            if float(pair[1]) > 0:
                feature_freq[int(pair[0])] += float(pair[1])
    #print feature_freq
    f.close()

def cal_mapping(freq_lst):
    '''
    f = open('feature_freq', 'r')
    line = f.readline().strip()
    f.close()
    
    freq_lst_str = line.split(', ')
    freq_lst = []
    for item in freq_lst_str:
        freq_lst.append(float(item))
    '''
    zero_lst = []
    non_zero_lst = []

    print 'freq lst'
    #print freq_lst

    freq_lst.sort()
    freq_lst.reverse()
    print freq_lst

    sys.exit(1)

    for i in range(0, len(freq_lst)):
        if freq_lst[i] == 0:
            zero_lst.append(i)
        else:
            non_zero_lst.append(freq_lst[i])


    #zero_lst = zero_lst[1:] # get rid of 0 index
    non_zero_lst.sort()
    non_zero_lst.reverse()
    #print 'set of non-zero features:', non_zero_lst
    print 'cnt of zero features:', len(zero_lst)
    print 'set of zero features:', zero_lst
    
    mapping = []
    prev_item = 'x'

    for item in non_zero_lst:
        if item != prev_item:
            for j in range(0, len(freq_lst)):
                if item == freq_lst[j]:
                    mapping.append(j)
        prev_item = item

    for item in zero_lst:
        mapping.append(item)

    # safety check
    correct = 245350-700
    sum = 0
    for item in mapping:
        sum += item
    if sum != correct:
        print 'error:', sum
    #else:
        #print mapping
    
    reverse_mapping = [0]*700
    for i in range(0, 700):
        reverse_mapping[i] = mapping.index(i)
    print reverse_mapping

def cal_feature_freq_from_tree():
    f = open(tree_file, 'r')
    lines = f.readlines()
    f.close()
    
    cnt = [0]*700   # [0, 699]
    for line in lines:
        words = line.strip().split()
        if words[0] == 'root':
            cnt[int(words[-2])] += 1
        elif words[0] == 'node':
            cnt[int(words[-3])] += 1
    #print cnt

    '''
    for i in range(0, len(cnt)):
        print i, cnt[i], '\t',
    '''
    return cnt


def main():
    cnt = cal_feature_freq_from_tree()
    mapping = cal_mapping(cnt)

if __name__ == '__main__':
    main()
