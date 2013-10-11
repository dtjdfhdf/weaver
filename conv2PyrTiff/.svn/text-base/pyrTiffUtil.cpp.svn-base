
#include "pyrTiffUtil.h"

bool isPyrTiff(char *fileName)
{
    int i;
    int periodOff = 0;
    char extn[32] = "";
    TIFF *tiffImg;
    bool bigTiffFlag = false;
    unsigned int tiffTileW = 0, tiffTileH = 0;


    // detect the extension
    for(i = (int)(strlen(fileName) - 1); i>=0; i--)
    {
            if(fileName[i] == '.')
            {
                    periodOff = i;
                    break;
            }
    }

    strcpy(extn, fileName + periodOff + 1);
    // check extension of output file. Currently only TIFF is supported
    if(!(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 ||
            strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 ))
    return false;

    // Open the TIFF file, in 32 bit or 64-bit mode
    if((tiffImg = TIFFOpen(fileName, "r4")) == NULL){
        if((tiffImg = TIFFOpen(fileName, "r8")) == NULL) return false;
        else
            bigTiffFlag = true;
    }

    // Get number of directories in the TIFF and the tile width/height
    TIFFGetField(tiffImg, TIFFTAG_TILEWIDTH, &tiffTileW);
    TIFFGetField(tiffImg, TIFFTAG_TILELENGTH, &tiffTileH);

    // if any of the conditions for a pyramidal tiff is not satisfied, return error
    if(tiffTileW <=0 || tiffTileH <=0)
    {
        TIFFClose(tiffImg);
        return false;
    }

    TIFFClose(tiffImg);
    return true;
}


