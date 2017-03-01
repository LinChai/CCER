TreeNumber=8000
Leaf=150
Testfile='input/yahoo/2k.txt'
Logfile='log_'${TreeNumber}'_'${Leaf}'.txt'
Dataset='yahoo'

#make clean; make
rm ${Logfile}

for Alg in '8_VPred' #'SOT_StructPlus' #'TFTF_StructPlus' 'DOT_StructPlus' 
do
    for TreeNumber in '100' '500' '1000' '2000' '4000' '8000' '16000' '32000'
    do
        date +"%T"
        echo ${Alg}' '${TreeNumber} >> ${Logfile};
        
        #T=${TreeNumber}
        #F='200'
        #sed -i '5 s/^.*$/#define F '${F}'/' src/StructPlus.h
        #sed -i '6 s/^.*$/#define T '${T}'/' src/StructPlus.h
        #rm out/TFTF_StructPlus; make
        #echo ${F}' '${T} >> ${Logfile};

        perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg} -ensemble tree_model/${Dataset}/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
    done
done
