TreeNumber=2000
Leaf=150
Outfile='result_'${TreeNumber}'_'${Leaf}'.txt'
Testfile='../data/doc_dataset.txt'
Logfile='log_'${TreeNumber}'_'${Leaf}'.txt'

#make clean; make
rm ${Outfile}
rm ${Logfile}

for Alg in 'DOT' 'SOT' #'FTFT' 'FTTF' 'TFFT' 'TFTF' 
do
    echo ${Alg} >> ${Outfile}
    ../target/${Alg}_StructPlus -ensemble ../data/MSLR-tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves $Leaf >> ${Outfile}
    date +"%T"
    echo 'done prog'
    #valgrind --tool=cachegrind  --branch-sim=yes --log-file=${Logfile}.app 
    ../target/${Alg}_StructPlus -ensemble ../data/MSLR-tree-ensemble-file_${TreeNumber}_${Leaf} -instances ${Testfile} -maxLeaves $Leaf >> ${Outfile}
    date +"%T"
    #echo 'done valgrind'
    #cat ${Logfile}.app >> ${Logfile}
done    
