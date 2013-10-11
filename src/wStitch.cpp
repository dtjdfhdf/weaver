/************************************************************************************

	Author		:		Raj Singh (raj@ncmir.ucsd.edu)
	Status		:		Aplha

	Description	:		Class for OpenCV based code to cross-correlate and blend a pair
						of images.

************************************************************************************/

#include "wStitch.h"
// Manual alignment tool
#include <QtGui/QApplication>
#include "weavermanualalign.h"

//QApplication *qtApp;
WeaverManualAlign *manAlignTool;

wStitch::wStitch()
{
        img1= img2 = grayImg1 = grayImg2 = 0;

        // set up the manual alignment tool
        manAlignTool = new WeaverManualAlign(0);

	// create a window
//	cvNamedWindow("Weaver", CV_WINDOW_AUTOSIZE);
}

wStitch::~wStitch()
{
	// cleanup
/*	if (img1)
		cvReleaseImage(&img1);
	if (img2)
		cvReleaseImage(&img2);
	if (grayImg1 != img1 && grayImg1)
		cvReleaseImage(&grayImg1);
	if (grayImg2 != img2 && grayImg2)
		cvReleaseImage(&grayImg2);
*/
}

bool wStitch::stitch(char *fileName1, char *fileName2,
					char *tempFileName1, char *tempFileName2,
					wStitchLayout layout, 
					int &matchX, int &matchY,
                                        int &imgW, int &imgH, char *tempDir,
                                        bool continueDespiteFailure,
                                        bool firstTileOfRow)
{
	
	IplImage *tempGrayImg1, *tempGrayImg2;
	double conversionScale = 1.0f; 
	char blendedFileName1[512], blendedFileName2[512];

	// Open the input images
	if((img1 = loadImage(fileName1)) == NULL)
	{
		std::cout << "\nwStitch::blend(): Error opening : " << fileName1 << std::endl;
		return false;
	}

	if((img2 = loadImage(fileName2)) == NULL)
	{
		std::cout << "\nwStitch::blend(): Error opening : " << fileName2 << std::endl;
		cvReleaseImage(&img1);
		return false;
	}

	imgW = img2->width;
	imgH = img2->height;

	// make sure both images have the same number of channels. Algorithm will not work if thats not the case
	if(img1->nChannels != img2->nChannels)
	{
		std::cout << "\nwStitch::blend(): Input files have different number of color channels." << std::endl;
		
		cvReleaseImage(&img1);
		cvReleaseImage(&img2);
		return false;
	}

	// If the images are not already grayscale, convert them to 32-bit float grayscale (needed by template matching)
	if(img1->depth == IPL_DEPTH_8U || img1->depth == IPL_DEPTH_8S) conversionScale = 1.0 / 255.0;
	if(img1->depth == IPL_DEPTH_16U || img1->depth == IPL_DEPTH_16S) conversionScale = 1.0 / 65535.0;

	if(img1->nChannels > 1)
	{
		tempGrayImg1 = cvCreateImage(cvSize(img1->width, img1->height), img1->depth,1);
		cvCvtColor(img1,tempGrayImg1,CV_BGR2GRAY);
		grayImg1 = cvCreateImage(cvSize(img1->width, img1->height), IPL_DEPTH_32F,1);
		cvConvertScale(tempGrayImg1, grayImg1, conversionScale, 0);
		cvReleaseImage(&tempGrayImg1);

		tempGrayImg2 = cvCreateImage(cvSize(img2->width, img2->height), img2->depth,1);
		cvCvtColor(img2,tempGrayImg2,CV_BGR2GRAY);
		grayImg2 = cvCreateImage(cvSize(img2->width, img2->height), IPL_DEPTH_32F,1);
		cvConvertScale(tempGrayImg2, grayImg2, conversionScale, 0);
		cvReleaseImage(&tempGrayImg2);

	}
	else
	{
		if(img1->depth != IPL_DEPTH_32F)
		{
			grayImg1 = cvCreateImage(cvSize(img1->width, img1->height), IPL_DEPTH_32F,1);
			cvConvertScale(img1, grayImg1, conversionScale, 0);

			grayImg2 = cvCreateImage(cvSize(img2->width, img2->height), IPL_DEPTH_32F,1);
			cvConvertScale(img2, grayImg2, conversionScale, 0);
		}
		else
		{
			grayImg1 = img1;
			grayImg2 = img2;
		}
	}


	matchX = matchY = -1;
	// Find the overlap between the images using smart correlation
	if(!findOverlap(grayImg1, grayImg2, layout, matchX, matchY))
	{
                std::cout << "\nwStitch::blend(): Could not find an image overlap between [ " << fileName1
                            << " ] and [ " << fileName2 << " ] " << std::endl;

                if(!continueDespiteFailure)
                {
                    cvReleaseImage(&img1);
                    cvReleaseImage(&img2);
                    return false;
                }

                // pop up the manual alignment tool to let user do the alignment. But we do only
                // in the case of horizontal layout tiles. For vertically aligned tiles if no solid
                // alignment is found we ignore it and just do the left tile blend.
                switch(layout)
                {
                case _WSM_HORIZONTAL:// need manual alignment
                    matchX = matchY = 0;
                    manAlignTool->show(fileName1, fileName2,
                                      layout, &matchX, &matchY);
                    while(manAlignTool->isVisible()) QApplication::processEvents();;
                    // When the user quits the tool, matchX and matchY should have the correct coords

                    break;
                case _WSM_VERTICAL: // run with data from the left tile alignment. only need manual alignment
                                    // for first tile of a row

                    if(firstTileOfRow)
                    {
                        matchX = matchY = 0;
                        manAlignTool->show(fileName1, fileName2,
                                          layout, &matchX, &matchY);
                        while(manAlignTool->isVisible()) QApplication::processEvents();;

                    }
                    else
                        return true; // return true because returning a false will stop the program
                                    // but its not really a success as we are not going ahead with the
                                    // blending

                    break;
                } // end of switch
        } // if (!findOverlap()

	// Once we have the overlap, check to see if temp versions of the images exist. If they
	// do, that's what we will use for final blending.
	if(strcmp(tempFileName1, "") != 0)
	{
		if((void *)grayImg1 == (void *)img1) grayImg1 = NULL;
		cvReleaseImage(&img1);

		if((img1 = loadImage(tempFileName1)) == NULL)
		{
			std::cout << "\nwStitch::blend(): Error opening : " << tempFileName1 << std::endl;
			return false;
		}
	}

	if(strcmp(tempFileName2, "") != 0)
	{
		if((void *)grayImg2 == (void *)img2) grayImg2 = NULL;
		cvReleaseImage(&img2);

		if((img2 = loadImage(tempFileName2)) == NULL)
		{
			std::cout << "\nwStitch::blend(): Error opening : " << tempFileName2 << std::endl;
			return false;
		}
	}

        // A special condition check. It might happen that during manual alignment the user decides there
        // is either a thin overlap or a gap. In this case blending cannot occur.
        if(!(matchX >= (img1->width - 10) || matchY >= (img1->height - 10)))
        {
            // Do a blend of the two images using smart watershed blending. When the function
            // returns the two images have the same blended information placed in the correct
            // portions of the images.
            if(!watershedBlend(img1, img2, layout, matchX, matchY, false))
            {
                    std::cout << "\nwStitch::blend(): Could not blend the image using watershed." << std::endl;

                    cvReleaseImage(&img1);
                    cvReleaseImage(&img2);
                    return false;
            }
        }

	// write images out to temp directory as .tif if one was specified. 
	// Otherwise overwrite the original images
	if(tempDir)
	{
		// get the path for the .tif output file 
		convertToTempTifFilePath(fileName1, tempDir, blendedFileName1);
		convertToTempTifFilePath(fileName2, tempDir, blendedFileName2);
	}
	else
	{
		strcpy(blendedFileName1, fileName1);
		strcpy(blendedFileName2, fileName2);
	}
	
	// write files back to disk. Again openCV save functions deal with 8 bit depth only
	if(!saveImage(img1, blendedFileName1))
	{
		std::cout << "\nwStitch::blend(): Could not write temp file : " << blendedFileName1
			<< std::endl;

		cvReleaseImage(&img1);
		cvReleaseImage(&img2);
		return false;
	}

	if(!saveImage(img2, blendedFileName2))
	{
		std::cout << "\nwStitch::blend(): Could not write temp file : " << blendedFileName2
			<< std::endl;

		cvReleaseImage(&img1);
		cvReleaseImage(&img2);
		return false;
	}
	
	// cleanup
	if(grayImg1 && (void *)grayImg1 != (void *)img1)
		cvReleaseImage(&grayImg1);
	if(grayImg2 && (void *)grayImg2 != (void *)img2)
		cvReleaseImage(&grayImg2);

	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	return true;
}


