TreeNumber=10000
Leaf=200
Testfile='MQ2007_10k.txt'
Logfile='log_'${TreeNumber}'_'${Leaf}'.txt'
Dataset='MQ2008'

rm ${Logfile}

for Alg in '8_VPred' #'TFTF_StructPlus' #'16_VPred' #'TFTF_StructPlus' #'16_VPred' #'TFTF' 'FTFT'
do 
    for F in '100' '1000' '10000' 
    do
        for T in '16' '160' '1600' #'16000' '32000'
        do
            #for R in '1' '2' '4' '8' '16'
            #do
<<VPRED
                #sed -i '9 s/^.*$/#define NUM_THREADS '${R}'/' src/StructPlus.h 
                sed -i '5 s/^.*$/#define F '${F}'/' src/StructPlus.h 
                sed -i '6 s/^.*$/#define T '${T}'/' src/StructPlus.h 
                rm out/TFTF_StructPlus
                #rm out/FTFT_StructPlus
                make
VPRED

                sed -i '5 s/^.*$/#define F '${F}'/' src/Struct.h 
                sed -i '6 s/^.*$/#define T '${T}'/' src/Struct.h 
                rm out/8_VPred
                make

                echo ${Alg}' '${F}' '${T} >> ${Logfile};
                perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg} -ensemble tree_model/${Dataset}/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
                date +"%T"
            #done
        done
    done
done    
            #perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses,L1-dcache-loads,L1-dcache-misses,L1-dcache-stores,L1-dcache-store-misses,L1-dcache-prefetches,L1-dcache-prefetch-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetches,LLC-prefetch-misses,branch-loads,branch-load-misses  out/${Alg}_StructPlus -ensemble tree_model/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
            # perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg}_StructPlus -ensemble tree_model/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
            
