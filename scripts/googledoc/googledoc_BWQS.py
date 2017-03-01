#!/usr/bin/python
import sys
fin = open('out_BWQS_MS_64_run2', 'r')
lines = fin.readlines()
fin.close()
time = []
for i in range(5):
  t = []
  for j in range(8):
    t.append(0.0)
  time.append(t)
a = 0
b = 0
for i in range(len(lines)):
  line = lines[i].strip()
  if line[0] == 'T':
    words = line.split(' ')
    time[a][b] = float(words[-1])
    a += 1
    if a == 5:
      b += 1
      a = 0
for i in range(5):
  for j in range(8):
    sys.stdout.write(str(time[i][j])+'\t')
  sys.stdout.write('\n')