bool convToPyrTiff(char *inFileName, char *outFileName, unsigned short compression, int oImgTileW, int oImgTileH,
                   void(*progressCb)(int), char* errStr)
{
    int i, j, k, l, m, n, iImgW, iImgH;
    unsigned short iImgBps, iImgSpp;
    unsigned int iImgRowsPerStrip = 0;
    int periodOff = 0;
    char extn[32] = "";
    TIFF *tiffImg, *currLevelTiffImg, *nextLevelTiffImg;
    IplImage *ocvImg;
    bool bigTiffFlag = false;

    char dirName[512] = "./";
    char *tileRowBuf = NULL;
    char *stripBuf = NULL;
    char *tileBuf = NULL;

    int currLevel = 0;
    int pixelSize;
    int oImgNumTileCols, oImgNumTileRows;
    int tileCount, tileBufWidthStep;
    unsigned short int orientation = 0, planarConfig = 0;
    int progress = 0;

    int tileRowBufCount, tileRowBufTileOff;

    // extr1act the directory from the outFileName
    for(i=(int)(strlen(outFileName) - 1); i>=0; i--)
    {
        if(outFileName[i] == '/' || outFileName[i] == '\\')
        {
            strncpy(dirName, outFileName, i+1);
            break;
        }
    }

    // if it's a tiff file, we'll use the tiff library to open it, otherwise
    // the regular OpenCV call is good
    // detect the extension
    for(i = (int)(strlen(inFileName) - 1); i>=0; i--)
    {
            if(inFileName[i] == '.')
            {
                    periodOff = i;
                    break;
            }
    }


    //  ***************************************************************************************
    //                                      PHASE I
    //              Generate Level 0 tile tiff in the output file specified.
    //
    //  ***************************************************************************************
    progress = 10;
    if(progressCb) progressCb(progress);
    strcpy(extn, inFileName + periodOff + 1);
    // check extension of output file.
    if(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 ||
            strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 )

    {
        // Open the input TIFF image as 32 bit input. If it fails try as 64-bit (BigTiff)
        if((tiffImg = TIFFOpen(inFileName, "r4")) == NULL){
                if((tiffImg = TIFFOpen(inFileName, "r8")) == NULL)
                {
                    sprintf(errStr, "Could not open input file ( %s ) in 32-bit or 64-bit mode", inFileName);
                    return false;
                }
                else
                    bigTiffFlag = true;
        }

        // read in the TIFF file parameters
        TIFFGetField(tiffImg, TIFFTAG_IMAGEWIDTH, &iImgW);           // uint32 width;
        TIFFGetField(tiffImg, TIFFTAG_IMAGELENGTH, &iImgH);        // uint32 height;
        TIFFGetField(tiffImg, TIFFTAG_BITSPERSAMPLE, &iImgBps);
        TIFFGetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, &iImgSpp);
        TIFFGetField(tiffImg, TIFFTAG_ROWSPERSTRIP, &iImgRowsPerStrip);
        if(iImgRowsPerStrip < 1)
        {
            sprintf(errStr,"Input file ( %s ) does not store information in strips.",
                    inFileName);
            return false;
        }

        TIFFGetField(tiffImg, TIFFTAG_PLANARCONFIG, &planarConfig);
        if(planarConfig != PLANARCONFIG_CONTIG)
        {
            sprintf(errStr,"Input file ( %s ) does not store pixels contiguously. Can't handle this file.",
                    inFileName);
            return false;
        }

        if(TIFFGetField(tiffImg, TIFFTAG_ORIENTATION, &orientation) != 1)
		{
			// assign orientation default
			orientation = ORIENTATION_TOPLEFT;
		}

        pixelSize = iImgSpp * iImgBps / 8;

        oImgNumTileCols = (int)ceil((double)iImgW/(double)oImgTileW);
        oImgNumTileRows = (int)ceil((double)iImgH/(double)oImgTileH);

        // Now open the output file for writing. For the first pass this will also
        // be the Level 0 file
        currLevel = 0;
        if((currLevelTiffImg = TIFFOpen(outFileName, "w8")) == NULL)
        {
            sprintf(errStr,"Error opening output file ( %s ) for writing as 64-bit TIFF", outFileName);
            return false;
        }

        TIFFSetField(currLevelTiffImg, TIFFTAG_IMAGEWIDTH, iImgW);
        TIFFSetField(currLevelTiffImg, TIFFTAG_IMAGELENGTH, iImgH);
        TIFFSetField(currLevelTiffImg, TIFFTAG_BITSPERSAMPLE, iImgBps);
        TIFFSetField(currLevelTiffImg, TIFFTAG_SAMPLESPERPIXEL, iImgSpp);
//		TIFFSetField(nextLevelImg, TIFFTAG_ROWSPERSTRIP, 0);
        TIFFSetField(currLevelTiffImg, TIFFTAG_COMPRESSION, compression);
        if(iImgSpp == 1)
                TIFFSetField(currLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        if(iImgSpp == 3 || iImgSpp == 4)
                TIFFSetField(currLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(currLevelTiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(currLevelTiffImg, TIFFTAG_ORIENTATION, orientation );
        TIFFSetField(currLevelTiffImg, TIFFTAG_TILEWIDTH, oImgTileW);
        TIFFSetField(currLevelTiffImg, TIFFTAG_TILELENGTH, oImgTileH);

        // Allocate buffers for storing an entire row of output tiles
        tileBufWidthStep = oImgTileW * oImgNumTileCols * pixelSize;
        tileRowBuf = (char *)malloc(oImgTileH * tileBufWidthStep);
        if(tileRowBuf == NULL)
        {
            sprintf(errStr, "Error allocating %d bytes for tileRowBuf", oImgTileH * tileBufWidthStep);
            return false;
        }
        tileBuf = (char *)malloc(oImgTileW * oImgTileH * pixelSize);
        if(tileBuf == NULL)
        {
            sprintf(errStr, "Error allocating %d bytes for tileBuf", oImgTileW * oImgTileH * pixelSize);
            return false;
        }
        stripBuf = (char *)malloc(iImgRowsPerStrip * iImgW * pixelSize);
        if(stripBuf == NULL)
        {
            sprintf(errStr, "Error allocating %d bytes for stripBuf", iImgRowsPerStrip * iImgW * pixelSize);
            return false;
        }

        int readStrips = 0, readRows = 0;
        int iImgNumOfStrips = (int)(ceil)((double)iImgH / (double)iImgRowsPerStrip);


        memset((void*)tileRowBuf, 0, oImgTileH * tileBufWidthStep);
        tileRowBufCount = 0;
        tileCount = 0;
        while(readStrips < iImgNumOfStrips)
        {
            TIFFReadEncodedStrip(tiffImg, readStrips, stripBuf, (tsize_t) -1);
            ++readStrips;

            // Copy over the rows from the strips to the tileRowBuf.
            for(i = 0; i<iImgRowsPerStrip; i++)
            {
                memcpy((void *)(tileRowBuf + tileRowBufCount * tileBufWidthStep),
                       (void *)(stripBuf + i * iImgW * pixelSize),
                       iImgW * pixelSize);

                ++readRows;
                ++tileRowBufCount;

                tileRowBufCount %= oImgTileH;

                // if the tileRowBuf is full, write the output row of tiles to file
                if(tileRowBufCount == 0 || (readRows == iImgH))
                {
                    tileRowBufTileOff = 0;
                    // break the tileRowBuf into individual tiles and write to Tiff
                    for(j = 0; j < oImgNumTileCols; j++)
                    {
                        tileRowBufTileOff = j * oImgTileW * pixelSize;
                        for(k=0; k < oImgTileH; k++)
                        {
                            memcpy((void *)(tileBuf + k * oImgTileW * pixelSize),
                                   (void * )(tileRowBuf + tileRowBufTileOff + k * tileBufWidthStep),
                                   oImgTileW * pixelSize);
                        }

                        // write tile to tiff
                        TIFFWriteEncodedTile(currLevelTiffImg, (ttile_t)tileCount, (tdata_t)tileBuf, (tsize_t)-1);
                        ++tileCount;
                    }

                    // reset the tileRowBuf to all black once done
                    memset((void*)tileRowBuf, 0, oImgTileH * oImgTileW *
                            oImgNumTileCols * pixelSize);
                } // End of if(
            } // end of for (i ...

        } // end of while(readStrips < iImgNumOfStrips


        TIFFClose (tiffImg);
        TIFFWriteDirectory(currLevelTiffImg); // Close the directory for level 0
        TIFFClose (currLevelTiffImg);
        free (tileRowBuf);

        free(stripBuf);

    } // if the input file is a tiff file
    else
    {
        // Open input file with OpenCV calls
        if((ocvImg = cvLoadImage(inFileName, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
        {
                sprintf(errStr, "Error opening input file ( %s )", inFileName);
                return false;
        }

        iImgW = ocvImg->width;
        iImgH = ocvImg->height;
        iImgBps = ocvImg->depth;
        iImgSpp = ocvImg->nChannels;

        pixelSize = iImgSpp * iImgBps / 8;

        oImgNumTileCols = (int)ceil((double)iImgW/(double)oImgTileW);
        oImgNumTileRows = (int)ceil((double)iImgH/(double)oImgTileH);

        // Now open the output file for writing. For the first pass this will also
        // be the Level 0 file
        currLevel = 0;
        if((currLevelTiffImg = TIFFOpen(outFileName, "w8")) == NULL)
        {
            sprintf(errStr,"Error opening output file ( %s ) for writing as 64-bit TIFF", outFileName);
            return false;
        }

        // we will assume the most common format of pixel storage.
        planarConfig = PLANARCONFIG_CONTIG;
        orientation = ORIENTATION_TOPLEFT;

        TIFFSetField(currLevelTiffImg, TIFFTAG_IMAGEWIDTH, iImgW);
        TIFFSetField(currLevelTiffImg, TIFFTAG_IMAGELENGTH, iImgH);
        TIFFSetField(currLevelTiffImg, TIFFTAG_BITSPERSAMPLE, iImgBps);
        TIFFSetField(currLevelTiffImg, TIFFTAG_SAMPLESPERPIXEL, iImgSpp);
//		TIFFSetField(nextLevelImg, TIFFTAG_ROWSPERSTRIP, 0);
        TIFFSetField(currLevelTiffImg, TIFFTAG_COMPRESSION, compression);
        if(iImgSpp == 1)
                TIFFSetField(currLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        if(iImgSpp == 3 || iImgSpp == 4)
                TIFFSetField(currLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(currLevelTiffImg, TIFFTAG_PLANARCONFIG, planarConfig);
        TIFFSetField(currLevelTiffImg, TIFFTAG_ORIENTATION,  orientation);
        TIFFSetField(currLevelTiffImg, TIFFTAG_TILEWIDTH, oImgTileW);
        TIFFSetField(currLevelTiffImg, TIFFTAG_TILELENGTH, oImgTileH);

        // allocate storage for a tile
        tileBufWidthStep = oImgTileW * oImgNumTileCols * pixelSize;
        tileRowBuf = (char *)malloc(oImgTileH * tileBufWidthStep);
        if(tileRowBuf == NULL)
        {
            sprintf(errStr, "Error allocating %d bytes for tileRowBuf", oImgTileH * tileBufWidthStep);
            return false;
        }
        tileBuf = (char *)malloc(oImgTileW * oImgTileH * pixelSize);
        if(tileBuf == NULL)
        {
            sprintf(errStr, "Error allocating %d bytes for tileBuf", oImgTileW * oImgTileH * pixelSize);
            return false;
        }


        tileRowBufCount = 0;
        tileCount = 0;
        memset((void*)tileRowBuf, 0, oImgTileH * tileBufWidthStep);
        for(i=0; i<iImgH; i++)
        {
            memcpy((void *)(tileRowBuf + tileRowBufCount * tileBufWidthStep),
                   (void *)(ocvImg->imageData + i * ocvImg->widthStep),
                   iImgW * pixelSize);

            ++tileRowBufCount;
            tileRowBufCount %= oImgTileH;

            // when the tileRowBuff is full, split into tiles
            if(tileRowBufCount == 0)
            {
                tileRowBufTileOff = 0;
                // break the tileRowBuf into individual tiles and write to Tiff
                for(j = 0; j < oImgNumTileCols; j++)
                {
                    tileRowBufTileOff = j * oImgTileW * pixelSize;
                    for(k=0; k < oImgTileH; k++)
                    {
                        memcpy((void *)(tileBuf + k * oImgTileW * pixelSize),
                               (void * )(tileRowBuf + tileRowBufTileOff + k * tileBufWidthStep),
                               oImgTileW * pixelSize);
                    }

                    // write tile to tiff
                    TIFFWriteEncodedTile(currLevelTiffImg, (ttile_t)tileCount, (tdata_t)tileBuf, (tsize_t)-1);
                    ++tileCount;
                }

                // reset the tileRowBuf to all black once done
                memset((void*)tileRowBuf, 0, oImgTileH * oImgTileW *
                        oImgNumTileCols * pixelSize);

            } // if (tileRowBufCount == 0)
        } // End of for( i ..

//        TIFFClose (tiffImg);
        TIFFWriteDirectory(currLevelTiffImg); // Close the directory for level 0
        TIFFClose (currLevelTiffImg);

        free(tileRowBuf);

    }

    // Level 0 is approximately 30% of the effort
    progress = 30;
    if(progressCb) progressCb(progress);
    //  ***************************************************************************************
    //                                      PHASE II
    //    Generate Level 1 to Level n tiff files. These are temp files and will be deleted.
    //
    //  ***************************************************************************************

    // Figure out the number of levels we'll be generating for multires
    unsigned int numTiffLevels = 1;
    unsigned int estLowestLevelDim;
    unsigned int nextLevelW, nextLevelH, nextLevelNumTileCols, nextLevelNumTileRows;
    unsigned int currLevelW, currLevelH, currLevelNumTileCols, currLevelNumTileRows;

    estLowestLevelDim = PTU_MAX(iImgW, iImgH);

    // Calculate how many levels of multi-res we need
    while(estLowestLevelDim > PTU_TIFF_LOWESTLEVEL_DIM)
    {
            estLowestLevelDim /= 2;
            ++numTiffLevels;
    }

    int perLevelEffort = (int)(30 / (numTiffLevels - 1));
    char currLevelTiffFile[512], nextLevelTiffFile[512];
    // Create OpenCV image which will be used for storing the 2x2 tile group from a level
    IplImage *currLevelTile = cvCreateImage(cvSize(oImgTileW, oImgTileH),
                                                    iImgBps, iImgSpp);
    // OpenCV image used to store the resized image
    IplImage *resizedTile = cvCreateImage(cvSize(oImgTileW / 2, oImgTileH / 2),
                                                    iImgBps, iImgSpp);

    strcpy(currLevelTiffFile, outFileName);

    for (i = 0; i<numTiffLevels; i++)
    {
        // generate name for next level tiff
        sprintf(nextLevelTiffFile,"%s.level%d", outFileName, i+1);

        // open current file for reading and next level file for writing
        if((currLevelTiffImg = TIFFOpen(currLevelTiffFile, "r8")) == NULL)
        {
            sprintf(errStr,"Error opening Level %d file ( %s ) for reading",i, currLevelTiffFile);
            return false;
        }

        if((nextLevelTiffImg = TIFFOpen(nextLevelTiffFile, "w8")) == NULL)
        {
            sprintf(errStr,"Error opening Level %d file ( %s ) for witing",i + 1, nextLevelTiffFile);
            return false;
        }

        // calculate image dimensions for current and next level
        currLevelW = iImgW / (int)pow((double)2.0, (double)(i));
        currLevelH = iImgH / (int)pow((double)2.0, (double)(i));
        nextLevelW = iImgW / (int)pow((double)2.0, (double)(i+1));
        nextLevelH = iImgH / (int)pow((double)2.0, (double)(i+1));

        currLevelNumTileCols = (unsigned)ceil((double)currLevelW/(double)oImgTileW);
        currLevelNumTileRows = (unsigned)ceil((double)currLevelH/(double)oImgTileH);
        nextLevelNumTileCols = (unsigned)ceil((double)nextLevelW/(double)oImgTileW);
        nextLevelNumTileRows = (unsigned)ceil((double)nextLevelH/(double)oImgTileH);

        // Populate Tiff tags for level level tiff file
        TIFFSetField(nextLevelTiffImg, TIFFTAG_IMAGEWIDTH, nextLevelW);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_IMAGELENGTH, nextLevelH);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_BITSPERSAMPLE, iImgBps);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_SAMPLESPERPIXEL, iImgSpp);
//		TIFFSetField(nextLevelTiffImg, TIFFTAG_ROWSPERSTRIP, 0);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_COMPRESSION, compression);
        if(iImgSpp == 1)
                TIFFSetField(nextLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        if(iImgSpp == 3 || iImgSpp == 4)
                TIFFSetField(nextLevelTiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_PLANARCONFIG, planarConfig);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_ORIENTATION, orientation );
        TIFFSetField(nextLevelTiffImg, TIFFTAG_TILEWIDTH, oImgTileW);
        TIFFSetField(nextLevelTiffImg, TIFFTAG_TILELENGTH, oImgTileH);

        // go through the tiles in this level. Group them as 2x2 and resize them into one tile
        tileCount = 0;
        for(j=0; j<currLevelNumTileRows; j+=2)
        {
            for (k=0; k<currLevelNumTileCols; k+=2)
            {
                memset((void *)tileBuf, 0, oImgTileW * oImgTileH * pixelSize);
                // loops for going through a 2x2 tile group in the current level
                for(l=0; l<2; l++)
                {
                    for(m=0; m<2; m++)
                    {
                        memset((void *)currLevelTile->imageData, 0,
                               oImgTileW * oImgTileH * pixelSize);

                        // Boundary conditions
                        if((j + l) >= currLevelNumTileRows) continue;
                        if((k + m) >= currLevelNumTileCols) continue;

                        if(TIFFReadEncodedTile(currLevelTiffImg, (j + l)*currLevelNumTileCols + (k+m),
                                           (tdata_t)currLevelTile->imageData, (tsize_t)-1 ) == -1)
                        continue;

                        // resize the tile to half it's size
                        cvResize(currLevelTile, resizedTile, CV_INTER_CUBIC);

                        // copy the resized tile to the correct quadrant in the tiffTileBuf
                        for(n=0; n < resizedTile->height; n++)
                                memcpy((void*)(tileBuf + ((l* resizedTile->height + n) * oImgTileW + m * resizedTile->width) * pixelSize),
                                        (void*)(resizedTile->imageData + n*resizedTile->widthStep),
                                        resizedTile->width * pixelSize );

                    } // for (m..
                } // for ( l..

                // dump the resized tile to the correct place in the new level
                TIFFWriteEncodedTile(nextLevelTiffImg, tileCount++,
                                     (tdata_t)tileBuf, (tsize_t)-1 );

            } // for (k ..
        } // for (j ..

        TIFFClose(currLevelTiffImg);
        TIFFClose(nextLevelTiffImg);
        // store next level as curr level for the next iteration
        strcpy(currLevelTiffFile, nextLevelTiffFile);

        // update progress
        progress += perLevelEffort;
        if(progressCb) progressCb(progress);
    }

    // cleanup
    cvReleaseImage(&currLevelTile);
    cvReleaseImage(&resizedTile);


    //  ***************************************************************************************
    //                                      PHASE III
    //              Combine Level 1 to Level n tiff files in to the output file.
    //
    //  ***************************************************************************************

    if((tiffImg = TIFFOpen(outFileName, "a8")) == NULL){
            sprintf(errStr, "Error opening %s for appending ", outFileName);
            return false;
    }

    for(i = 1; i < numTiffLevels; i++)
    {
        // generate name for next level tiff
        sprintf(currLevelTiffFile,"%s.level%d", outFileName, i);

        if((currLevelTiffImg = TIFFOpen(currLevelTiffFile, "r8")) == NULL){

            sprintf(errStr,"Error opening level file (%s) for consolidation", currLevelTiffFile);
            return false;
        }

        // populate Tiff Tags for this level based on what we get from the currLevelImg
        TIFFGetField(currLevelTiffImg, TIFFTAG_IMAGEWIDTH, &currLevelW);
        TIFFGetField(currLevelTiffImg, TIFFTAG_IMAGELENGTH, &currLevelH);
        TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, currLevelW);
        TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, currLevelH);
        TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, iImgBps);
        TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, iImgSpp);
//TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, 256);
        TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, compression);
        if(iImgSpp == 1)
                TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        if(iImgSpp == 3 || iImgSpp == 4)
                TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, planarConfig);
        TIFFSetField( tiffImg, TIFFTAG_ORIENTATION, orientation );
        TIFFSetField(tiffImg, TIFFTAG_TILEWIDTH, oImgTileW);
        TIFFSetField(tiffImg, TIFFTAG_TILELENGTH, oImgTileH);

        for( j = 0; j < TIFFNumberOfTiles(currLevelTiffImg); j++)
        {
            TIFFReadEncodedTile(currLevelTiffImg, j, (tdata_t)tileBuf, (tsize_t)-1 );
            TIFFWriteEncodedTile(tiffImg, j, (tdata_t)tileBuf, (tsize_t)-1 );
        }

        TIFFWriteDirectory(tiffImg);
        TIFFClose(currLevelTiffImg);

        // update progress
        progress += perLevelEffort;
        if(progressCb) progressCb(progress);
    } // for (i = 0; i<numTiffLevels; i++)

    TIFFClose(tiffImg);
    free(tileBuf);

    // clean up the temp files in the directory
    deleteTempPyrFiles(outFileName);

    progress = 100;
    if(progressCb) progressCb(progress);
    return true;

} // End of convToPyrTiff()



void deleteTempPyrFiles(char * fileName)
{
    char sysCmd[512];

#ifdef WIN32
    sprintf(sysCmd,"del %s.level*", fileName);
    system(sysCmd);
    Sleep(1000);

#else
    sprintf(sysCmd, "rm -f %s.level*", fileName);
    system(sysCmd);
    usleep(1000000);

#endif

    return;

} // end of deleteTempPyrFiles

