#!/usr/bin/python
import sys
fin = open('out_perf_MQ', 'r')
lines = fin.readlines()
fin.close()

# l1 and l3 are used to save the results of L1 and L3 cache miss ratio
c = 14 # number of iteration
l3 = []
l1 = []
for i in range(c):
  t1 = []
  t3 = []
  for j in range(3):
    t1.append('')
    t3.append('')
  l1.append(t1)
  l3.append(t3)

a=0
b=0
taketurn = 0
for i in range(len(lines)):
  line = lines[i].strip()
  if line.find('#') != -1:
    words = line.split(' ')
    # insert L3
    if taketurn == 0: # should be l3
      taketurn = 1
      l3[a][b] = words[-5]
    else: # should be l1
      taketurn = 0
      l1[a][b] = words[-5]
      a += 1
      if (a == c):
        b += 1
        a = 0

sys.stdout.write("L1 cache miss:\n")
for i in range(c):
  for j in range(3):
    sys.stdout.write(l1[i][j]+'\t')
  sys.stdout.write('\n')
sys.stdout.write("L3 cache miss:\n")
for i in range(c):
  for j in range(3):
    sys.stdout.write(l3[i][j]+'\t')
  sys.stdout.write('\n')