IplImage* wStitch::loadImage(char *fileName)
{
	int i, periodOff = 0, strip;
	unsigned int row;
	char extn[32] = "";
	TIFF *tiffImg;
	IplImage *ocvImg;
	bool bigTiffFlag = false;
	unsigned int w, h, bps=0, spp=0;

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

	// If the passed image was a TIFF, use bigtiff library to open it. The OpenCV
	// built in tiff reader is broken
	if(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 || 
		strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 )
	{
		// Open the TIFF image as 32 bit input. If it fails try as 64-bit (BigTiff)
		if((tiffImg = TIFFOpen(fileName, "r4")) == NULL){		
			if((tiffImg = TIFFOpen(fileName, "r8")) == NULL)
			{
				printf("\nwStitch::loadImage(): Error opening : %s", fileName);
				perror("\nwStitch::loadImage()");
				fflush(stdout);
				return NULL;
			}
			else
				bigTiffFlag = true;
		}

		// read in the TIFF file parameters
		TIFFGetField(tiffImg, TIFFTAG_IMAGEWIDTH, &w);           // uint32 width;
		TIFFGetField(tiffImg, TIFFTAG_IMAGELENGTH, &h);        // uint32 height;
		TIFFGetField(tiffImg, TIFFTAG_BITSPERSAMPLE, &bps);
		TIFFGetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, &spp);

		switch(spp)
		{
		case 1:
			switch(bps)
			{
			case 8: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1); break;
			case 16: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_16U,1); break;
			default: std::cout << "\nwStitch::loadImage(): Unsupported bits / pixel = " << bps << std::endl;
					return NULL;
			}
			break;
		case 3:
			switch(bps)
			{
			case 8: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3); break;
			case 16: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_16U,3); break;
			default: std::cout << "\nwStitch::loadImage(): Unsupported bits / pixel = " << bps << std::endl;
					return NULL;
			}
			break;
		case 4:
			switch(bps)
			{
			case 8: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,4); break;
			case 16: ocvImg = cvCreateImage(cvSize(w,h),IPL_DEPTH_16U,4); break;
			default: std::cout << "\nwStitch::loadImage(): Unsupported bits / pixel = " << bps << std::endl;
					return NULL;
			}
			break;
		default:
			std::cout << "\nwStitch::loadImage(): Unsupported samples / pixel = " << spp << std::endl;
			return NULL;
			break;
		}

		// create an OpenCV image structure here and copy over all the tiff pixels.
		unsigned char* ocvPixData = (unsigned char *)ocvImg->imageData;
		unsigned char* tiffPixData = (unsigned char *)malloc(w * h * bps * spp / 8);

		if(tiffPixData == NULL)
		{
			std::cout << "\nwStitch::loadImage(): Error allocating memory for Tiff temp buffer" << std::endl;
			cvReleaseImage(&ocvImg);
			return NULL;
		}
		
		// copy data using a strip interface from libtiff first into a temp buffer. Then align it
		// properly for openCV image structure.
		for(strip = 0; strip < (int)TIFFNumberOfStrips(tiffImg); strip++)
			TIFFReadEncodedStrip(tiffImg, strip, (void *)(tiffPixData + strip * TIFFStripSize(tiffImg)), (tsize_t) -1);
		
		// Now copy the Tiff pixel to the openCV image with proper alignment
		for(row = 0; row < h; row++)
		{
			memcpy((void *)(ocvPixData + row * ocvImg->widthStep), 
				(void *)(tiffPixData + row * ocvImg->width * bps * spp / 8),
				ocvImg->width * bps * spp / 8);
		}

		free(tiffPixData);
		TIFFClose(tiffImg);
		return ocvImg;
	}
	else
	{
		// If its not TIFF, use OpenCV to open the images.
		if((ocvImg = cvLoadImage(fileName, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
		{
			std::cout << "\nwStitch::loadImage(): Error opening : " << fileName << std::endl;
			return NULL;
		}
		else
			return ocvImg;
	}
}


bool wStitch::saveImage(IplImage* img, char* fileName)
{
	int i, periodOff = 0;
	char extn[32] = "";
	TIFF *tiffImg;
	bool bigTiffFlag = false;

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

	long long int estFileSize = img->width * img->height * img->depth * img->nChannels / 8;
	int estRowSize = img->width * img->depth * img->nChannels / 8;

	// If the passed image was a TIFF, use bigtiff library to open it. The OpenCV
	// built in tiff writer is broken for anythign but 8 bit depth
	if(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 || 
		strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 )
	{
		// Write the TIFF image as 32 bit input if the image is under 2GB
		if(estFileSize <= WS_32BIT_TIFF_FILE_LIMIT)
		{
			if((tiffImg = TIFFOpen(fileName, "w4")) == NULL){		
				printf("\nwStitch::saveImage(): Error writing %s as 32-bit TIFF.", fileName);
				perror("\nwStitch::saveImage()");
				fflush(stdout);
				return false;
			}
		}
		else
		{
			if((tiffImg = TIFFOpen(fileName, "w8")) == NULL){		
				printf("\nwStitch::saveImage(): Error writing %s as 64-bit TIFF.", fileName);
				perror("\nwStitch::saveImage()");
				fflush(stdout);
				return false;
			}

			bigTiffFlag = true;
		}

		TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, img->width);
		TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, img->height);
		TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, img->depth);
		TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, img->nChannels);
		TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		if(img->nChannels == 1)
			TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		if(img->nChannels == 3 || img->nChannels == 4)
			TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
//		TIFFSetField(tiffImg, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField( tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );

		for(i = 0; i< img->height; i++)
		{
			TIFFWriteEncodedStrip(tiffImg, i, (tdata_t)&img->imageData[img->widthStep * i], (tsize_t)estRowSize);
		}
		
		TIFFClose(tiffImg);
		return true;
	}
	else
	{
		// if its not a tiff file, use openCV's API. Only supports 3 channels, 8 bit, BGR images
		cvSaveImage(fileName, img, 0);
		return true;
	}

} // End of saveImage



