/***

	Description		:	Test the read/write behaviour of the bigtiff lib. Need to figure out the memory
						footprint and cpu requirements


***/
#ifdef WIN32
	#define BT_FILE				"E:\\temp\\testBigtiff.tif"
#else
	#define BT_FILE				"/home/r6singh/temp/testBigtiff.tif"
	#define BT_FILE				"/home/r6singh/temp/weaverTest_10x10.tif.weaver/level0_temp.tif"
	#define BT_FILE				"/home/r6singh/temp/bigtiff-be/tiger-separated-strip-contig-16.tif"
#endif

#define BT_IMG_W			1000
#define BT_IMG_H			25000
#define BT_IMG_BPP			16
#define BT_IMG_SPP			1

#include <stdio.h>
#include <stdlib.h>
#include <tiffio.h>


int main (int argc, char ** argv)
{
	TIFF *image;
	short *rowBuf;
	int i;
	int imgW = 0, imgH = 0, bps = 0, spp = 0;
	int rowsPerStrip = 0;

	// create a bigtiff file
	if((image = TIFFOpen(BT_FILE, "r8")) == NULL)
	{
		printf("\nCould not open file for reading\n");
		exit(1);
	}

	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &imgW);
	TIFFGetField(image, TIFFTAG_IMAGELENGTH, &imgH);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bps);
	if(TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &spp) == 0) printf("\nCould not read TIFFTAG_SAMPLESPERPIXEL");
	if(TIFFGetField(image, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip) == 0) printf("\nCould not read TIFFTAG_ROWSPERSTRIP");

//	TIFFGetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
//	TIFFGetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
//	TIFFGetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
//	TIFFGetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	//TIFFSetDirectory(image, 0);

	printf("\nImage Size = %dx%d. Bps = %d. Spp = %d. RowsPerStrip = %d", imgW, imgH, bps, spp, rowsPerStrip);
	fflush(stdout);
	printf("\nPress Enter to continue ..");
	getchar();

	// create a pattern in buffer
	int rowBufSize = imgW * bps * spp / 8;
	if((rowBuf = (short *)malloc (rowBufSize)) == NULL)
	{
		printf("\nError allocating %d bytes for row buffer", rowBufSize);
		exit(1);
	}
	

	// read the strips from the TIFF file
	for (i = 0; i < imgH; i++)
	{
		printf("\nReading row (strip) # %d", i);fflush(stdout);
		TIFFReadEncodedStrip(image, i, (tdata_t)rowBuf, (tsize_t)-1);
	}

	
	free(rowBuf);
	// close the file
	TIFFClose(image);

	return 0;
} // End of main 

