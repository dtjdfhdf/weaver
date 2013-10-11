/************************************************************************************

	Author		:		Raj Singh (raj@ncmir.ucsd.edu)
	Status		:		Aplha

	Description	:		Class for OpenCV based code to cross-correlate and blend a pair
						of images. Output initially will be another image file but this
						does not scale for large sets of images so will need to be changed

						[1] Do not be surprised if the code fails for cases where the images
							being stiched are not the same size (approx same size should be ok
							
************************************************************************************/

#ifndef _WSTITCH_H
#define _WSTITCH_H

#include <iostream>
#include <tiffio.h>
#include <cv.h>
#include <highgui.h>



#ifdef WIN32

#else
	#include <inttypes.h>

//	typedef uint64_t uint64;
	#define _unlink	 unlink
#endif

#define WS_32BIT_TIFF_FILE_LIMIT		4290000000	// 2^32 ~ 4GB, leaving a little for Tags and headers

#define WS_MIN(X, Y)				((X) < (Y) ? (X) : (Y))
#define WS_MAX(X, Y)				((X) > (Y) ? (X) : (Y))

#define WS_FILENAME_LEN				512
#define WS_SMALL_IMG_DIM			512		// Big images are scaled down to this dimension
													// along the overlap.
#define WS_SCALING_THRESH_DIM			1000		// if the dimension is bigger than this, scaling
													// down is considered.
#define WS_MIN_WIDTH_OVERLAP			0.02f		// will assume that there is at least a certain percentage of
#define WS_MAX_WIDTH_OVERLAP			0.5f		// overlap between images. Might not go up to 1.0
#define WS_NUM_TEMPL_ALONG_EDGE			4
#define WS_LOW_TEMPL_MATCH_SCORE		(int)(1.5 * WS_NUM_TEMPL_ALONG_EDGE)
                                                                // of the numerous templates matched if the best candidate has
                                                                // less than these many similar matches, the result is discarded

#define WS_TEMPL_CLUSTER_SQDIST                 36              // This is the square of distance of pixels within which a templ
                                                                // matching result is considered to be similar
#define WS_SEED_SIZE_FACTOR                     10		// factor by which the micro-structures in the image
                                                                // are smaller than the macro structures (an emperical guess)



#define WS_ALPHABLEND_LEN_FACTOR		0.001f		// Percentage of the tile width that is used as the brush
                                                                // thickness for alpha blending.

#define WS_SHORTDIM_GRID_FACTOR			2 //4           // number of seed points placed along the short dimension of the overlap
#define WS_LONGDIM_GRID_FACTOR			16		// number of seed points places along the long dimention of the overlap

#define WS_TIFFTILE_W				256
#define WS_TIFFTILE_H				256
#define WS_TIFF_LOWESTLEVEL_DIM			1024

#define WS_REMAP_FILE				"weaver.remap"
#define WS_TILE_OFFSET_FACTOR			1.1f		// this is the distance by which a tile is placed next to its neighbor

using namespace cv;

enum wStitchLayout {_WSM_NULL, _WSM_HORIZONTAL, _WSM_VERTICAL};

// Stores information about the individual tiles making up the montage
struct wStitchTile {
	int remappedX;				// The location of the top left corner of the tile wrt tile (0,0)
	int remappedY;				// This is determined after running the found overlap with either
                                                // the tile on the left or top. Can't be run till one of those
                                                // 'previous' tiles have been collected.
	int width;
	int height;
	char tileFileName[WS_FILENAME_LEN];
	char tempTileFileName[WS_FILENAME_LEN];
};

// Stores the remap of the tiles wrt to the row 0 and col 0. The information
// is built up as the tiles are collected (or fed to the program)
struct wStichRemap {
	int rows;
	int cols;
	struct wStitchTile **tiles;			// 2D array of remapped tiles into a common mosaic space

};


class wStitch {
private:
	IplImage *img1, *img2, *grayImg1, *grayImg2;
	IplImage *outImg;
	

	bool findOverlap( IplImage* refImg, IplImage* appendImg, wStitchLayout layout,
								int &matchX, int &matchY);
										// Finds the best match of appendImg on top of refImg and returns 'true' if an
										// overlap was found. 'False' if it wasn't. Returns the X, Y coords of the origin
										// of the overlap wrt to refImg.
	bool watershedBlend( IplImage* refImg, IplImage* appendImg, wStitchLayout layout,
								int matchX, int matchY, bool alphaBlend);
										// This function is called after an overlap is found between the images refImg and 
										// appendImg. The X and Y contain the location of appendImg wrt refImg.
										// It uses the OpenCV watershed algorithm to segment the image and then
										// applies graph cut followed by alpha blending along the graph cut contour
	unsigned int sqDist(CvPoint *, CvPoint *);
										// Calculate the square of distance between the two points. 
										// (oldest trick in the book ;)

	bool readTifRow(char *fileName, int rowNum, void *buf);
										// read in one specified row of Tif file in the buf. Assumption
										// is that the size of row is known previously.

	void alphaComposite(IplImage* img1 , CvPoint img1Pix, float img1Alpha,
						IplImage* img2 , CvPoint img2Pix, float img2Alpha,
						IplImage* resImg, CvPoint resPix);
										// Applies weight to pixels from img1 and img2 and stores the
										// resulting pixel at specified position in resImg

	void mergeTiles(unsigned char *tile1, unsigned char *tile2, 
                                        unsigned tiffTileWidth, unsigned tiffTileHeight,
					unsigned outImgDepth, unsigned outImgChannels);
										// function merges the two tiles together and stores the result in tile1
										// tile2 pixels are given preference unless they are black.

public:

        wStitch();
	~wStitch();

        bool stitch(char *fileName1, char *fileName2,
				char *tempFileName1, char *tempFileName2,
				wStitchLayout layout, 
				int &matchX, int &matchY,
				int &imgW, int &imgH,
                                char *tempDir, bool continueDespiteFailure,
                                bool firstTileOfRow);
										// One assumption in this functon is that the first image 
										// is the 'reference' and the second one is the image to be stitched 
										// on to it. 'layout' specifies whether the images are horizontally or 
										// vertically alligned.Once the blend is calculated the data is 
										// written back to the files. So its better to use temp copies of the 
										// original files. This is done for cases where the final montage is 
										// a lot bigger than memory. After a successful blend, the resulting images
										// are stored in the tempDir as .tif versions. If tempDir is null, 
										// resulting image is stored by forcing overwrite on the original images.


	IplImage* loadImage(char *);		// Opens images using BigTIFF for tiff files and OpenCV for everything else
										// One reason for diong this is that OpenCV internal function for opening
										// 16-bit grayscale images is broken. It returns a 8-bit converted image
										// (Not cool)
										// The image returned by this function should be free'd by the caller after use.

	bool saveImage(IplImage*, char*);	// Save images using the OpenCV internal calls, unless the image is supppsed to
										// be a TIFF

	bool writeMontageToFile(struct wStichRemap* remapInfo, char *outFile);
										// After all the blending has been done, this function is called to 
										// combine the  blended tiles to an output file.
	bool writeMontageToTiledTiffFile(struct wStichRemap* remapInfo, char *tempDir, char *outFile, int tileWidth, int tileHeight);
										// write output as a tiled tiff (also called multi-res tiff)

	bool convertToTempTifFilePath(char *fileName, char *tempDir, char *tempFileName);
										// A utility function that changes fileName to its .tif version
										// in the tempDir and returns the result in blendedFileName
	
};



#endif

