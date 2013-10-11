
/************************************************************************************

	Author		:		Raj Singh (raj@ncmir.ucsd.edu)
	Status		:		Aplha

	Description	:		Program to invoke the wStich class. Uses the wStich class to
						do auto correlation and watershed based blending.

************************************************************************************/

#include "wStitch.h"
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <mpi.h>

#ifdef WIN32
	#include <stdlib.h>
	#include <direct.h>
#else
	#include <unistd.h>
	#include <stdlib.h>
#endif

wStitchLayout imagesLayout;
char tileFile[WS_FILENAME_LEN], tileFileWoPath[WS_FILENAME_LEN];
char outFile[WS_FILENAME_LEN], outFileWoPath[WS_FILENAME_LEN];
char tempTifTileFile[WS_FILENAME_LEN];
char remapFile[WS_FILENAME_LEN], tempDir[WS_FILENAME_LEN];
char pieceFile[WS_FILENAME_LEN];

bool delTempDir = false;
bool writeOutput = false;
bool newProject = false;
bool alphaBlend = false;
bool tiledTiff = false;
bool continueAfterBlendFailure = false;

int numOfTileCols = 0, numOfTileRows = 0;
int tileLocX = -1, tileLocY = -1;
int tiffTileW = WS_TIFFTILE_W, tiffTileH = WS_TIFFTILE_H;

struct wStichRemap remapInfo;
struct wStitchTile tempTileInfo;

// MPI global variables
int mpiRank, mpiWorldSize, procNameLen;
char procName[WS_FILENAME_LEN];
MPI_Status mpiStatus;

// fwd decls
void printUsage();
bool parseCmdLine(int _argc, char **_argv);
bool createNewProject();
bool loadProject();
bool loadPieceFile();
bool deleteProjectWorkspace();
void normalizePath(char *);
bool syncRemapInfoToDisk();
bool fgetline(char *str, int strlen_max, FILE* fptr);


