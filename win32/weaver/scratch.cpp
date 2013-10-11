	int pieceLen, pixIntVal;
	int lastRefMarkerVal;
	bool refImgMarkerValFound;
	CvRect tempRoi1, tempRoi2;
	bool edgeCaseFlag;

	float *alphaBrushForX, *alphaBrushForY;
	int alphaBrushForXLen = 0, alphaBrushForYLen = 0;
	int refImgPixX, refImgPixY, appendImgPixX, appendImgPixY;

	// Allocate memory and prep the Alpha brush
	alphaBrushForXLen = (int)(appendImg->width * WS_ALPHABLEND_LEN_FACTOR);
	alphaBrushForYLen = (int)(appendImg->height * WS_ALPHABLEND_LEN_FACTOR);

	// clamp brush lengths to even values
	if(alphaBrushForXLen % 2 != 0) ++alphaBrushForXLen;
	if(alphaBrushForYLen % 2 != 0) ++alphaBrushForYLen;

	alphaBrushForX = (float *)malloc(alphaBrushForXLen * sizeof(float));
	alphaBrushForY = (float *)malloc(alphaBrushForYLen * sizeof(float));

	for(i=0; i<alphaBrushForXLen; i++) alphaBrushForX[i] =  (float)i / (float)alphaBrushForXLen ;
	for(i=0; i<alphaBrushForYLen; i++) alphaBrushForY[i] = (float)i / (float)alphaBrushForYLen ;


	switch(layout)
	{
	case _WSM_HORIZONTAL: // go along rows for blending

		for (j = 0; j < refMarkerGrid->height; j++)
		{
			pieceLen = 0;
			lastRefMarkerVal = -100;
			refImgMarkerValFound = false;
			edgeCaseFlag = false;
	
			// special treatment of edge cases
			if(j == 0) {++j;  edgeCaseFlag = true;}
			if(j == (refMarkerGrid->height - 1)) {--j;  edgeCaseFlag = true;}

			for (i = 0; i < refMarkerGrid->width; i++)
			{
				// how long is the contribution from the refImg. Optimize here .. dont check the set of
				// refImg markers if the value is same as last pass.
				pixIntVal = *(int *)(&refMarkerGrid->imageData[ j * refMarkerGrid->widthStep + i * 4]);
				
				// check for edge case or boundary between segments (marked  with -1 by cvWatershed())
				if(pixIntVal == -1)
				{
					++pieceLen;
					refImgMarkerValFound = true;
					continue;
				}

				if (pixIntVal ==  lastRefMarkerVal)
				{
					++pieceLen;
					refImgMarkerValFound = true;
					continue;
				}
				else
				{
					// check to see if the current pixel value in the segmented marker set is a part
					// of the refImg
					refImgMarkerValFound = false;
					for(k=0; k< setOfSegCloserToRefImgSize; k++)
					{
						if(pixIntVal == setOfSegCloserToRefImg[k])
						{
							++pieceLen;
							refImgMarkerValFound = true;
							lastRefMarkerVal = pixIntVal;
							break;
						}
					}
				}
				
				// if a marker value was not found for the refImg, quit loop. We know the blend point
				if(!refImgMarkerValFound) break;
			} // For (i ..)

			// if it was an edge case, re-adjust
			if(edgeCaseFlag && j == 1) --j;
			if(edgeCaseFlag && j == (refMarkerGrid->height - 2)) ++j;

			// This is where the actual 'blending' is done. 
			
			// Do Aplha blending. pieceLen is the length of contributing pixels from refImg. We will use 
			// this edge point to collect pixels from both ref and appendImg on both sides of the point.
			if(alphaBlend)
			{
				for (k=0; k< alphaBrushForXLen; k++)
				{
					// calculate the pixel locations into the two images
					refImgPixX = refImgRoi.x + pieceLen + k - (int)(alphaBrushForXLen / 2);
					refImgPixY = refImgRoi.y + j;

					appendImgPixX = appendImgRoi.x + pieceLen + k - (int)(alphaBrushForXLen / 2);
					appendImgPixY = appendImgRoi.y + j;

					// ignore if this pixel lies out of bounds
					if(refImgPixX < 0 || refImgPixX >= refImg->width || 
						refImgPixY < 0 || refImgPixY >= refImg->height ||
						appendImgPixX < 0 || appendImgPixX >= appendImg->width ||
						appendImgPixY < 0 || appendImgPixY >= appendImg->height) 
					continue;
					
					// Alpha composite pixel = a.A + b.(1 - A)
					alphaComposite(refImg , cvPoint(refImgPixX, refImgPixY), 1.0f - alphaBrushForX[k],
								appendImg, cvPoint(appendImgPixX, appendImgPixY), alphaBrushForX[k],
								appendImg, cvPoint(appendImgPixX, appendImgPixY));

				}
			}

			// Here we blacken out (value = 0) segments that do not belong to refImg and appendImg
			// The 0 values will be ignored when the final image is being constructed
			
			// Set up pixel line to be marked as 0 but only in the appendImg. We will make sure
			// that the actual blending in writeMontageToFile() gives preference to appendImg pixels

			tempRoi2 = cvRect(appendImgRoi.x, appendImgRoi.y + j, WS_MAX(pieceLen - (int)(alphaBrushForXLen / 2), 0), 1);
			cvSetImageROI(appendImg, tempRoi2);
			cvZero(appendImg);


		} // For (j ..)
		break;

	case _WSM_VERTICAL: // go along columns for blending

		for (i = 0; i < refMarkerGrid->width; i++)
		{
			pieceLen = 0;
			lastRefMarkerVal = -100;
			refImgMarkerValFound = false;
			edgeCaseFlag = false;
	
			// special treatment of edge cases
			if(i == 0) {++i;  edgeCaseFlag = true;}
			if(i == (refMarkerGrid->width - 1)) {--i;  edgeCaseFlag = true;}

			for (j = 0; j < refMarkerGrid->height; j++)
			{
				// how long is the contribution from the refImg. Optimize here .. dont check the set of
				// refImg markers if the value is same as last pass.
				pixIntVal = *((int *)&refMarkerGrid->imageData[ j * refMarkerGrid->widthStep + i * 4]);

				// check for edge case or boundary between segments (marked  with -1 by cvWatershed())
				if(pixIntVal == -1)
				{
					++pieceLen;
					refImgMarkerValFound = true;
					continue;
				}

				if ( pixIntVal ==  lastRefMarkerVal)
				{
					++pieceLen;
					refImgMarkerValFound = true;
					continue;
				}
				else
				{
					// check to see if the current pixel value in the segmented marker set is a part
					// of the refImg
					refImgMarkerValFound = false;
					for(k=0; k< setOfSegCloserToRefImgSize; k++)
					{
						if(pixIntVal == setOfSegCloserToRefImg[k])
						{
							++pieceLen;
							refImgMarkerValFound = true;
							lastRefMarkerVal = pixIntVal;
							break;
						}
					}
				}
				
				// if a marker value was not found for the refImg, quit loop. We know the blend point
				if(!refImgMarkerValFound) break;
			} // For (j ..)

			// if it was an edge case, re-adjust
			if(edgeCaseFlag && i == 1) --i;
			if(edgeCaseFlag && i == (refMarkerGrid->width - 2)) ++i;

			// This is where the 'blending' is done.

			// Do Aplha blending. pieceLen is the length of contributing pixels from refImg. We will use 
			// this edge point to collect pixels from both ref and appendImg on both sides of the point.
			if(alphaBlend)
			{
				for (k=0; k< alphaBrushForYLen; k++)
				{
					// calculate the pixel locations into the two images
					refImgPixX = refImgRoi.x + i;
					refImgPixY = refImgRoi.y + pieceLen + k - (int)(alphaBrushForYLen / 2);

					appendImgPixX = appendImgRoi.x + i;
					appendImgPixY = appendImgRoi.y + pieceLen + k - (int)(alphaBrushForYLen / 2);

					// ignore if this pixel lies out of bounds
					if(refImgPixX < 0 || refImgPixX >= refImg->width || 
						refImgPixY < 0 || refImgPixY >= refImg->height ||
						appendImgPixX < 0 || appendImgPixX >= appendImg->width ||
						appendImgPixY < 0 || appendImgPixY >= appendImg->height) 
					continue;
					
					// Alpha composite pixel = a.A + b.(1 - A)
					alphaComposite(refImg , cvPoint(refImgPixX, refImgPixY), 1.0f - alphaBrushForY[k],
								appendImg, cvPoint(appendImgPixX, appendImgPixY), alphaBrushForY[k],
								appendImg, cvPoint(appendImgPixX, appendImgPixY));

				}
			}
			
			// Here we blacken out (value = 0) segments that do not belong to refImg and appendImg
			// The 0 values will be ignored when the final image is being constructed
			
			// Set up pixel line to be marked as 0 but only in the appendImg. We will make sure
			// that the actual blending in writeMontageToFile() gives preference to appendImg pixels
			tempRoi2 = cvRect(appendImgRoi.x + i, appendImgRoi.y, 1,  WS_MAX(pieceLen - (int)(alphaBrushForXLen / 2), 0));
			cvSetImageROI(appendImg, tempRoi2);
			cvZero(appendImg);

		} // For (i ..)

		break;
	}