bool wStitch::findOverlap( IplImage* refImg, IplImage* appendImg, wStitchLayout layout,
								int &matchX, int &matchY )
{

	IplImage *smallRefImg, *smallAppendImg;
	unsigned int smallImgH, smallImgW;
	double smallImgScale = 0.0f;
	CvPoint    minloc, maxloc;
	double    minval, maxval;
	int i, j;

	// Since we do not have any information about the amount of overlap area or shift, we will
	// do this in two passes.
	// 1st pass: Reduce the images to a small scaled down representation. Consider a ,say, 5 pixel
	//			wide edge area, run template match on the two images and guesstimate how much 
	//			overlap there is.
	// 2nd pass: once the overlap area is known, consider the original (full size) images and
	//			choose a rectangular area in the middle of the overlap area (minimal perspective
	//			distortion ) and run the template match again in an ROI to find the exact match.


	/********************     1st PASS. Find approx correlation match in scaled down images **************/

	// First step is to scale down to the image so as to not deal with the entire large image
	// Assuming a constant fixed dimension of WS_SMALL_IMG_DIM
	switch(layout)
	{
	case _WSM_HORIZONTAL:
	
		// scale down the appendImage .. Then use the scaling factor to scale down the refImg
		// One reason for this step is to convert the image depth to 32-bit float depth
		smallImgH = WS_SMALL_IMG_DIM;
		smallImgScale = (double)appendImg->height / (double)smallImgH;
		smallImgW = (unsigned int)((double)appendImg->width / smallImgScale);

		smallAppendImg = cvCreateImage(cvSize(smallImgW, smallImgH), IPL_DEPTH_32F, appendImg->nChannels);
		cvResize(appendImg, smallAppendImg, CV_INTER_LINEAR);

		smallRefImg = cvCreateImage(cvSize((unsigned)(refImg->width / smallImgScale), 
											(unsigned)(refImg->height / smallImgScale)),
									IPL_DEPTH_32F, refImg->nChannels);
		cvResize(refImg, smallRefImg, CV_INTER_LINEAR);
	
		break;

	case _WSM_VERTICAL:
		// scale down the appendImage .. Then use the scaling factor to scale down the refImg
		
		smallImgW = WS_SMALL_IMG_DIM;
		smallImgScale = (double)appendImg->width / (double)smallImgW;
		smallImgH = (unsigned int)((double)appendImg->height / smallImgScale);

		smallAppendImg = cvCreateImage(cvSize(smallImgW, smallImgH), IPL_DEPTH_32F, appendImg->nChannels);
		cvResize(appendImg, smallAppendImg, CV_INTER_LINEAR);

		smallRefImg = cvCreateImage(cvSize((unsigned)(refImg->width / smallImgScale), 
											(unsigned)(refImg->height / smallImgScale)),
									IPL_DEPTH_32F, refImg->nChannels);
		cvResize(refImg, smallRefImg, CV_INTER_LINEAR);
	
		break;
	} // end of switch(layout)

	IplImage *resultImg;
	CvRect appendImgRoi, refImgRoi, *templRoi;
	CvPoint	*templMatchingResults;
	int templXOff, templYOff;
	unsigned int templW, templH;

	// calculate how many template areas we will be using. Basically a grid of 4 x N regions.
	// Where N is a number that covers 50% of the tile area
	unsigned int templMatchingGridDim1 = WS_NUM_TEMPL_ALONG_EDGE; // There are around 4 regions along the edge
	unsigned int templMatchingGridDim2 = (unsigned)(WS_MAX_WIDTH_OVERLAP / WS_MIN_WIDTH_OVERLAP); 
														// cover a good portion of the tile along the other dimension
	unsigned int numTemplMatchingAreas = templMatchingGridDim1 * templMatchingGridDim2;

	// Assign mem 
	templRoi = (CvRect *)malloc(numTemplMatchingAreas * sizeof(CvRect));
	templMatchingResults = (CvPoint *)malloc(numTemplMatchingAreas * sizeof(CvPoint));
	
	// Now for the first pass template matching. We will make quite a few assumptions here. We will also
	// do template matching at multiple locations along the edge of the appendImg to cover all extreme cases

	// Before doing the template matching, we define 6 areas in the small version of appendImg which will be
	// used as templates
	switch(layout) {
		case _WSM_HORIZONTAL:

			
			templW = (int)(smallAppendImg->width * WS_MIN_WIDTH_OVERLAP);
                        templH = (int)floor((double)smallAppendImg->height / (double)templMatchingGridDim1);

			// Mark the template regions
			templXOff = 0;
			i = 0;
			while( i< numTemplMatchingAreas )
			{
				templYOff = 0; 
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templYOff += templH;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templYOff += templH;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templYOff += templH;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templXOff += (int)(smallAppendImg->width * WS_MIN_WIDTH_OVERLAP);
			}
			
			break;

		case _WSM_VERTICAL:

			
                        templW = (int)floor((double)smallAppendImg->width / (double)templMatchingGridDim1);
			templH = (int)(smallAppendImg->height * WS_MIN_WIDTH_OVERLAP);

			// Template regions along the top edge
			templYOff = 0;
			i = 0;

			while(i < numTemplMatchingAreas)
			{
				templXOff = 0;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templXOff += templW;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templXOff += templW;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

				templXOff += templW;
				templRoi[i++] = cvRect(templXOff, templYOff, templW, templH);

                                templYOff += (int)(smallAppendImg->height * WS_MIN_WIDTH_OVERLAP);
			}
			
			break;

	}
	
	resultImg = cvCreateImage(cvSize(smallRefImg->width - templW + 1, smallRefImg->height - templH + 1), 
						IPL_DEPTH_32F, 1);

	for(i = 0; i<numTemplMatchingAreas; i++)
	{
		cvSetImageROI(smallAppendImg, templRoi[i]);

		// match template
		cvMatchTemplate(smallRefImg, smallAppendImg, resultImg, CV_TM_CCOEFF_NORMED);

		// now go through the result to determine where the best match was found
		cvMinMaxLoc(resultImg, &minval, &maxval, &minloc, &maxloc, 0);

		// For CV_TM_SQDIFF, minloc carries the best match. THINK it carries the coords 
		// for the reference image and not coords inside the resulting image. Adjust for
		// the template offsets
		templMatchingResults[i].x = maxloc.x - templRoi[i].x;
		templMatchingResults[i].y = maxloc.y - templRoi[i].y;

		cvResetImageROI(smallAppendImg);
	}

	// Done with resultImg .. for now
	cvReleaseImage(&resultImg);

	// We will go through the results and try to find two points which are fairly close to each other
	// At that point we assume that particular template was a strong match. For a 8 pixel difference
	// the square of distance is = 64
	int bestCandidateIdx;
	int candidateScore, bestCandidateScore;

	bestCandidateIdx = -1;
	bestCandidateScore = 0;
	for(i=0; i<numTemplMatchingAreas; i++)
	{
		candidateScore = 0;
		for(j=0; j<numTemplMatchingAreas; j++)
		{
			if(i==j) continue;
                        if(sqDist(&(templMatchingResults[i]), &(templMatchingResults[j])) <= WS_TEMPL_CLUSTER_SQDIST )
			{
				++candidateScore;
			}
		}

		if(candidateScore > bestCandidateScore) 
		{
			bestCandidateScore = candidateScore;
			bestCandidateIdx = i;
		}
	}

	// should have found a clear candidate by now
	if(bestCandidateIdx < 0)
	{
		std::cout << "\nwStitch::findOverlap(): Could not find a clear winning match out of " <<
			numTemplMatchingAreas << " candidate templates used" << std::endl;
		return false;
	}

        printf("\nBest candidate reported match at (%d, %d) with score = %d out of %d candidates", templMatchingResults[bestCandidateIdx].x,
	   templMatchingResults[bestCandidateIdx].y, bestCandidateScore, numTemplMatchingAreas);

	// if the candidate score is ridiculously low, don't trust it
	if(bestCandidateScore < WS_LOW_TEMPL_MATCH_SCORE)
	{
		std::cout << "\nwStitch::findOverlap(): Out of  " << numTemplMatchingAreas << 
				" candidate templates used, the best candidates score was too low to be trusted" 
				<< std::endl;
		return false;
	}

	// Estimate the coordinates in the full image now, taking into account the fact the sliver of
	// pixels we used were from the edge of appendImg
	int estMatchX, estMatchY, estMatchW, estMatchH;

	estMatchX = (int)(templMatchingResults[bestCandidateIdx].x * smallImgScale);
	estMatchY = (int)(templMatchingResults[bestCandidateIdx].y * smallImgScale);

	if(estMatchX < 0)
		estMatchW = WS_MIN(appendImg->width - abs(estMatchX), refImg->width);
	else
		estMatchW = WS_MIN(refImg->width - estMatchX, appendImg->width);

	if(estMatchY < 0)
		estMatchH = WS_MIN(appendImg->height - abs(estMatchY), refImg->height);
	else
		estMatchH = WS_MIN(refImg->height - estMatchY, appendImg->height);

	// release mem
	free(templRoi); 
	free(templMatchingResults);

        matchX = estMatchX;
        matchY = estMatchY;

	// Done with resultImg .
	cvReleaseImage(&resultImg);

	// unset the ROI to avoid future confusion
	cvResetImageROI(appendImg);
	cvResetImageROI(refImg);

	// clean up before quitting;
	cvReleaseImage(&smallRefImg);
	cvReleaseImage(&smallAppendImg);

	return true;
}