int main (int argc, char ** argv)
{
	int matchX, matchY, i, j, k;
	IplImage *tempCvImg;
	bool writeOutFileErr = false;
	bool blendFailed = false;
	int noOfComputedRows;
	wStitch *weaver;

	// Init MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);
	MPI_Get_processor_name(procName, &procNameLen);
	
	if(mpiRank == 0)
	{
		printf("\nWeaver : Mosaicing tool for large EM microcopy image datasets");
		printf("\n===============================================================");
		printf("\nCopyright: National Center for Microscopy and Imaging Research, 2010");
		printf("\nAuthor: Raj Singh (rsingh@ncmir.ucsd.edu)");
		printf("\n\n");

		if (argc < 4)
		{
			printUsage();
			return -1;
		}
	}

	// Init variables
	strcpy(tileFile, "");
	strcpy(tempDir, ".");
	strcpy(outFile,"");
	strcpy(outFileWoPath,"");
	sprintf(remapFile, "./%s", WS_REMAP_FILE);


	// Parse command line parameters
	if(!parseCmdLine(argc, argv))
	{
		printf("\nWeaver [%d]: Error parsing command line. Please check parameters.", mpiRank);
		return -1;
	}

	// normalize paths
	normalizePath(tileFile);
	normalizePath(pieceFile);
	normalizePath(outFile);
	normalizePath(tempDir);

	double clock = (double)cvGetTickCount();

	// For a new project if the workspace directory does not exist, create it
	if(newProject)
	{
		// Create workspace .. only applicable to the master process
		if(mpiRank == 0)
		{
			printf("\nWeaver [%d]: Creating new project workspace for a montage of %dx%d tiles", 
				mpiRank, numOfTileCols, numOfTileRows);
			if(!createNewProject())
			{
				printf("\nWeaver: Error creating new project workspace");
				return -1;
			}
		}

		// All other processes just load up the project after its done
		MPI_Barrier(MPI_COMM_WORLD);
		if(mpiRank != 0)
		{
			if(!loadProject())
			{
				printf("\nWeaver [%d]: Error loading project workspace from %s", mpiRank, tempDir);
				return -1;
			}
		}

	}
	else
	{
		// If its not a new project, load up the info in the temp directory
		printf("\nWeaver [%d]: Loading project from %s ... ", mpiRank, remapFile);
		if(!loadProject())
		{
			printf("\nWeaver [%d]: Error loading project workspace from %s", mpiRank, tempDir);
			return -1;
		}

		//printf("Done. Montage size = %dx%d tiles", numOfTileCols, numOfTileRows);
	}

	// Load up the pieceFile on all processes.
	if(strcmp(pieceFile, "") != 0)
	{
		if(!loadPieceFile())
		{
			printf("\nWeaver [%d]: Error loading piecefile from %s", mpiRank, pieceFile);
			fflush(stdout);
			return -1;
		}
		else
			printf("\nWeaver [%d]: Loaded piecefile ( %s ) with %d cols and %d rows", 
				mpiRank, pieceFile, remapInfo.cols, remapInfo.rows);
	}
	else
	{
		if(writeOutput || delTempDir) goto ProjectWrapUp;
	}


	weaver = new wStitch();
	// ************************   PHASE I : One column computed by one process  *************** //
	// For the MPI version of the code, stitching is done in multiple phases. For Phase I, one process
	// computes an entire column(s). The master process does the first column (column 0) and stores
	// the remap info. The other remaps are ignored here 
	for(tileLocX = mpiRank;  tileLocX < remapInfo.cols; tileLocX += mpiWorldSize)
	{ 
		for(tileLocY = 0;  tileLocY < remapInfo.rows; tileLocY++)
		{
			printf("\nWeaver [%d]: Computing overlap and blending for : %s", 
				mpiRank, remapInfo.tiles[tileLocX][tileLocY].tileFileName);
			fflush(stdout);

			strcpy(tileFile, remapInfo.tiles[tileLocX][tileLocY].tileFileName);
			// We want the output blended images in a temp dir. So find out whats the path
			// for the temp .tif version of the tileFile
			weaver->convertToTempTifFilePath(tileFile, tempDir, tempTifTileFile);
	
			// For the special case of tiles of row 0 no processing is done .. just copy over to temp dir
			if(tileLocY == 0)
			{
				tempCvImg = weaver->loadImage(tileFile);
				if(tempCvImg == NULL)
				{
					printf("\nWeaver [%d]: Check file : %s", mpiRank, tileFile);
					return -1;
				}

				remapInfo.tiles[tileLocX][tileLocY].remappedX = 0;
				remapInfo.tiles[tileLocX][tileLocY].remappedY = 0;
				remapInfo.tiles[tileLocX][tileLocY].width = tempCvImg->width;
				remapInfo.tiles[tileLocX][tileLocY].height = tempCvImg->height;

				// save to temp tif file
				if(!weaver->saveImage(tempCvImg, tempTifTileFile))
				{
					printf("\nWeaver [%d]: Could not create temp file: ", mpiRank, tempTifTileFile);
					fflush(stdout);
					return -1;
				}

				cvReleaseImage(&tempCvImg);

				strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
				continue;	
			}

			// If row > 0 blend with tile above
			matchX = matchY = 0;
			if(strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName, "") == 0 || 
				strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName, "") == 0)
			{
				printf("\nWeaver [%d]: Top tile is missing. Check if processing for top tile failed. Cannot continue.", mpiRank);
				fflush(stdout);
				return -1;
			}
			else
			{
				blendFailed = false;
				// When the blend function successfully finishes, it has written
				// the output temp images to the temp directory
				if(! weaver->stitch(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName, 
								remapInfo.tiles[tileLocX][tileLocY].tileFileName,
								remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName,
								remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
								_WSM_VERTICAL, matchX, matchY,
								remapInfo.tiles[tileLocX][tileLocY].width, 
								remapInfo.tiles[tileLocX][tileLocY].height,
								tempDir, alphaBlend))
				{
						printf("\nWeaver [%d]: Error blending images %s and %s.", mpiRank,
						remapInfo.tiles[tileLocX][tileLocY].tileFileName,
						remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName);
						blendFailed = true;
				}

				if(blendFailed)
                                {
                                        // If user does not want the program to quit, move the tile
                                        // by a certain offset so that it stands out in the final image
                                        if(continueAfterBlendFailure)
                                        {
                                                printf("\nWeaver[%d]: Continuing despite failure to blend. Tile will be placed in an approx location",
							mpiRank);
                                                // Basically just copy over the iamge to the temp directory
                                                // IF it already does not exist. If the temp already exists,
                                                // just move the tile
						tempCvImg = weaver->loadImage(remapInfo.tiles[tileLocX][tileLocY].tileFileName);
                                                if(strcmp(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, "") == 0)
                                                {
                                                        if(!weaver->saveImage(tempCvImg, tempTifTileFile))
                                                        {
                                                                printf("\nWeaver: Could not create temp file: ", tempTifTileFile);
                                                                fflush(stdout);
                                                                return -1;
                                                        }
                                                }

                                                remapInfo.tiles[tileLocX][tileLocY].width = tempCvImg->width;
                                                remapInfo.tiles[tileLocX][tileLocY].height = tempCvImg->height;
                                                remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                        remapInfo.tiles[tileLocX][tileLocY - 1].remappedX;
                                                remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                        remapInfo.tiles[tileLocX][tileLocY - 1].remappedY + 
                                                        (int)(WS_TILE_OFFSET_FACTOR * remapInfo.tiles[tileLocX][tileLocY  - 1].height);

                                                cvReleaseImage(&tempCvImg);
                                        }
                                        else
                                                return -1;

                                }
                                else
                                {
                                        // Update the remapInfo structure with the matched coordinates
                                        remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedX + matchX;

                                        remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedY + matchY;
                                }

				strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);

		/*		// update the remap info but only for col 0 (which should be on master process)
				if(tileLocX == 0)
				{
					remapInfo.tiles[tileLocX][tileLocY].remappedX = 
						remapInfo.tiles[tileLocX][tileLocY - 1].remappedX + matchX;

					remapInfo.tiles[tileLocX][tileLocY].remappedY = 
						remapInfo.tiles[tileLocX][tileLocY - 1].remappedY + matchY;
				}
		*/
			}
		} // for (tileLocY ..

	} // for(tileLocX ..

	// Wait for all the processes here before going ahead
	MPI_Barrier(MPI_COMM_WORLD);

	// If the code reached here, all the columns were successfully computed and their temp files were created
	// A bit of a dirty hack but we  will update the all remapInfo on all mpi nodes to reflect the temp files
	// This is needed since the next stage will fail if it cannot find the string for temp files of tile to 
	// the left.
	for(tileLocY = 0; tileLocY < remapInfo.rows; tileLocY++)
	{
		for(tileLocX = 0; tileLocX < remapInfo.cols; tileLocX++)
		{
			weaver->convertToTempTifFilePath(remapInfo.tiles[tileLocX][tileLocY].tileFileName, 
						tempDir, remapInfo.tiles[tileLocX][tileLocY].tempTileFileName);
		}
	}
	

	noOfComputedRows = 0;

	// ************************   PHASE II : Each process computes entire row(s)   *************** //
	// For the MPI version of the code, stitching is done in multiple phases. For Phase II, only entire rows
	// are computed by processes. Then at the end, the remapInfo structures from all processes are collected
	// and merged into one remapInfo structure on the master process
	for(tileLocY = mpiRank; tileLocY < remapInfo.rows; tileLocY+=mpiWorldSize)
	{
		++noOfComputedRows;

		for(tileLocX = 1; tileLocX < remapInfo.cols; tileLocX++)
		{
			printf("\nWeaver [%d]: Computing overlap and blending for : %s", 
				mpiRank, remapInfo.tiles[tileLocX][tileLocY].tileFileName);
			fflush(stdout);

			if (tileLocX >= numOfTileCols || tileLocY >= numOfTileRows)
			{
				printf("\nWeaver [%d]: Tile location (%dx%d) is not within the bounds of this project. Quitting.", 
					mpiRank, tileLocX, tileLocY);
				return -1;
			}
			
			strcpy(tileFile, remapInfo.tiles[tileLocX][tileLocY].tileFileName);
			// We want the output blended images in a temp dir. So find out whats the path
			// for the temp .tif version of the tileFile
			weaver->convertToTempTifFilePath(tileFile, tempDir, tempTifTileFile);

			// Figure out the tile to the left and blend with it. Then figure out the tile on top 
			// and blend with it. Take care of edge cases.
			matchX = matchY = 0;
			if(strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName, "") == 0 ||
				strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName, "") == 0)
			{
				printf("\nWeaver [%d]: Left tile is missing. Check if processing for left tile failed. Cannot go ahead.", mpiRank);
				fflush(stdout);
				return -1;
			}
			else
			{
				blendFailed = false;
				// When the blend function successfully finishes, it has written
				// the output temp images to the temp directory
				if(! weaver->stitch(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName, 
								remapInfo.tiles[tileLocX][tileLocY].tileFileName,
								remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName,
								remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
								_WSM_HORIZONTAL, matchX, matchY,
								remapInfo.tiles[tileLocX][tileLocY].width, 
								remapInfo.tiles[tileLocX][tileLocY].height,
								tempDir, alphaBlend))
				{
					printf("\nWeaver [%d]: Error blending images %s and %s.", mpiRank,
						remapInfo.tiles[tileLocX][tileLocY].tileFileName,
						remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName);
					blendFailed = true;
				}

				if(blendFailed)
                                {
                                        // If user does not want the program to quit, move the tile
                                        // by a certain offset so that it stands out in the final image
                                        if(continueAfterBlendFailure)
                                        {
                                                printf("\nWeaver[%d]: Continuing despite failure to blend. Tile will be placed in an approx location",
							mpiRank);
                                                // Basically just copy over the iamge to the temp directory
                                                // IF it already does not exist. If the temp already exists,
                                                // just move the tile
						tempCvImg = weaver->loadImage(remapInfo.tiles[tileLocX][tileLocY].tileFileName);
                                                if(strcmp(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, "") == 0)
                                                {
                                                        if(!weaver->saveImage(tempCvImg, tempTifTileFile))
                                                        {
                                                                printf("\nWeaver: Could not create temp file: ", tempTifTileFile);
                                                                fflush(stdout);
                                                                return -1;
                                                        }
                                                }

                                                remapInfo.tiles[tileLocX][tileLocY].width = tempCvImg->width;
                                                remapInfo.tiles[tileLocX][tileLocY].height = tempCvImg->height;
                                                remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                        remapInfo.tiles[tileLocX - 1][tileLocY].remappedX +
                                                        (int)(WS_TILE_OFFSET_FACTOR * remapInfo.tiles[tileLocX - 1][tileLocY].width);
                                                remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                        remapInfo.tiles[tileLocX - 1][tileLocY].remappedY; 

                                                cvReleaseImage(&tempCvImg);
                                        }
                                        else
                                                return -1;

                                }
				else
				{
					// Update the remapInfo structure
					remapInfo.tiles[tileLocX][tileLocY].remappedX = 
						remapInfo.tiles[tileLocX - 1][tileLocY].remappedX + matchX;

					remapInfo.tiles[tileLocX][tileLocY].remappedY = 
						remapInfo.tiles[tileLocX - 1][tileLocY].remappedY + matchY;

					// update the remap info with tempfile that was created for this tile
					strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
				}
			}

		} // for (tileLocX ..
	} // for (tileLocY ..

	// Wait for all the processes here before going ahead
	MPI_Barrier(MPI_COMM_WORLD);


	// *******************************   PHASE III   *************************************** //
	// Collect the remapInfo structures from all processes and merge them into one
	int computedRowIdx;
	if(mpiRank == 0)
	{
		for(i = 1; i< mpiWorldSize; i++)
		{
			// Receive the process' remap info data. One painful element at a time. The master
			// process will only receive the rows that were affected by the slaves and not the
			// entire remapInfo structure

			// First receive how many rows were computed by the slave process ?
			MPI_Recv((void *)&noOfComputedRows, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpiStatus);
			computedRowIdx = i;
			for(j = 0; j < noOfComputedRows; j++)
			{
			   for(k = 1; k < remapInfo.cols; k++)
			   { 
				// receive the remapped values
				MPI_Recv((void *)&remapInfo.tiles[k][computedRowIdx].remappedX, 
					1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpiStatus);
				MPI_Recv((void *)&remapInfo.tiles[k][computedRowIdx].remappedY, 
					1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpiStatus);
				MPI_Recv((void *)&remapInfo.tiles[k][computedRowIdx].width, 
					1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpiStatus);
				MPI_Recv((void *)&remapInfo.tiles[k][computedRowIdx].height, 
					1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpiStatus);

				MPI_Recv((void *)remapInfo.tiles[k][computedRowIdx].tileFileName, 
					WS_FILENAME_LEN, MPI_CHAR, i, 0, MPI_COMM_WORLD, &mpiStatus);
				MPI_Recv((void *)remapInfo.tiles[k][computedRowIdx].tempTileFileName, 
					WS_FILENAME_LEN, MPI_CHAR, i, 0, MPI_COMM_WORLD, &mpiStatus);

				// adjust the remap values based on the tile on the left
				remapInfo.tiles[k][computedRowIdx].remappedX += 
						remapInfo.tiles[0][computedRowIdx].remappedX;
				remapInfo.tiles[k][computedRowIdx].remappedY += 
						remapInfo.tiles[0][computedRowIdx].remappedY;

			   } // for (k..

			   computedRowIdx += mpiWorldSize;				
			} // for (j ..	

		} // for (i ..

		// If the blends were successful, sync the remapInfo structure to disk.
		syncRemapInfoToDisk();
	}
	else
	{
		// Send affected rows of the remapInfo structure to the master process
		MPI_Send((void *)&noOfComputedRows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		computedRowIdx = mpiRank;
		for(j = 0; j < noOfComputedRows; j++)
 		{
                   for(k = 1; k < remapInfo.cols; k++)
                   {
                          // receive the remapped values
                          MPI_Send((void *)&remapInfo.tiles[k][computedRowIdx].remappedX,
                                        1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                          MPI_Send((void *)&remapInfo.tiles[k][computedRowIdx].remappedY,
                                        1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                          MPI_Send((void *)&remapInfo.tiles[k][computedRowIdx].width,
                                        1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                          MPI_Send((void *)&remapInfo.tiles[k][computedRowIdx].height,
                                        1, MPI_INT, 0, 0, MPI_COMM_WORLD);

                          MPI_Send((void *)remapInfo.tiles[k][computedRowIdx].tileFileName,
                                        WS_FILENAME_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                          MPI_Send((void *)remapInfo.tiles[k][computedRowIdx].tempTileFileName,
                                        WS_FILENAME_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

                    } // for (k ..

                    computedRowIdx += mpiWorldSize;
		} // for (j .
	} // else ..

	
	// Hmm .. labels are bad .. I know I know
ProjectWrapUp:

	// Check if user wants to write the tiles to the final output image file
	writeOutFileErr = false;
	if(mpiRank == 0 && writeOutput)
	{
		if(tiledTiff)
		{
			if(!weaver->writeMontageToTiledTiffFile(&remapInfo, tempDir, outFile, tiffTileW, tiffTileH))
			{
				printf("\nWeaver [%d]: Could not write output to %s", mpiRank, outFile);
				writeOutFileErr = true;
			}
		}
		else
		{
			if(!weaver->writeMontageToFile(&remapInfo, outFile))
			{
				printf("\nWeaver []%d: Could not write output to %s", mpiRank, outFile);
				writeOutFileErr = true;
			}
		}
	}


	// Did the user want to delete the temp dir
	if(mpiRank == 0 && !writeOutFileErr && delTempDir) deleteProjectWorkspace();

	clock = (double)cvGetTickCount() - clock;
	if(mpiRank == 0) printf( "\nWeaver: Exec time = %gms\n", clock/(cvGetTickFrequency()*1000.) );

	// sync processes
	MPI_Barrier(MPI_COMM_WORLD);

	delete weaver;
	MPI_Finalize();
	return 0;

} // End of main


void printUsage()
{
	printf("\nUsage: weaver -o <Output Tiff File> -m <Temp dir (optional)> <Commands>");
	printf("\n\nCommands:\n--------\n");
	printf("\n -n <MxN>: Start a new project which will stitch M columns and N rows of tiles");
	printf("\n -p <filename>: Piece-file containing row-wise list of tiles.");
	printf("\n		");
	printf("\n -m <Temporary dir>: Temporary working directory. Default is data directory");
	printf("\n -w	: Write output to disk without deleting temporary files.");
	printf("\n -mr XxY : Write output as multires tiled TIFF with tile size XxY.");
	printf("\n -d	: Delete temporary files for the project");
	printf("\n -c   : Continue even after failure to align a tile. In this case the tile is placed at a fixed offset from its neighbors");
	printf("\n -h	: Print this usage.");
	printf("\n\n");
}


bool parseCmdLine(int _argc, char **_argv)
{
	int i, count;
	for (i = 1; i< _argc; i++)
	{
		if(strcmp(_argv[i],"-h") == 0 || strcmp(_argv[i], "-help") == 0)
		{
			printUsage();
			exit (1);
		}

		// Get the output file name
		if(strcmp(_argv[i],"-o") == 0)
		{
			i++;
			strcpy(outFile, _argv[i]);
	
			// get the file name without the path .. needed later
			for (count = (int)strlen(outFile) - 1; count >=0; count --)
			{
				if(outFile[count] == '/' || outFile[count] == '\\') break;
			}
			strcpy(outFileWoPath, outFile + count + 1);

			// assume that the output directory contains the temp directory.
			// This will change if user specified otherwise
			strcpy(tempDir, outFile);
			strcat(tempDir, ".weaver");
			sprintf(remapFile, "%s/%s", tempDir, WS_REMAP_FILE);
			continue;
		}

		// figure out the temp dir
		if(strcmp(_argv[i],"-m") == 0)
		{
			i++;
			strcpy(tempDir, _argv[i]);
			strcat(tempDir,"/");
			strcat(tempDir, outFileWoPath);
			strcat(tempDir, ".weaver");
			sprintf(remapFile, "%s/%s", tempDir, WS_REMAP_FILE);
			continue;
		}
		
		// Is it a new project
		if(strcmp(_argv[i], "-n") == 0)
		{
			newProject = true;
			i++;
			sscanf(_argv[i],"%dx%d", &numOfTileCols, & numOfTileRows);

			continue;
		}
		

		// Piecefile
		if(strcmp(_argv[i], "-p") == 0)
		{
			i++;
			strcpy(pieceFile, _argv[i]);
			continue;
		}

		// output write
		if(strcmp(_argv[i], "-w") == 0)
		{
			writeOutput = true;
			continue;
		}

		// output write
		if(strcmp(_argv[i], "-mr") == 0)
		{
			tiledTiff = true;
			
			++i;
			sscanf(_argv[i],"%dx%d", &tiffTileW, &tiffTileH);
			continue;
		}
		
		// Delete the temp dir
		if(strcmp(_argv[i], "-d") == 0)
		{
			delTempDir = true;
			continue;
		}

		if(strcmp(_argv[i], "-c") == 0)
                {
                        continueAfterBlendFailure = true;
                        continue;
                }

	}

	// there should be an output file name at least
	if(strcmp(outFile,"") == 0 || strcmp(outFileWoPath, "") == 0)
	{
		printf("\nError: Please specify a valid output file name.");
		return false;
	}

	return true;
}


bool createNewProject()
{
//	char sysCmd[512];
	int existStatus, mkdirStatus;
	int i, j;

#ifdef WIN32
	struct _stat buf;
	existStatus = _stat(tempDir, &buf);
#else
	struct stat buf;
	existStatus = stat(tempDir, &buf);
#endif
	

	if(existStatus != 0)
	{
		// dir not found, Try to create one now
#ifdef WIN32
		mkdirStatus = _mkdir(tempDir);
#else
		mkdirStatus = mkdir(tempDir, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
		if(mkdirStatus != 0)
		{
			printf("\nWeaver: Could not create directory for workspace at : %s", tempDir);
			return false;
		}
		
		// if the directory was created, create the remap file
		//sprintf(remapFile, "%s/%s", tempDir, WS_REMAP_FILE);
		
		remapInfo.cols = numOfTileCols; 
		remapInfo.rows = numOfTileRows;
		remapInfo.tiles = (struct wStitchTile **)malloc(remapInfo.cols * sizeof(struct wStitchTile *));
		for(i = 0; i < remapInfo.cols; i++)
			remapInfo.tiles[i] = (struct wStitchTile *)malloc(remapInfo.rows * sizeof(struct wStitchTile));

		// Initialize tile info
		for(j = 0 ; j < remapInfo.rows; j++)
		{
			for (i = 0; i < remapInfo.cols; i++)
			{
				remapInfo.tiles[i][j].remappedX = 0;
				remapInfo.tiles[i][j].remappedY = 0;
				remapInfo.tiles[i][j].width = 0;
				remapInfo.tiles[i][j].height = 0;
				memset((void *)remapInfo.tiles[i][j].tileFileName, 0, WS_FILENAME_LEN);
				strcpy(remapInfo.tiles[i][j].tileFileName,"");
				strcpy(remapInfo.tiles[i][j].tempTileFileName,"");
			}
		}

		// dump the entire remapInfo structure onto disk
		syncRemapInfoToDisk();
	}
	else
	{
		// dir exists. Empty it
		printf("\nWeaver: Overwriting existing workspace at : %s", tempDir);
		if(!deleteProjectWorkspace()) return false;
		
		// call the function again, it should try and create the directory now
		return createNewProject();
	}

	return true;

} // createNewProject()



bool loadProject()
{
	int i,j;
	int readItems;
	int noOfFileOpenAttempts = 0;

	// open the remapFile
	FILE *remapFptr;

	// Dont give up if the file open failed. Sometimes the NFS sync takes while in the MPI case	
	while(1)
	{
		remapFptr = fopen(remapFile, "rb");
		++noOfFileOpenAttempts;
		if(remapFptr == NULL)
		{
			printf("\nWeaver[%d]: Could not open remapFile : %s", mpiRank, remapFile);
			if(noOfFileOpenAttempts < 5)
			{
				printf("\nWeaver[%d]: Wating for a second and trying again ...", mpiRank);
#ifdef WIN32
				Sleep(1000);
#else
				usleep(1000000);
#endif
			}
			else
				return false;
		}
		else
			break;
	} // end of while(1)

	// load the wStichRemap part first
	fread((void *)&remapInfo, sizeof(remapInfo), 1, remapFptr);
	numOfTileCols = remapInfo.cols;
	numOfTileRows = remapInfo.rows;

	remapInfo.tiles = (struct wStitchTile **)malloc(remapInfo.cols * sizeof(struct wStitchTile *));
	for(i = 0; i < remapInfo.cols; i++)
		remapInfo.tiles[i] = (struct wStitchTile *)malloc(remapInfo.rows * sizeof(struct wStitchTile));

	// Read in the tiles
	for(j = 0 ; j < remapInfo.rows; j++)
	{
		for (i = 0; i < remapInfo.cols; i++)
		{
			if(feof(remapFptr))
			{
				printf("\nWeaver: Reached end of remap file earlier than expected. Cannot continue");
				return false;
			}
			
			readItems = (int)fread((void*)&remapInfo.tiles[i][j], sizeof(struct wStitchTile), 1, remapFptr);
			if( readItems != 1)
			{
				perror("Weaver");
				printf("\nWeaver: Could not read in remap info for tile (%d,%d) Cannot continue", i, j);
				fclose(remapFptr);
				return false;
			}
		}
	}

	fclose(remapFptr);
	return true;

} // End of loadProject()


bool loadPieceFile()
{
	unsigned int cols, rows, fileCount;
	unsigned int fileNameLen = WS_FILENAME_LEN;

	// open the pieceFile
	FILE *pFptr = fopen(pieceFile, "rb");
	if(pFptr == NULL)
	{
		printf("\nWeaver: Could not open piecefile : %s", pieceFile);
		return false;
	}

	rows = remapInfo.rows;
    cols = remapInfo.cols;

	// read up the tilefiles
	fileCount = 0;
	while(!feof(pFptr))
	{
		fgetline(remapInfo.tiles[fileCount % cols][(unsigned int)floor((float)fileCount / (float)cols)].tileFileName,
				WS_FILENAME_LEN, pFptr);

		++fileCount;
		if(fileCount >= rows * cols) break;
	}

	if(fileCount != rows * cols)
	{
		printf("\nWeaver: Reached end of piecefile beore reading in the expected %d files. Check the piecefile.", rows * cols);
		return false;
	}

	return true;
} //end of loadPieceFile()


void normalizePath(char *str)
{
	int i;
	// change paths to unix style
	for (i = 0; i < (int)strlen(str); i++)
	{
		if(str[i] == '\\') str[i] = '/';
	}

	return;
}


bool syncRemapInfoToDisk()
{
	int i, j;
	FILE *remapFptr = fopen(remapFile, "wb");
	if(remapFptr == NULL)
	{
		printf("\nWeaver: Could not create workspace under : %s", tempDir);
		return false;
	}

	fwrite((void *)&remapInfo, sizeof(remapInfo), 1, remapFptr);
	for(j = 0 ; j < remapInfo.rows; j++)
	{
		for (i = 0; i < remapInfo.cols; i++)
		{
			fwrite((void*)&remapInfo.tiles[i][j], sizeof(struct wStitchTile), 1, remapFptr);
		}
	}

	fclose(remapFptr);
	return true;
}


bool deleteProjectWorkspace()
{
	char sysCmd[512];
	int existStatus;

#ifdef WIN32
	struct _stat buf;

	sprintf(sysCmd,"rmdir /S /Q \"%s\"", tempDir);
	system(sysCmd);
	Sleep(1000);
	// check if the dir disappeared
	existStatus = _stat(tempDir, &buf);
#else
	struct stat buf;

	sprintf(sysCmd, "rm -rf %s", tempDir);
	system(sysCmd);
	usleep(1000000);
	// check if the dir disappeared
	existStatus = stat(tempDir, &buf);
#endif

	// if the directory still exists, can't go ahead
	if(existStatus == 0 )
	{
		printf("\nWeaver: Workspace could not be cleared at : %s", tempDir);
		printf("\nWeaver: Please check if you have write permissions");
		return false;
	}

	return true;
}


bool fgetline(char *str, int strlen_max, FILE* fptr)
{
	int charCount;
	char c;

	charCount = 0;
	while(!feof(fptr))
	{
		fread((void*)&c, 1, 1, fptr);

		if(c == '\r') continue; // ignore windows fav end of line char

		str[charCount] = c;

		// if reached the end of line, tie off the end
		if(c == '\n') 
		{
			str[charCount] = '\0'; 
			if(strlen(str) < 3) { charCount = 0; continue; }
			else
			{
				return true; 
			}
		}

		++charCount;

		if (charCount == strlen_max)
		{
			printf("\nfgetline(): read string greater than buffer size");
			str[charCount - 1] = '\0';
			return false;
		}
	}

	return false;
}

