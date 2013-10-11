/***

	Description		:	Test the read/write behaviour of the bigtiff lib. Need to figure out the memory
						footprint and cpu requirements


***/

#ifdef WIN32
#define BT_FILE				"E:\\data\\images\\testTiletiff.tif"
#else
#define BT_FILE				"/home/r6singh/temp/testTiletiff.tif"
#endif

#define BT_NUM_OF_LEVELS	8
#define BT_TILE_W			256
#define BT_TILE_H			256
#define BT_IMG_W			256*(int)pow((float)2, (float)BT_NUM_OF_LEVELS)
#define	BT_IMG_H			256*(int)pow((float)2, (float)BT_NUM_OF_LEVELS)
#define BT_IMG_BPP			16
#define BT_IMG_SPP			1

#include <stdio.h>
#include <stdlib.h>
#include <tiffio.h>
#include <math.h>


int main (int argc, char ** argv)
{
	TIFF *image;
	char *tileBuf;
	char tileFileName[32];
	int i, j, k;
	FILE *tfptr;
	int levelImgW, levelImgH;

	// create a Tiff file
	if((image = TIFFOpen(BT_FILE, "w8")) == NULL)
	{
		printf("Could not open output for writing\n");
		exit(1);
	}

	int tileByteSize = BT_TILE_W * BT_TILE_H * BT_IMG_BPP * BT_IMG_SPP / 8;
	tileBuf = (char  *)malloc(tileByteSize);

	for(i=0 ; i< BT_NUM_OF_LEVELS; i++)
	{
		// for each level, open the image tile file and load it
		sprintf(tileFileName,"%d.raw", i+1);
		tfptr = fopen(tileFileName, "rb");
		fread((void *)tileBuf, tileByteSize, 1, tfptr);
		fclose(tfptr);

		levelImgW = BT_IMG_W / (int)pow((float)2, (float)i);
		levelImgH = BT_IMG_H / (int)pow((float)2, (float)i);

		// populate the TIFF tags
		TIFFSetField(image, TIFFTAG_IMAGEWIDTH, levelImgW);
		TIFFSetField(image, TIFFTAG_IMAGELENGTH, levelImgH);
		TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, BT_IMG_BPP);
	//	TIFFSetField(image, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, BT_IMG_SPP);

		TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		if(BT_IMG_SPP == 1)
			TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		if(BT_IMG_SPP == 3 || BT_IMG_SPP == 4)
			TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	//		TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );

//		TIFFSetField(image, TIFFTAG_XRESOLUTION, 100.0);
//		TIFFSetField(image, TIFFTAG_YRESOLUTION, 100.0);
//		TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		TIFFSetField(image, TIFFTAG_TILEWIDTH, BT_TILE_W);
		TIFFSetField(image, TIFFTAG_TILELENGTH, BT_TILE_H);

		printf("\nWriting Level %d / %d", i+1, BT_NUM_OF_LEVELS);fflush(stdout);
		for(k=0; k < levelImgH / BT_TILE_H; k++)
		{
			for(j=0; j < levelImgW / BT_TILE_W; j++)
			{
				if(TIFFWriteEncodedTile(image, k * (levelImgH / BT_TILE_H) + j, (tdata_t)tileBuf, (tsize_t)-1 ) == -1)
				{
					TIFFError("\ntileTiffTester", "Could not write tile %dx%d", j, k);
				}
			}
		}

		// dump the current directory to disk and start a new one
		TIFFWriteDirectory(image);
	}

	TIFFClose(image);
	free(tileBuf);



} // End of main