bool wStitch::watershedBlend( IplImage* refImg, IplImage* appendImg, wStitchLayout layout,
								int X, int Y, bool alphaBlend )
{
	CvRect refImgRoi, appendImgRoi;
	int i, j, k;
	float conversionScale;

	// we will only work on the ROI and not the entire image. Sadly copies need to be
	// made since cvWatershed needs RGB images .. whaaa ? whyyy ?

	// Calculate ROIs in both images. Take care of edge cases when X or Y < 0. Also
	// take care of cases when the images are not of equal size
	switch(layout)
	{
        case _WSM_HORIZONTAL:
		if(Y >= 0)
		{
			refImgRoi = cvRect(X, Y, WS_MIN(refImg->width - X, appendImg->width), 
									WS_MIN(refImg->height - Y, appendImg->height));
			appendImgRoi = cvRect(0, 0, WS_MIN(refImg->width - X, appendImg->width), 
									WS_MIN(refImg->height - Y, appendImg->height));
		}
		else
		{
			refImgRoi = cvRect(X, 0, WS_MIN(refImg->width - X, appendImg->width), 
									WS_MIN(appendImg->height + Y, refImg->height));
			appendImgRoi = cvRect(0, abs(Y), WS_MIN(refImg->width - X, appendImg->width), 
									WS_MIN(appendImg->height + Y, refImg->height));
		}
		break;

        case _WSM_VERTICAL:
		if(X >= 0)
		{
			refImgRoi = cvRect(X, Y, WS_MIN(refImg->width - X, appendImg->width), 
									WS_MIN(refImg->height - Y, appendImg->height));
			appendImgRoi = cvRect(0, 0, WS_MIN(refImg->width - X, appendImg->width),
									WS_MIN(refImg->height - Y, appendImg->height));
		}
		else
		{
			refImgRoi = cvRect(0, Y, WS_MIN(appendImg->width + X, appendImg->width), 
									WS_MIN(refImg->height - Y, appendImg->height));
			appendImgRoi = cvRect(abs(X), 0, WS_MIN(appendImg->width + X, refImg->width), 
									WS_MIN(refImg->height - Y, appendImg->height));
		}

		break;
	}

	IplImage *refImgRoiImg, *tempRefImgRoiImg;
	IplImage *refMarkerGrid;

	// set the ROIs in the image
	cvSetImageROI(refImg, refImgRoi);
	cvSetImageROI(appendImg, appendImgRoi);

	// create images for ROIs
	tempRefImgRoiImg = cvCreateImage(cvSize(refImgRoi.width, refImgRoi.height), IPL_DEPTH_8U, refImg->nChannels);
	refImgRoiImg = cvCreateImage(cvSize(refImgRoi.width, refImgRoi.height), IPL_DEPTH_8U, 3);

	// convert images to 32-bit float grayscale (needed by template matching)
	if(refImg->depth == IPL_DEPTH_8U || refImg->depth == IPL_DEPTH_8S) conversionScale = 1.0f;
	if(img1->depth == IPL_DEPTH_16U || img1->depth == IPL_DEPTH_16S) conversionScale = 255.0f / 65535.0f;

	// First convert the scale to 8-bit
	cvConvertScale(refImg, tempRefImgRoiImg, conversionScale, 0);
	// Then convert the color (passed images are grayscale)
	cvCvtColor(tempRefImgRoiImg, refImgRoiImg, CV_GRAY2BGR);
	
	// clear the ROIs for now
	cvResetImageROI(appendImg);
	cvResetImageROI(refImg);

	// At this point we have the ROIs images as 8-bit, 3channel images as required by cvWatershed.
	// Prepare the marker grid
	refMarkerGrid = cvCreateImage( cvGetSize(refImgRoiImg), IPL_DEPTH_32S, 1 );
	cvZero( refMarkerGrid );

	// Draw the grid with circular seed points
	// The marker radius has to be kind of bigger than the microstructure. The factor is a calculated
	// guess based on test images

	int gridSizeW, gridSizeH, markerRad;
	unsigned int numSeedPointsW, numSeedPointsH;
	// choose a grid size that'll create a good seam. 
	switch(layout)
	{
	case _WSM_HORIZONTAL:
		gridSizeW = WS_MAX(refMarkerGrid->width / WS_SHORTDIM_GRID_FACTOR, 10);
		gridSizeH = WS_MAX(refMarkerGrid->height / WS_LONGDIM_GRID_FACTOR, 10);
		markerRad = WS_MAX(WS_MIN(gridSizeW, gridSizeH) / WS_SEED_SIZE_FACTOR, 1);

		break;

	case _WSM_VERTICAL:
		gridSizeW = WS_MAX(refMarkerGrid->width / WS_LONGDIM_GRID_FACTOR, 10);
		gridSizeH = WS_MAX(refMarkerGrid->height / WS_SHORTDIM_GRID_FACTOR, 10);
		markerRad = WS_MAX(WS_MIN(gridSizeW, gridSizeH) / WS_SEED_SIZE_FACTOR, 1);
		break;
	}
	numSeedPointsW = (unsigned)floor((double)refMarkerGrid->width / (double)gridSizeW); // floor because we are shifting the
	numSeedPointsH = (unsigned)floor((double)refMarkerGrid->height / (double)gridSizeH); // seed points by half a grid from the edge
				
	int seedColor = 100;
	CvPoint refImgCenter = cvPoint(-X + refImg->width/2, -Y + refImg->height / 2);
	CvPoint appendImgCenter = cvPoint(appendImg->width / 2, appendImg->height / 2);
	CvPoint seedCenter;

	int *setOfSegCloserToRefImg = (int *)malloc (sizeof(int) * (numSeedPointsW + 1)
									* (numSeedPointsH + 1));
	int setOfSegCloserToRefImgSize = 0;

	for(i = (int)(gridSizeH/2); i < refImgRoiImg->height; i += gridSizeH)
	{
		for(j = (int)(gridSizeW/2); j < refImgRoiImg->width; j += gridSizeW)
		{
			seedColor += 1;
			seedCenter = cvPoint(j, i);
			cvCircle(refMarkerGrid, seedCenter, markerRad, cvScalarAll(seedColor), -1, 8, 0);
			// Keep a track of if this seed is closer to refImg than appendImg
			if(sqDist(&seedCenter, &refImgCenter) < sqDist(&seedCenter, &appendImgCenter))
			{
				setOfSegCloserToRefImg[setOfSegCloserToRefImgSize++] = seedColor;
			}
		}
	}

	// Perform watershed on the common overlap area based on the seeds. This will sub
	// divide the image based on photometric gradients and hopefully on the boundary
	// of major features
	cvWatershed( refImgRoiImg, refMarkerGrid );

	// Now the blending can be done based on pre-decided seed points. Pixels are chosen
	// either from refImg or appendImg based on whether the seed point of the segment was
	// closer to refImg center ot appendImg center respectively. Then the pixels in the appendImg
	// that should be from refImg are marked as 0 (black). Alpha blending can be applied along the
	// painted and unpainted (black) pixel edges. 
	int pieceLen;
	int *pixPtr;
	int lastRefMarkerVal, lastMarkerVal;
	bool refImgMarkerValFound, edgeCaseFlag;
	CvRect tempRoi;

	// First eliminate the -1 and 0 values in refMarkerGrid by copying
	// a neighboring pixel. We'll go in anti-clockwise fashion starting at 
	// 6 o'clock, 4:30, 3, 1:30, 12 ...
	edgeCaseFlag = false;
	for (j = 0; j < refMarkerGrid->height; j++)
	{
		pieceLen = 0;
		lastRefMarkerVal = -100;
		lastMarkerVal = -100;
		refImgMarkerValFound = false;

		for (i = 0; i < refMarkerGrid->width; i++)
		{
			edgeCaseFlag = (j == 0 || j == (refMarkerGrid->height - 1) ||
							i == 0 || i == (refMarkerGrid->width - 1)) ? true : false;
			pixPtr = (int *)(&refMarkerGrid->imageData[j * refMarkerGrid->widthStep + i*4]);

			if(*pixPtr < 1) // Boundary pixel or just blank
			{
				if(edgeCaseFlag) // cut down on computation by flagging edge cases
				{
					// Left edge
					if( i == 0 ){
						// For top left corner choose 4:30
						if (j == 0)
							*pixPtr = *(int *)(&refMarkerGrid->imageData[(j + 1) * refMarkerGrid->widthStep + (i+1)*4]);
						else
						{
							// For Bottom Left corner choose 1:30
							if(j == (refMarkerGrid->height - 1))
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i+1)*4]);
							else
							{
								// for everything else, choose a right'ish neighbor
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j) * refMarkerGrid->widthStep + (i+1)*4]);
								if(*pixPtr < 1)
								{
									*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i+1)*4]);
									if(*pixPtr < 1)
									{
										*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i+1)*4]);
									}
								}
							}
						}
					}

					// Right edge
					if(i == (refMarkerGrid->width - 1)){
						// For top right corner choose 7:30
						if( j == 0)
							*pixPtr = *(int *)(&refMarkerGrid->imageData[(j + 1) * refMarkerGrid->widthStep + (i-1)*4]);
						else
						{
							// For Bottom right corner choose 10:30
							if(j == (refMarkerGrid->height - 1))
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i-1)*4]);
							else
							{
								// For everything else choose a left'ish neighbor
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j) * refMarkerGrid->widthStep + (i-1)*4]);
								if(*pixPtr < 1)
								{
									*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i-1)*4]);
									if(*pixPtr < 1)
									{
										*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i-1)*4]);
									}
								}
							}
						}
					}
					
					// Top edge. The corners have already been taken care of
					if(j == 0)
					{
						if(i > 0 && i < (refMarkerGrid->width - 1))
						{
							// Choose something from bottom
							*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i)*4]);
							if(*pixPtr < 1)
							{
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i-1)*4]);
								if(*pixPtr < 1)
								{
									*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i+1)*4]);
								}
							}

						}
					}

					// Bottom edge. Corners have been taken care of
					if(j == (refMarkerGrid->height - 1))
					{
						if(i > 0 && i < (refMarkerGrid->width - 1))
						{
							// Choose something from top
							*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i)*4]);
							if(*pixPtr < 1)
							{
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i-1)*4]);
								if(*pixPtr < 1)
								{
									*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i+1)*4]);
								}
							}

						}
					}
				} // if(edgeCaseFlag)
				else
				{
					// For non-edge cases, choose a pixel from a neighbor starting anti-clockwise from 6 o clock
					*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i)*4]);
					if (*pixPtr < 1){
						*pixPtr = *(int *)(&refMarkerGrid->imageData[(j+1) * refMarkerGrid->widthStep + (i+1)*4]);
						if(*pixPtr < 1){
							*pixPtr = *(int *)(&refMarkerGrid->imageData[(j) * refMarkerGrid->widthStep + (i+1)*4]);
							if(*pixPtr < 1){
								*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i+1)*4]);
								if(*pixPtr < 1){
									*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i)*4]);
									if(*pixPtr < 1){
										*pixPtr = *(int *)(&refMarkerGrid->imageData[(j-1) * refMarkerGrid->widthStep + (i-1)*4]);
										if(*pixPtr < 1){
											*pixPtr = *(int *)(&refMarkerGrid->imageData[(j) * refMarkerGrid->widthStep + (i-1)*4]);
										}
									}
								}
							}
						}
					}
				}
			} // if(*pixPtr < 1)

			// if the pixel is same as the last ref marker, just increment pieceLen
			if(*pixPtr == lastRefMarkerVal)
			{
				++pieceLen;
			}
			else
			{
				lastRefMarkerVal = -100;
				refImgMarkerValFound = false;

				if(*pixPtr != lastMarkerVal)
				{
					// is it supposed to be part of the ref marker set
					for(k=0; k< setOfSegCloserToRefImgSize; k++)
					{
						if(*pixPtr == setOfSegCloserToRefImg[k])
						{
							++pieceLen;
							refImgMarkerValFound = true;
							lastRefMarkerVal = *pixPtr;
							break;
						}
					}
				}
			}

			if(!refImgMarkerValFound || i == (refMarkerGrid->width - 1))
			{
				if(pieceLen > 0)
				{
					tempRoi = cvRect(appendImgRoi.x + i - pieceLen, appendImgRoi.y + j, pieceLen, 1);
					cvSetImageROI(appendImg, tempRoi);
					cvZero(appendImg);
					pieceLen = 0;
				}
			}

			lastMarkerVal = *pixPtr;

		} // for (i ..		
	} // for (j ..


	// clear the ROIs
	cvResetImageROI(appendImg);
	cvResetImageROI(refImg);

	// free mem
//	free(alphaBrushForX);
//	free(alphaBrushForY);

	// clean up the images too
	cvReleaseImage(&tempRefImgRoiImg);
	cvReleaseImage(&refImgRoiImg);
	cvReleaseImage(&refMarkerGrid);
	free(setOfSegCloserToRefImg);

	return true;

} // End of watershedBlend()


unsigned int wStitch::sqDist(CvPoint *a, CvPoint *b)
{
	return (a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y); 
} // End of distance()


