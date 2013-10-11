:: create a new project/image workspace
echo "Creating new workspace ..."
weaver -o E:/temp/weaverTest.tif -m E:/Temp -n 3x3

:: Add tiles to the project from left to right and top to bottom
echo "Adding tiles ..."
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.000.tif -l 0x0
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.001.tif -l 1x0
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.002.tif -l 2x0

weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.003.tif -l 0x1
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.004.tif -l 1x1
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.005.tif -l 2x1

weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.006.tif -l 0x2
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.007.tif -l 1x2
weaver -o E:/temp/weaverTest.tif -m E:/Temp -a 125 -t e:/data/images/100x100_3x3_small/100x100_3x3.008.tif -l 2x2

:: write the final image output from all computed tiles
echo "Writing final output image ..."
weaver -o E:/temp/weaverTest.tif -mr 256x256 -m E:/Temp -w

:: Delete the temp workspace
:: weaver -o E:/temp/weaverTest.tif -m E:/Temp -d