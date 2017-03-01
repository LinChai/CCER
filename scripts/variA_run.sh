TreeNumber=8000
Leaf=50
Testfile='test/test10000.txt'
Logfile='log_'${TreeNumber}'_'${Leaf}'.txt'
Dataset='yahoo'

rm ${Logfile}
#make clean; make

#for Alg in '4_VPred_orig_151' '8_VPred_orig_151' '16_VPred_orig_151' #'32_VPred_orig' #'DOT_StructPlus''SOT_StructPlus'
for Alg in '4_VPred_orig' '8_VPred_orig' '16_VPred_orig' '32_VPred_orig' 'DOT_StructPlus' 'SOT_StructPlus'
do
    echo ${Alg} >> ${Logfile};
    perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg} -ensemble tree_model/${Dataset}/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
    date +"%T"
done            
            #perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses,L1-dcache-loads,L1-dcache-misses,L1-dcache-stores,L1-dcache-store-misses,L1-dcache-prefetches,L1-dcache-prefetch-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetches,LLC-prefetch-misses,branch-loads,branch-load-misses  out/${Alg}_StructPlus -ensemble tree_model/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
            # perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg}_StructPlus -ensemble tree_model/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
           
