#!/bin/bash

# Yahoo
# Set F
sed -i "s/#define F.*/#define F 1500 \/\/ d/g" src/DSD_StructPlus1.c
#sed -i "s/#define F.*/#define F 5 \/\/ d/g" src/DSD_StructPlus1.c
#sed -i "s/#define F.*/#define F 747 \/\/ d/g" src/DSD_StructPlus2.c
#sed -i "s/#define F.*/#define F 2989 \/\/ d/g" src/DSD_StructPlus3.c
#sed -i "s/#define F.*/#define F 100000 \/\/ d/g" src/DSD_StructPlus4.c

# Set T
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
    
    make
    
    echo "result of Leaf_num=$LEAF"
    for VAR in 1
    do
        echo 'DSD'$VAR
        out/DSD_StructPlus$VAR -ensemble /home/xin_jin/fast_rank/tree_model/yahoo/xin/leaf_$LEAF/tree-ensemble-file_20000_$LEAF -instances /home/xin_jin/fast_rank/input/yahoo/100k.txt -maxLeaves $LEAF
    done
    echo
done




