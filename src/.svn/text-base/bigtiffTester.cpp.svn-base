/***

	Description		:	Test the read/write behaviour of the bigtiff lib. Need to figure out the memory
						footprint and cpu requirements


***/
#ifdef WIN32
	#define BT_FILE				"E:\\temp\\testBigtiff.tif"
#else
	#define BT_FILE				"/home/r6singh/temp/testBigtiff.tif"
#endif

#define BT_IMG_W			100000
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

	// create a bigtiff file
	if((image = TIFFOpen(BT_FILE, "w8")) == NULL)
	{
		printf("Could not open output for writing\n");
		exit(1);
	}

	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, BT_IMG_W);
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, BT_IMG_H);
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, BT_IMG_BPP);
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, BT_IMG_SPP);
	TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, 1);

	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
//	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);


	// create a pattern in buffer
	int rowBufSize = BT_IMG_W * BT_IMG_BPP * BT_IMG_SPP / 8;
	rowBuf = (short *)malloc (rowBufSize);

	for (i = 0; i < BT_IMG_W; i ++)
	{
		rowBuf[i] = (i % 512) * 65535 / rowBufSize;
	}

	// write the strips to the TIFF file
	for (i = 0; i < BT_IMG_H; i++)
	{
		printf("\nWriting row (strip) # %d", i);
		TIFFWriteEncodedStrip(image, i, rowBuf, rowBufSize);
	}

	
	free(rowBuf);
	// close the file
	TIFFClose(image);

	return 0;
} // End of main 
