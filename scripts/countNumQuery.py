#!/usr/bin/python
from sets import Set

def parseFile(fileName):
    f = open(fileName, 'r')
    lines = f.readlines()
    firstLine = lines[0]
    docCount = int(firstLine.strip().split()[0])
    lines = lines[1:]   # drop first line
    queryIds = Set([])

    for line in lines:
        words = line.strip().split()
        queryIds.add(words[1].split(':')[1])

    return [docCount, len(queryIds)]

if __name__ == '__main__':
    fileNames = ['test.txt', 'MSLR_test.txt', 'MQ2007.txt']
    for file in fileNames:
        [docCount, queryCount] = parseFile(file)
        print file, docCount, queryCount, 1.0*docCount/queryCount