bool wStitch::writeMontageToFile(struct wStichRemap* remapInfo, char *outFile)
{
	int minX, minY, maxX, maxY;	
	unsigned int maxTileW, maxTileH;
	unsigned int outImgW, outImgH;
	unsigned int outImgDepth = 0, outImgChannels = 0;
	unsigned int i, j, k;

	uint64 estFileSize;

	minX = minY = maxX = maxY = 0;
	maxTileW = maxTileH = 0;

	int periodOff = 0;
	char extn[32] = "";
	TIFF *tiffImg;
	IplImage *tempImg;
	bool bigTiffFlag = false;
	unsigned char * rowBuf = NULL;	// stores one row for the output image
	int rowBufSize;
	unsigned char *tileRowBuf = NULL;	// Stores one row for the tiles being read
	int tileRowBufSize;

	// detect the extension
	for(i = (int)(strlen(outFile) - 1); i>=0; i--)
	{
		if(outFile[i] == '.')
		{
			periodOff = i;
			break;
		}
	}

	strcpy(extn, outFile + periodOff + 1);
	// check extension of output file. Currently only TIFF is supported
	if(!(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 || 
		strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 ))
	{
		printf("\nwStitch::writeMontageToFile(): Current supported output format is TIFF");
		return false;
	}


	// figure out the actual dimensions of the final image
	for(j = 0; j < remapInfo->rows; j++)
	{
		for(i = 0; i < remapInfo->cols; i++)
		{
			if(remapInfo->tiles[i][j].remappedX < minX)
				minX = remapInfo->tiles[i][j].remappedX;
			if((remapInfo->tiles[i][j].remappedX + remapInfo->tiles[i][j].width) > maxX)
				maxX = remapInfo->tiles[i][j].remappedX + remapInfo->tiles[i][j].width;

			if(remapInfo->tiles[i][j].remappedY < minY)
				minY = remapInfo->tiles[i][j].remappedY;
			if((remapInfo->tiles[i][j].remappedY + remapInfo->tiles[i][j].height) > maxY)
				maxY = remapInfo->tiles[i][j].remappedY +  remapInfo->tiles[i][j].height;

			if(remapInfo->tiles[i][j].width > maxTileW)
				maxTileW = remapInfo->tiles[i][j].width;
			if(remapInfo->tiles[i][j].height > maxTileH)
				maxTileH = remapInfo->tiles[i][j].height;
		}
	}

	outImgW = maxX - minX;
	outImgH = maxY - minY;

	// load one image to read in the format
	tempImg = loadImage(remapInfo->tiles[0][0].tileFileName);
	if(!tempImg)
	{
		printf("\nwStitch::writeMontageToFile(): Cannot load image for tile (0,0) ");
		return false;
	}

	outImgDepth = tempImg->depth;
	outImgChannels = tempImg->nChannels;
	estFileSize = (uint64)outImgW * (uint64)outImgH * (uint64)outImgDepth * (uint64)outImgChannels / 8;

	if(estFileSize > WS_32BIT_TIFF_FILE_LIMIT) bigTiffFlag = true;
	else
		bigTiffFlag = false;

	// close the tempImg
	cvReleaseImage(&tempImg);

	// create the output file .. BigTiff if necessary
	if(!bigTiffFlag)
	{
		if((tiffImg = TIFFOpen(outFile, "w4")) == NULL){		
			printf("\nwStitch::writeMontageToFile(): Error writing %s as 32-bit TIFF.", outFile);
			perror("\nwStitch::writeMontageToFile()");
			fflush(stdout);
			return false;
		}
	}
	else
	{
		if((tiffImg = TIFFOpen(outFile, "w8")) == NULL){		
			printf("\nwStitch::writeMontageToFile(): Error writing %s as 64-bit TIFF.", outFile);
			perror("\nwStitch::writeMontageToFile()");
			fflush(stdout);
			return false;
		}
	}

	// populate the TIFF tags
	TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, outImgW);
	TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, outImgH);
	TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, (unsigned short)outImgDepth);
//	TIFFSetField(tiffImg, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)outImgChannels);
	TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	if(outImgChannels == 1)
		TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	if(outImgChannels == 3 || outImgChannels == 4)
		TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
//		TIFFSetField(tiffImg, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField( tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );

	// allocate memory for reading in one row of tile. Choose the tile with the max width
	tileRowBufSize = maxTileW * outImgDepth * outImgChannels / 8;
	tileRowBuf = (unsigned char *)malloc(tileRowBufSize);

	// allocate memory for storing one entire row. Hopefully this will always be ok
	rowBufSize = outImgW * outImgDepth * outImgChannels / 8;
	if((rowBuf = (unsigned char *)malloc(rowBufSize)) == NULL)
	{
		std::cout << "\nwStitch::writeMontageToFile(): Error allocating memory for one row : "
			<< outImgW * outImgDepth * outImgChannels / 8000000 << " MegaBytes "
			<< std::endl;

		TIFFClose(tiffImg);
		return false;
	}

	int normTileX, normTileY;
	int rowOff;
	unsigned int l;
	int tempPixVal;
	printf("\nMerging and writing row 0 of %d", outImgH - 1);

	for(i = 0; i < outImgH; i++)
	{
		memset((void*)rowBuf, 0, rowBufSize);
		//find contributing pixels from various images
		for (j = 0; j < remapInfo->rows; j++)
		{
			for (k = 0; k <remapInfo->cols; k++)
			{
				normTileX = remapInfo->tiles[k][j].remappedX + abs(minX);
				normTileY = remapInfo->tiles[k][j].remappedY + abs(minY);
				// Is this tile a contributing factor
				// for this row in the final image ?
				if(i >= normTileY && i < (normTileY + remapInfo->tiles[k][j].height))
				{
					memset((void *)tileRowBuf, 0, tileRowBufSize);
					// read the correct row in the right place in the rowBuf
					if(!readTifRow(remapInfo->tiles[k][j].tempTileFileName,
								i - normTileY, tileRowBuf))
					{
						std::cout << "\nwStitch::writeMontageToFile(): Error reading row num " << i - normTileY
								<< " from " << remapInfo->tiles[k][j].tempTileFileName << std::endl;
					}

					// ignore black pixels (0 value) since they represent empty space. This is done by 
					// adding the read tile row into the output row of pixels. pixels with 0 values will
					// be overwritten with 
					rowOff = normTileX;

					switch(outImgChannels)
					{
					case 1: // grayscale
						switch(outImgDepth)
						{
						case 8:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								if(tileRowBuf[l] != 0)
									rowBuf[rowOff + l] = (unsigned char)tileRowBuf[l];
							}
							break;

						case 16:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								if(((unsigned short *)tileRowBuf)[l] != 0)
									((unsigned short *)rowBuf)[rowOff + l] = ((unsigned short *)tileRowBuf)[l];
							}
							break;
						}
						break;

					case 3: // BGR
						switch(outImgDepth)
						{
						case 8:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								tempPixVal = (unsigned char)tileRowBuf[l*3] + 
											(unsigned char)tileRowBuf[l*3 + 1] + 
											(unsigned char)tileRowBuf[l*3 + 2];
								if(tempPixVal != 0)
								{
									rowBuf[(rowOff + l)*3] = (unsigned char)tileRowBuf[l*3];
									rowBuf[(rowOff + l)*3 + 1] = (unsigned char)tileRowBuf[l*3 + 1];
									rowBuf[(rowOff + l)*3 + 2] = (unsigned char)tileRowBuf[l*3 + 2];
								}
							}
							break;

						case 16:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								tempPixVal = ((unsigned short *)tileRowBuf)[l*3] + 
											((unsigned short *)tileRowBuf)[l*3 + 1] + 
											((unsigned short *)tileRowBuf)[l*3 + 2];
								if(tempPixVal != 0)
								{
									((unsigned short *)rowBuf)[(rowOff + l)*3] = ((unsigned short *)tileRowBuf)[l*3];
									((unsigned short *)rowBuf)[(rowOff + l)*3 + 1] = ((unsigned short *)tileRowBuf)[l*3 + 1];
									((unsigned short *)rowBuf)[(rowOff + l)*3 + 2] = ((unsigned short *)tileRowBuf)[l*3 + 2];
								}
							}
							break;
						}
						break;

					case 4: // BGRA
						
						switch(outImgDepth)
						{
						case 8:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								tempPixVal = (unsigned char)tileRowBuf[l*4] + 
											(unsigned char)tileRowBuf[l*4 + 1] + 
											(unsigned char)tileRowBuf[l*4 + 2] +
											(unsigned char)tileRowBuf[l*4 + 3];
								if(tempPixVal != 0)
								{
									rowBuf[(rowOff + l)*4] = (unsigned char)tileRowBuf[l*4];
									rowBuf[(rowOff + l)*4 + 1] = (unsigned char)tileRowBuf[l*4 + 1];
									rowBuf[(rowOff + l)*4 + 2] = (unsigned char)tileRowBuf[l*4 + 2];
									rowBuf[(rowOff + l)*4 + 3] = (unsigned char)tileRowBuf[l*4 + 3];
								}
							}
							break;

						case 16:
							for(l = 0; l < remapInfo->tiles[k][j].width; l++)
							{
								tempPixVal = ((unsigned short *)tileRowBuf)[l*4] + 
											((unsigned short *)tileRowBuf)[l*4 + 1] + 
											((unsigned short *)tileRowBuf)[l*4 + 2]+ 
											((unsigned short *)tileRowBuf)[l*4 + 3];
								if(tempPixVal != 0)
								{
									((unsigned short *)rowBuf)[(rowOff + l)*4] = ((unsigned short *)tileRowBuf)[l*4];
									((unsigned short *)rowBuf)[(rowOff + l)*4 + 1] = ((unsigned short *)tileRowBuf)[l*4 + 1];
									((unsigned short *)rowBuf)[(rowOff + l)*4 + 2] = ((unsigned short *)tileRowBuf)[l*4 + 2];
									((unsigned short *)rowBuf)[(rowOff + l)*4 + 3] = ((unsigned short *)tileRowBuf)[l*4 + 3];
								}
							}
							break;
						}
						break;
		
					default:
						printf("\nwStitch::writeMontageToFile(): Unsupported number of image channels for blending");
						printf("\nwStitch::writeMontageToFile(): Skipping pixels");
						break;
					}
					
				}
			} // for(k ..
		} // for (j ..

		// write the row to the output Tiff
		printf("\rMerging and writing row %d of %d", i, outImgH - 1);
		TIFFWriteEncodedStrip(tiffImg, i, rowBuf, rowBufSize);
	}

	// cleanup
	TIFFClose(tiffImg);
	free(rowBuf);
	free(tileRowBuf);

	return true;
} // End of writeMontageToFile


