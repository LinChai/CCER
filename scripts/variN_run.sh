TreeNumber='8000'
Leaf='150'
Logfile='log_'${TreeNumber}'_'${Leaf}'.txt'
Dataset='yahoo'

rm ${Logfile}

for Alg in '8_VPred' #'SOT_StructPlus' 'TFTF_StructPlus' #'16_VPred' #'SOT' 'DOT'
do
    for DocNumber in '10' '100' '1k' '2k' '5k' '10k' '100k' #'10' '100' '1000' '10000' '100000'
    do
        echo ${Alg}' '${Leaf}' '${DocNumber} >> ${Logfile};
        Testfile='input/yahoo/'${DocNumber}'.txt'
        
        if [ ${DocNumber} == '10' ]; then
            F='10'
        else 
            if [ ${DocNumber} == '100' ]; then
                F='100'
            else
                F='480'
            fi
        fi

        T='1000'
        #sed -i '5 s/^.*$/#define F '${F}'/' src/StructPlus.h
        #sed -i '6 s/^.*$/#define T '${T}'/' src/StructPlus.h
        #rm out/TFTF_StructPlus
        sed -i '5 s/^.*$/#define F '${F}'/' src/Struct.h
        sed -i '6 s/^.*$/#define T '${T}'/' src/Struct.h
        rm out/8_VPred
        make
        echo ${F}' '${T} >> ${Logfile};
        
        perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-misses,LLC-loads,LLC-load-misses,branch-loads,branch-load-misses out/${Alg} -ensemble tree_model/${Dataset}/tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves ${Leaf} >> ${Logfile} 2>&1;
        
        date +"%T"
    done
done
