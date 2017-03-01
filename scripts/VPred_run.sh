TreeNumber=$1
Leaf=150
echo "8_VPred:" >> result_2000_150.txt;
../target/8_VPred -ensemble ../data/MSLR-tree-ensemble-file_${TreeNumber}_${Leaf} -instances ../data/doc_dataset.txt -maxLeaves $Leaf >> result_2000_150.txt;
echo "8_VPred finished";