bool wStitch::writeMontageToTiledTiffFile(struct wStichRemap* remapInfo, char *tempDir, 
					char *outFile, int tiffTileWidth, int tiffTileHeight)
{
	int minX, minY, maxX, maxY;	
	int maxTileW, maxTileH;
	int outImgW, outImgH;
	int outImgDepth = 0, outImgChannels = 0;
	unsigned int i, j, k, l, m, n;

	uint64 estFileSize;

	minX = minY = maxX = maxY = 0;
	maxTileW = maxTileH = 0;

	int periodOff = 0;
	char extn[32] = "";
	TIFF *tiffImg, *tempLevelImg, *currLevelImg, *nextLevelImg;
	IplImage *tempImg;
	bool bigTiffFlag = false, tempLevelBigTiffFlag = false;
	unsigned char * rowBuf = NULL;	// stores one row for the output image
	unsigned char *tiffTileBuf = NULL;	// Stores one row for the tiles being read
	unsigned char *tileRowBuf; // Stores one whole row of tiles
	int tiffTileBufSize;

	char tempLevelFile[WS_FILENAME_LEN];
	char currLevelFile[WS_FILENAME_LEN];
	char nextLevelFile[WS_FILENAME_LEN];

	// detect the extension
	for(i = (int)(strlen(outFile) - 1); i>=0; i--)
	{
		if(outFile[i] == '.')
		{
			periodOff = i;
			break;
		}
	}

	strcpy(extn, outFile + periodOff + 1);
	// check extension of output file. Currently only TIFF is supported
	if(!(strcmp(extn, "tif") == 0 || strcmp(extn, "tiff") == 0 || 
		strcmp(extn, "TIF") == 0 || strcmp(extn, "TIFF") == 0 ))
	{
		printf("\nwStitch::writeMontageToFile(): Current supported output format is TIFF");
		return false;
	}

	// Write out the TIFF file as a flat non tiled image temp file
	sprintf(tempLevelFile,"%s/level0_temp.tif", tempDir);
	printf("\nwStitch::writeMontageToFile(): Writing temp Level0 file. This might take a while ... "); fflush(stdout);
	if(!writeMontageToFile(remapInfo, tempLevelFile))
	{
		printf("\nwStitch::writeMontageToFile(): Error generating Level 0 temp file ( %s ). Quitting !!", tempLevelFile);
		return false;
	}

	// figure out the actual dimensions of the final image
	for(j = 0; j < remapInfo->rows; j++)
	{
		for(i = 0; i < remapInfo->cols; i++)
		{
			if(remapInfo->tiles[i][j].remappedX < minX)
				minX = remapInfo->tiles[i][j].remappedX;
			if((remapInfo->tiles[i][j].remappedX + remapInfo->tiles[i][j].width) > maxX)
				maxX = remapInfo->tiles[i][j].remappedX + remapInfo->tiles[i][j].width;

			if(remapInfo->tiles[i][j].remappedY < minY)
				minY = remapInfo->tiles[i][j].remappedY;
			if((remapInfo->tiles[i][j].remappedY + remapInfo->tiles[i][j].height) > maxY)
				maxY = remapInfo->tiles[i][j].remappedY +  remapInfo->tiles[i][j].height;

			if(remapInfo->tiles[i][j].width > maxTileW)
				maxTileW = remapInfo->tiles[i][j].width;
			if(remapInfo->tiles[i][j].height > maxTileH)
				maxTileH = remapInfo->tiles[i][j].height;
		}
	}

	outImgW = maxX - minX;
	outImgH = maxY - minY;

	// load one image to read in the format
	tempImg = loadImage(remapInfo->tiles[0][0].tileFileName);
	if(!tempImg)
	{
		printf("\nwStitch::writeMontageToTiledTiffFile(): Cannot load image for tile (0,0) ");
		return false;
	}

	outImgDepth = tempImg->depth;
	outImgChannels = tempImg->nChannels;
	unsigned bytesPerPixel = outImgDepth * outImgChannels / 8;
	// close the tempImg
	cvReleaseImage(&tempImg);

	// double the size because that's what an uncompressed tiled pyramid would take
	estFileSize = 2 * (uint64)outImgW * (uint64)outImgH * (uint64)outImgDepth * (uint64)outImgChannels / 8;

	// Mark if the Tiff files we are dealing with are BigTIFF
	if(estFileSize > WS_32BIT_TIFF_FILE_LIMIT) bigTiffFlag = true;
	else
		bigTiffFlag = false;

	if((estFileSize / 2) > WS_32BIT_TIFF_FILE_LIMIT) tempLevelBigTiffFlag = true;
	else
		tempLevelBigTiffFlag = false;
	
	// Open the final output image to be used as tile TIFF
	if(!bigTiffFlag)
	{
		if((tiffImg = TIFFOpen(outFile, "w4")) == NULL){		
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error writing %s as 32-bit TIFF.", outFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}
	}
	else
	{
		if((tiffImg = TIFFOpen(outFile, "w8")) == NULL){		
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error writing %s as 64-bit TIFF.", outFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}
	}

	// Open the tempLevel0 Tiff file for reading
	if(!tempLevelBigTiffFlag)
	{
		if((tempLevelImg = TIFFOpen(tempLevelFile, "r4")) == NULL){		
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading %s as 32-bit TIFF.", tempLevelFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}
	}
	else
	{
		if((tempLevelImg = TIFFOpen(tempLevelFile, "r8")) == NULL){		
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading: %s as 64-bit TIFF.", tempLevelFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}
	}

	// populate Tiff Tags for level 0
	TIFFSetField(tiffImg, TIFFTAG_TILEWIDTH, tiffTileWidth);
	TIFFSetField(tiffImg, TIFFTAG_TILELENGTH, tiffTileHeight);

	TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, outImgW);
	TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, outImgH);
	TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, outImgDepth);
	TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, outImgChannels);
//TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	if(outImgChannels == 1)
		TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	if(outImgChannels == 3 || outImgChannels == 4)
		TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField( tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	

	// Figure out the number of levels we'll be generating for multires
	unsigned int numTiffLevels = 1;
	unsigned int estLowestLevelDim;

	estLowestLevelDim = WS_MAX(outImgW, outImgH);

	// Calculate how many levels of multi-res we need
	while(estLowestLevelDim > WS_TIFF_LOWESTLEVEL_DIM)
	{
		estLowestLevelDim /= 2; 
		++numTiffLevels;
	}

	unsigned int tiffLevelW, tiffLevelH;
	unsigned int tiffLevelNumTileCols, tiffLevelNumTileRows;
	float progress;

	// for level 0 (lowest level of pyramid)
	tiffLevelW = outImgW;
	tiffLevelH = outImgH;
	tiffLevelNumTileCols = (unsigned)ceil((double)outImgW/(double)tiffTileWidth);
	tiffLevelNumTileRows = (unsigned)ceil((double)outImgH/(double)tiffTileHeight);

	// allocate memory for one tile.
	tiffTileBufSize = tiffTileWidth * tiffTileHeight * bytesPerPixel;
	tiffTileBuf = (unsigned char *)malloc(tiffTileBufSize);
	memset((void *)tiffTileBuf, 0, tiffTileBufSize);

	// Allocate memory for reading one entire row of tiles from the tempLevel0 image.
	// This is the most efficient way to convert the flat tiff to tile tiff.
	uint64 tileRowBufSize = tiffTileWidth * tiffTileHeight * tiffLevelNumTileCols * 
										bytesPerPixel;
	tileRowBuf = (unsigned char *)malloc((size_t)tileRowBufSize);

	if(tileRowBuf == NULL)
	{
		std::cout << "\nwStitch::writeMontageToTiledTiffFile(): Error allocating " << tileRowBufSize
					<< " Bytes for storing a row of tiles " << std::endl;
		return false;
	}

	// read an entire row of tiles from the tempLevel0 file and store it as tiles in the
	// output file
	progress = 0.0;
	printf("\nGenerating Level 1 / %d : %0.2f %%", numTiffLevels, progress);
	for(i = 0; i < tiffLevelNumTileRows; i++)
	{
		memset((void *)tileRowBuf, 0, (size_t)tileRowBufSize);
		for(j=0; j<tiffTileHeight; j++)
		{
			// break if last row is read
			if((j + i*tiffTileHeight) >= outImgH) break;

			if(TIFFReadEncodedStrip(tempLevelImg, j + i*tiffTileHeight, 
					(tdata_t)(tileRowBuf + j * tiffTileWidth * tiffLevelNumTileCols * bytesPerPixel),
					(tsize_t) -1) == -1)
			{
				printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading strip %d from %s",
                                                j + i*tiffTileHeight, tempLevelFile);
				perror("\nwStitch::writeMontageToTiledTiffFile");
			}
		}

		// break up the row of tiles to individual tiles and write up in the output file
		for(j=0; j< tiffLevelNumTileCols; j++)
		{
			memset((void *)tiffTileBuf, 0, tiffTileBufSize);
			for(k=0; k<tiffTileHeight; k++)
			{
				memcpy((void*)(tiffTileBuf + k * tiffTileWidth * bytesPerPixel), 
					(void*)(tileRowBuf + (k * tiffTileWidth * tiffLevelNumTileCols + j * tiffTileWidth) * bytesPerPixel), 
					tiffTileWidth * bytesPerPixel);
			}

			// write to tile tiff
//			if(TIFFWriteEncodedTile(tiffImg, TIFFComputeTile(tiffImg, j, i, 0, 0), (tdata_t)tiffTileBuf, (tsize_t)-1 ) == -1)
			if(TIFFWriteEncodedTile(tiffImg, i*tiffLevelNumTileCols + j, (tdata_t)tiffTileBuf, (tsize_t)-1 ) == -1)
			{
				printf("\nwStitch::writeMontageToTiledTiffFile(): Could not write tile %dx%d", j, k);
				perror("\nwStitch::writeMontageToTiledTiffFile");

				TIFFClose(tiffImg);
				free(tiffTileBuf);
				free(tileRowBuf);
				TIFFClose(tempLevelImg);
				return false;
			}
		} // for (j..

		progress = (float)i / (float)(tiffLevelNumTileRows - 1) * 100.0f;
		printf("\rGenerating Level 1 / %d : %0.2f %%", numTiffLevels, progress);
	} // for (i ..
	
	// Close level 0 directory and the tiff file. Re-open in read mode
	TIFFWriteDirectory(tiffImg);
	TIFFClose(tiffImg);

	// clean up and delete the tempLevel0 file. We don't need it anymore
	free(tileRowBuf);
	TIFFClose(tempLevelImg);
	_unlink(tempLevelFile);

	// For level 0, the outFile becomes the base level for level 1. Level 1 .. n levels are written as
	// temp files in the temp directory. At the very end, the level 1 .. n are added to outFile which already
	// contains level 0

	// Create OpenCV image which will be used for storing the 2x2 tile group from a level
	IplImage *currLevelTile = cvCreateImage(cvSize(tiffTileWidth, tiffTileHeight), 
							outImgDepth, outImgChannels);
	// OpenCV image used to store the resized image
	IplImage *resizedTile = cvCreateImage(cvSize(tiffTileWidth / 2, tiffTileHeight / 2), 
							outImgDepth, outImgChannels);

	strcpy(currLevelFile, outFile);
	// Now that we have the base level, start subsampling tiles to generate subsequent levels
	for(i = 0; i < numTiffLevels - 1; i++)
	{
		progress = 0.0f;

		// open the curr level for reading. Watch out for the 'special' level 0 case. Level 1 onwards are
		// stored as bigtiff by default
		if(i == 0)
		{
			if(!bigTiffFlag)
			{
				if((currLevelImg = TIFFOpen(currLevelFile, "r4")) == NULL){		
					printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading %s as 32-bit TIFF.", currLevelFile);
					perror("\nwStitch::writeMontageToTiledTiffFile");
					fflush(stdout);
					return false;
				}
			}
			else
			{
				if((currLevelImg = TIFFOpen(currLevelFile, "r8")) == NULL){		
					printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading %s as 64-bit TIFF.", currLevelFile);
					perror("\nwStitch::writeMontageToTiledTiffFile");
					fflush(stdout);
					return false;
				}
			}
		}
		else
		{
			if((currLevelImg = TIFFOpen(currLevelFile, "r8")) == NULL){		
				printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading %s as 64-bit TIFF.", currLevelFile);
				perror("\nwStitch::writeMontageToTiledTiffFile");
				fflush(stdout);
				return false;
			}
		}

		sprintf(nextLevelFile,"%s/level_%d.tif", tempDir, i+1);

		// open current level for writing. Always do
		if((nextLevelImg = TIFFOpen(nextLevelFile, "w8")) == NULL){			
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error writing %s as 64-bit TIFF.", nextLevelFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}

		// populate Tiff Tags for the next level
		TIFFSetField(nextLevelImg, TIFFTAG_IMAGEWIDTH, tiffLevelW / 2);
		TIFFSetField(nextLevelImg, TIFFTAG_IMAGELENGTH, tiffLevelH / 2);
		TIFFSetField(nextLevelImg, TIFFTAG_BITSPERSAMPLE, outImgDepth);
		TIFFSetField(nextLevelImg, TIFFTAG_SAMPLESPERPIXEL, outImgChannels);
//		TIFFSetField(nextLevelImg, TIFFTAG_ROWSPERSTRIP, 0);
		TIFFSetField(nextLevelImg, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		if(outImgChannels == 1)
			TIFFSetField(nextLevelImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		if(outImgChannels == 3 || outImgChannels == 4)
			TIFFSetField(nextLevelImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(nextLevelImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(nextLevelImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField(nextLevelImg, TIFFTAG_TILEWIDTH, tiffTileWidth);
		TIFFSetField(nextLevelImg, TIFFTAG_TILELENGTH, tiffTileHeight);

		unsigned tileNum;
		tileNum = 0;

		printf("\nGenerating Level %d / %d : %0.2f %%", i+2, numTiffLevels, progress);

		// go through the tiles in this level. Group them as 2x2 and resize them into one tile
		for(j=0; j<tiffLevelNumTileRows; j+=2)
		{
			if(j >= tiffLevelNumTileRows) break;

			for (k=0; k<tiffLevelNumTileCols; k+=2)
			{
				if (k >= tiffLevelNumTileCols) break;

				memset((void *)tiffTileBuf, 0, tiffTileBufSize);
				// loops for going through a 2x2 tile group in the current level
				for(l=0; l<2; l++)
				{
					for(m=0; m<2; m++)
					{
						memset((void *)currLevelTile->imageData, 0, tiffTileWidth * tiffTileHeight * 
																		bytesPerPixel);

						// Boundary conditions
						if((j + l) >= tiffLevelNumTileRows) continue;
						if((k + m) >= tiffLevelNumTileCols) continue;

//						if(TIFFReadEncodedTile(currLevelImg, TIFFComputeTile(currLevelImg, k + m, j + l, 0, 0), 
//												(tdata_t)currLevelTile->imageData, (tsize_t)-1 ) == -1)
						if(TIFFReadEncodedTile(currLevelImg, (j + l)*tiffLevelNumTileCols + (k+m), 
												(tdata_t)currLevelTile->imageData, (tsize_t)-1 ) == -1)
						continue;

						// resize the tile to half it's size
						cvResize(currLevelTile, resizedTile, INTER_LINEAR);

						// copy the resized tile to the correct quadrant in the tiffTileBuf
						for(n=0; n < resizedTile->height; n++)
							memcpy((void*)(tiffTileBuf + ((l* resizedTile->height + n) * tiffTileWidth + m * resizedTile->width) * bytesPerPixel), 
								(void*)(resizedTile->imageData + n*resizedTile->widthStep), 
								resizedTile->width * bytesPerPixel );

					} // for (m..

				} // for ( l..

				// dump the resized tile to the correct place in the new level
				TIFFWriteEncodedTile(nextLevelImg, tileNum++,
									(tdata_t)tiffTileBuf, (tsize_t)-1 );

			} // for (k ..

			progress = (float )j / (float)(tiffLevelNumTileRows - 2) * 100.0f;
			if(progress > 100.0f) progress = 100.0f;
			printf("\rGenerating Level %d / %d : %0.2f %%", i+2, numTiffLevels, progress);

		} // for (j ..

		TIFFClose(currLevelImg);
		TIFFClose(nextLevelImg);

		// Calculate dimensions for the next level
		tiffLevelW /= 2;
		tiffLevelH /= 2;
		tiffLevelNumTileCols = (unsigned)ceil((double)tiffLevelW/(double)tiffTileWidth);
		tiffLevelNumTileRows = (unsigned)ceil((double)tiffLevelH/(double)tiffTileHeight);
		
		strcpy(currLevelFile, nextLevelFile);
		
	} // for (i..

	// cleanup
	cvReleaseImage(&currLevelTile);
	cvReleaseImage(&resizedTile);

	// Now that all the levels have been generated, consolidate them in with the level 0
	// in the output file
	if(!bigTiffFlag)
	{
		if((tiffImg = TIFFOpen(outFile, "a4")) == NULL){		
			std::cout << "\nwStitch::writeMontageToTiledTiffFile(): Error writing : " << outFile
					<< " as 32-bit TIFF" << std::endl;
			return false;
		}
	}
	else
	{
		if((tiffImg = TIFFOpen(outFile, "a8")) == NULL){		
			std::cout << "\nwStitch::writeMontageToTiledTiffFile(): Error writing : " << outFile
					<< " as 64-bit TIFF" << std::endl;
			return false;
		}
	}

	// go through all generated level files
	for(i = 1; i < numTiffLevels; i++)
	{
		printf("\nConsolidating Level %d / %d into output file .. Please wait", i+1, numTiffLevels);

		sprintf(currLevelFile,"%s/level_%d.tif", tempDir, i);
		if((currLevelImg = TIFFOpen(currLevelFile, "r8")) == NULL){		
			printf("\nwStitch::writeMontageToTiledTiffFile(): Error reading %s as 64-bit TIFF.", currLevelFile);
			perror("\nwStitch::writeMontageToTiledTiffFile");
			fflush(stdout);
			return false;
		}

		// populate Tiff Tags for this level based on what we get from the currLevelImg
		TIFFGetField(currLevelImg, TIFFTAG_IMAGEWIDTH, &tiffLevelW);
		TIFFGetField(currLevelImg, TIFFTAG_IMAGELENGTH, &tiffLevelH);
		TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, tiffLevelW);
		TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, tiffLevelH);
		TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, outImgDepth);
		TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, outImgChannels);
	//TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, 256);
		TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		if(outImgChannels == 1)
			TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		if(outImgChannels == 3 || outImgChannels == 4)
			TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField( tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField(tiffImg, TIFFTAG_TILEWIDTH, tiffTileWidth);
		TIFFSetField(tiffImg, TIFFTAG_TILELENGTH, tiffTileHeight);

		for( j = 0; j < TIFFNumberOfTiles(currLevelImg); j++)
		{
			TIFFReadEncodedTile(currLevelImg, j, (tdata_t)tiffTileBuf, (tsize_t)-1 );
			TIFFWriteEncodedTile(tiffImg, j, (tdata_t)tiffTileBuf, (tsize_t)-1 );
		}

		TIFFWriteDirectory(tiffImg);
		TIFFClose(currLevelImg);
	} // for (i ...

	printf("\n");
	TIFFClose(tiffImg);
	free(tiffTileBuf);

	return true;

} // End of writeMontageToTiledTiffFile





bool wStitch::convertToTempTifFilePath(char *fileName, char *tempDir, char *tempFileName)
{
	char tempStr[512];
	int fileNameOff = 0;
	int fileExtnOff = 0;
	int i; 

	if (!fileName || strcmp(fileName,"") == 0 || !tempFileName) return false;

	// find where the extension and fileName is
	for( i = (int)strlen(fileName) - 1; i>= 0; i--)
	{
		if(fileExtnOff == 0 && fileName[i] == '.') fileExtnOff = i;
		if(fileName[i] == '\\' || fileName[i] == '/')
		{
			fileNameOff = i;
			break;
		}
	}

	strcpy(tempStr, fileName);
	strcpy(tempStr + fileExtnOff, ".tif");

	// write the data into the output string provided
	if(strcmp(tempDir, "") == 0)
		sprintf(tempFileName, "./%s", tempStr + fileNameOff + 1);
	else
		sprintf(tempFileName, "%s/%s", tempDir, tempStr + fileNameOff + 1);
	
	return true;

} // end of convertToTempFilePath



bool wStitch::readTifRow(char *fileName, int rowNum, void *buf)
{
	TIFF *tiffImg;
	bool bigTiffFlag = false;

	// Open the TIFF image as 32 bit input. If it fails try as 64-bit (BigTiff)
	if((tiffImg = TIFFOpen(fileName, "r4")) == NULL){		
		if((tiffImg = TIFFOpen(fileName, "r8")) == NULL)
		{
			printf("\nwStitch::readTifRow(): Error opening : %s", fileName);
			perror("\nwStitch::readTifRow()");
			fflush(stdout);
			return false;
		}
		else
			bigTiffFlag = true;
	}
	
	// copy data using a strip interface from libtiff. Since the tempFile were stored as one row / strip
	TIFFReadEncodedStrip(tiffImg, rowNum, buf, (tsize_t) -1);

	TIFFClose(tiffImg);
	return true;

} // end of readTifRow


void wStitch::alphaComposite(IplImage* img1 , CvPoint img1Pix, float img1Alpha,
						IplImage* img2 , CvPoint img2Pix, float img2Alpha,
						IplImage* resImg, CvPoint resPix)
{
	int tempIntVal;
	
	// Before doing the alpha blending check if the img2 pixel is 0 and don't do the compositing. 
	// This is a special case that causes artifacts in the image. Its ok for img1 pixel to be 0 
	// since img1 is overridden and has less preferece.

	switch (img1->nChannels)
	{
	case 1: // grayscale
		switch(img1->depth)
		{
		case 8:
			tempIntVal = (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x]);
			if(tempIntVal == 0) return;

			*(unsigned char *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x]);
			break;

		case 16:
			tempIntVal = (unsigned short)(*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 2]));
			if(tempIntVal == 0) return;

			*((unsigned short *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 2]) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 2]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 2])));

			break;
		}
		break;

	case 3: // BGR
		switch(img1->depth)
		{
		case 8:
			tempIntVal = (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3])
				+ (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3 + 1])
				+ (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3 + 2]);
			if (tempIntVal == 0) return;

			*(unsigned char *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 3] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 3]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3]);

			*(unsigned char *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 3 + 1] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 3 + 1]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3 + 1]);

			*(unsigned char *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 3 + 2] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 3 + 2]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 3 + 2]);

			break;

		case 16:
			tempIntVal = (unsigned short)(*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6]))
				+ (unsigned short)(*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6 + 2]))
				+ (unsigned short)(*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6 + 4]));
			if (tempIntVal == 0) return;

			*((unsigned short *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 6]) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 6]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6])));

			*((unsigned short *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 6 + 2]) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 6 + 2]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6 + 2])));

			*((unsigned short *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 6 + 4]) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 6 + 4]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 6 + 4])));

			break;
		}

		break;

	case 4: // BGRA
		
		switch(img1->depth)
		{
		case 8:
			tempIntVal = (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4])
				+ (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4 + 1])
				+ (unsigned char)(img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4 + 2]);
			if (tempIntVal == 0) return;


			*(unsigned char *)resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 4] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 4]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4]);

			*(unsigned char *)resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 4 + 1] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 4 + 1]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4 + 1]);

			*(unsigned char *)resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 4 + 2] =
				(unsigned char)(img1Alpha * img1->imageData[img1Pix.y * img1->widthStep + img1Pix.x * 4 + 2]) + 
				(unsigned char)(img2Alpha * img2->imageData[img2Pix.y * img2->widthStep + img2Pix.x * 4 + 2]);

			break;

		case 16:
			tempIntVal = *(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 ])
						+ *(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 + 2 ])
						+ *(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 + 4 ]);
			if(tempIntVal == 0) return;


			*((unsigned short *)&resImg->imageData[resPix.y * resImg->widthStep + resPix.x * 8]) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[ img1Pix.y * img1->widthStep + img1Pix.x * 8 ]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 ])));

			*((unsigned short *)&resImg->imageData + resPix.y * resImg->widthStep + resPix.x * 8 + 2) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[ img1Pix.y * img1->widthStep + img1Pix.x * 8 + 2 ]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 + 2 ])));

			*((short *)&resImg->imageData + resPix.y * resImg->widthStep + resPix.x * 8 + 4) =
				(unsigned short)(img1Alpha * (*(unsigned short *)(&img1->imageData[ img1Pix.y * img1->widthStep + img1Pix.x * 8 + 4 ]))) + 
				(unsigned short)(img2Alpha * (*(unsigned short *)(&img2->imageData[ img2Pix.y * img2->widthStep + img2Pix.x * 8 + 4 ])));

			break;
		}

		break;

	default:
		printf("\nwStitch::alphaComposite: Unsupported number of channels");
		break;
	}


	return;
} // End of alphaComposite


