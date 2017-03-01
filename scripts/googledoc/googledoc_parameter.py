#!/usr/bin/python
import sys
fin = open('out_parameter_sweeping_MQ', 'r')
lines = fin.readlines()
fin.close()
time = []
for i in range(14):
  t = []
  for j in range(3):
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
    if a == 14:
      b += 1
      a = 0
for i in range(14):
  for j in range(3):
    sys.stdout.write(str(time[i][j])+'\t')
  sys.stdout.write('\n')

