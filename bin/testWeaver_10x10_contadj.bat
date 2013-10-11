:: create a new project/image workspace
echo "Creating new workspace ..."
weaver -o G:/data/images/weaverTest_10x10_contadj.tif -m G:/temp -n 10x10 -p pieceFile_10x10.txt -w -mr 256x256


:: Delete the temp workspace
:: weaver -o G:/data/images/weaverTest_10x10_contadj.tif -m G:/temp -d