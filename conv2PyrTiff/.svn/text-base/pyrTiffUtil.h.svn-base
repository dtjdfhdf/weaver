#ifndef PYRTIFFUTIL_H
#define PYRTIFFUTIL_H

#include <stdio.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include <tiffio.h>


#ifdef WIN32

#else
        #include <inttypes.h>

//	typedef uint64_t uint64;
        #define _unlink	 unlink
#endif

#define PTU_32BIT_TIFF_FILE_LIMIT		4290000000 // 2^32 ~ 4GB, leaving a little for Tags and headers

#define PTU_MIN(X, Y)					((X) < (Y) ? (X) : (Y))
#define PTU_MAX(X, Y)					((X) > (Y) ? (X) : (Y))

#define PTU_TIFFTILE_W					256
#define PTU_TIFFTILE_H					256
#define PTU_TIFF_LOWESTLEVEL_DIM		1024

// fwd declarations
bool isPyrTiff(char *fileName);
bool convToPyrTiff(char *inFile, char *outFile, unsigned short compression,
                   int tileW, int tileH,
                   void(*progressCb)(int), char *errStr);
                                                // On failure, an error string is place in errStr.
                                                // The progress parameter is updated through out the
                                                // conversion process and can be used to update a progress
                                                // bar. The callback updateProgress() is used to update
                                                // an integer between 0 to 100.

void deleteTempPyrFiles(char * fileName);       // Remove the temp files associated with the tif
                                                // file specified in the parameter


#endif // PYRTIFFUTIL_H