void wStitch::mergeTiles(unsigned char *tile1, unsigned char *tile2, 
					unsigned tileWidth, unsigned tileHeight, 
					unsigned outImgDepth, unsigned outImgChannels)
{

	unsigned int i, j, rowOff;
	int tempPixVal;

	switch(outImgChannels)
	{
	case 1: // grayscale
		switch(outImgDepth)
		{
		case 8:
			for(j = 0; j< tileHeight; j++)
			{
				rowOff = j * tileHeight;
				for(i = 0; i < tileWidth; i++)
				{
					if(tile2[rowOff + i] != 0)
						tile1[rowOff + i] = tile2[rowOff + i];
				}
			}
			break;

		case 16:
			for(j = 0; j< tileHeight; j++)
			{
				rowOff = j * tileHeight;
				for(i = 0; i < tileWidth; i++)
				{
					if(((unsigned short *)tile2)[rowOff + i] != 0)
						((unsigned short *)tile1)[rowOff + i] = ((unsigned short *)tile2)[rowOff + i];
				}
			}
			break;
		}
		break;

	case 3: // BGR
		switch(outImgDepth)
		{
		case 8:
			for(j = 0; j< tileHeight; j++)
			{
				rowOff = j * tileHeight;
				for(i = 0; i < tileWidth; i++)
				{
					tempPixVal = (unsigned char)tile2[(rowOff + i)*3] + 
							(unsigned char)tile2[(rowOff + i)*3 + 1] + 
							(unsigned char)tile2[(rowOff + i)*3 + 2];

					if(tempPixVal != 0)
					{
						tile1[(rowOff + i)*3] = tile2[(rowOff + i)*3];
						tile1[(rowOff + i)*3 + 1] = tile2[(rowOff + i)*3 + 1];
						tile1[(rowOff + i)*3 + 2] = tile2[(rowOff + i)*3 + 2];
					}
				}
			}

			break;

		case 16:
			for(j = 0; j< tileHeight; j++)
			{
				rowOff = j * tileHeight;
				for(i = 0; i < tileWidth; i++)
				{
					tempPixVal = ((unsigned short *)tile2)[(rowOff + i)*3] + 
							((unsigned short *)tile2)[(rowOff + i)*3 + 1] + 
							((unsigned short *)tile2)[(rowOff + i)*3 + 2];

					if(tempPixVal != 0)
					{
						((unsigned short *)tile1)[(rowOff + i)*3] = ((unsigned short *)tile2)[(rowOff + i)*3];
						((unsigned short *)tile1)[(rowOff + i)*3 + 1] = ((unsigned short *)tile2)[(rowOff + i)*3 + 1];
						((unsigned short *)tile1)[(rowOff + i)*3 + 2] = ((unsigned short *)tile2)[(rowOff + i)*3 + 2];
					}
				}
			}
		}
		break;

	case 4: // BGRA .. Implement it later if needed

	default:
		printf("\nwStitch::mergeTiles(): Unsupported number of image channels for blending");
		printf("\nwStitch::mergeTiles(): Skipping pixels");
		break;
	}

}
