#!/bin/bash

# MQ

for LOOP in 1 2 3
do
    if [ "$LOOP" = "1" ]; then
       LEAF=10
    fi

    if [ "$LOOP" = "2" ]; then
       LEAF=50
    fi

    if [ "$LOOP" = "3" ]; then
       LEAF=150
    fi
    
    echo "result of Leaf_num=$LEAF"
    for FF in 40 89 200 500 1000 2000 5000 11441 20000 35000 45765 60000 75000 100000 
    do
      echo FF=$FF
      sed -i "s/#define F.*/#define F $FF \/\/ d/g" src/DSD_StructPlus1.c
      make
      out/DSD_StructPlus1 -ensemble /home/xin_jin/fast_rank/tree_model/MQ2007/tree-ensemble-file_20000_$LEAF -instances /home/xin_jin/fast_rank/input/MQ2007/100k.txt -maxLeaves $LEAF
    done
done

