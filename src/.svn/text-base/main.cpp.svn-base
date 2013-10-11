
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

#ifdef WIN32
	#include <stdlib.h>
	#include <direct.h>
#else
	#include <unistd.h>
	#include <stdlib.h>
#endif

#include <QtGui/QApplication>
#include <weaverprojectdialog.h>

wStitchLayout imagesLayout;
char tileFile[WS_FILENAME_LEN], tileFileWoPath[WS_FILENAME_LEN];
char outFile[WS_FILENAME_LEN], outFileWoPath[WS_FILENAME_LEN];
char tempTifTileFile[WS_FILENAME_LEN];
char remapFile[WS_FILENAME_LEN], tempDir[WS_FILENAME_LEN];
char pieceFile[WS_FILENAME_LEN];

bool delTempDir = false;
bool writeOutput = false;
bool newProject = false;
bool tiledTiff = false;
bool continueDespiteFailure = true;

int numOfTileCols = 0, numOfTileRows = 0;
int tileLocX = -1, tileLocY = -1;
int tiffTileW = WS_TIFFTILE_W, tiffTileH = WS_TIFFTILE_H;

struct wStichRemap remapInfo;

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
	int matchX, matchY;
        IplImage *tempCvImg;
        bool firstTileOfRow;

	printf("\nWeaver : Mosaicing tool for large EM microcopy image datasets");
	printf("\n===============================================================");
	printf("\nCopyright: National Center for Microscopy and Imaging Research, 2010");
	printf("\nAuthor: Raj Singh (rsingh@ncmir.ucsd.edu)");
	printf("\n\n");


        if (argc < 2)
	{
		printUsage();
		return -1;
	}

	// Init variables
	strcpy(tileFile, "");
	strcpy(tempDir, ".");
	strcpy(outFile,"");
	strcpy(outFileWoPath,"");
	sprintf(remapFile, "./%s", WS_REMAP_FILE);
        QApplication *qtApp = new QApplication(argc, argv);

	// Parse command line parameters
	if(!parseCmdLine(argc, argv))
	{
		printf("\nWeaver: Error parsing command line. Please check parameters.");
		return -1;
	}

	// normalize paths
	normalizePath(tileFile);
	normalizePath(outFile);
	normalizePath(tempDir);

	double clock = (double)cvGetTickCount();

	// For a new project if the workspace directory does not exist, create it
	if(newProject)
	{
		printf("\nWeaver: Creating new project workspace for a montage of %dx%d tiles", 
				numOfTileCols, numOfTileRows);
		if(!createNewProject())
		{
			printf("\nWeaver: Error creating new project workspace");
			return -1;
		}
	}
	else
	{
		// If its not a new project, load up the info in the temp directory
		printf("\nWeaver: Loading project from %s ... ", remapFile);
		if(!loadProject())
		{
			printf("\nWeaver: Error loading project workspace from %s", tempDir);
			return -1;
		}

		printf("Done. Montage size = %dx%d tiles", numOfTileCols, numOfTileRows);
	}

	// If a pieceFile was specified, load it up
	if(strcmp(pieceFile, "") != 0)
	{
		if(!loadPieceFile())
		{
			printf("\nWeaver: Error loading piecefile from %s", pieceFile);
			return -1;
		}
		else
			printf("\nWeaver: Loaded piecefile ( %s ) with %d cols and %d rows", 
					pieceFile, remapInfo.cols, remapInfo.rows);
	}

        wStitch *weaver = new wStitch();

	// **********************     SINGLE TILE CASE    *****************************
	// If its a single tile file, we treat it wrt to to previously computed tiles
	if(strcmp(tileFile, "") != 0)
	{
                printf("\nWeaver : Stitching : %s", tileFile);
		fflush(stdout);

		if (tileLocX < 0 || tileLocY < 0 || 
			tileLocX >= numOfTileCols || tileLocY >= numOfTileRows)
		{
			printf("\nWeaver: Tile location (%dx%d) is not within the bounds of this project. Quitting.", 
				tileLocX, tileLocY);
			return -1;
		}
		
		strcpy(remapInfo.tiles[tileLocX][tileLocY].tileFileName, tileFile);
		// We want the output blended images in a temp dir. So find out whats the path
		// for the temp .tif version of the tileFile
		weaver->convertToTempTifFilePath(tileFile, tempDir, tempTifTileFile);

		// For the special case of tile 0,0 no processing is done .. just a copy over to temp dir
		if(tileLocX == 0 && tileLocY == 0)
		{
			tempCvImg = weaver->loadImage(tileFile);
			if(tempCvImg == NULL)
			{
				printf("\nWeaver: Check file : %s", tileFile);
				return -1;
			}

			remapInfo.tiles[tileLocX][tileLocY].remappedX = 0;
			remapInfo.tiles[tileLocX][tileLocY].remappedY = 0;
			remapInfo.tiles[tileLocX][tileLocY].width = tempCvImg->width;
			remapInfo.tiles[tileLocX][tileLocY].height = tempCvImg->height;

			// save to temp tif file
			if(!weaver->saveImage(tempCvImg, tempTifTileFile))
			{
				printf("\nWeaver: Could not create temp file: ", tempTifTileFile);
				fflush(stdout);
				return -1;
			}

			cvReleaseImage(&tempCvImg);

			strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
			
		}

		// Figure out the tile to the left and blend with it. Then figure out the tile on top 
		// and blend with it. Take care of edge cases.
		matchX = matchY = 0;
		if(tileLocX >= 1)
                {
			if(strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName, "") == 0 ||
				strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName, "") == 0)
			{
				printf("\nWeaver: Left tile is missing. Check if processing for left tile failed. Cannot go ahead.");
				fflush(stdout);
				return -1;
			}
			else
			{
                                if(tileLocX == 0) firstTileOfRow = true;
                                else
                                    firstTileOfRow = false;
				// When the blend function successfully finishes, it has written
				// the output temp images to the temp directory
                                if(! weaver->stitch(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName,
                                                    remapInfo.tiles[tileLocX][tileLocY].tileFileName,
                                                    remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName,
                                                    remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
                                                    _WSM_HORIZONTAL, matchX, matchY,
                                                    remapInfo.tiles[tileLocX][tileLocY].width,
                                                    remapInfo.tiles[tileLocX][tileLocY].height,
                                                    tempDir, continueDespiteFailure,
                                                    firstTileOfRow))
				{
                                        printf("\nWeaver: Error stitching images %s and %s.",
						remapInfo.tiles[tileLocX][tileLocY].tileFileName,
						remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName);
                                        return -1;
				}

                                // Update the remapInfo structure with the matched coordinates
                                remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                        remapInfo.tiles[tileLocX - 1][tileLocY].remappedX + matchX;

                                remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                        remapInfo.tiles[tileLocX - 1][tileLocY].remappedY + matchY;

				// update the remap info with tempfile that was created for this tile
				strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);

			}
		}

		
		matchX = matchY = 0;
		if(tileLocY >= 1)
		{
			// Top tile blend
			if(strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName, "") == 0 || 
				strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName, "") == 0)
			{
				printf("\nWeaver: Top tile is missing. Check if processing for top tile failed. Cannot continue.");
				fflush(stdout);
				return -1;
			}
			else
			{
                                if(tileLocX == 0) firstTileOfRow = true;
                                else
                                    firstTileOfRow = false;
				// When the blend function successfully finishes, it has written
				// the output temp images to the temp directory
                                if(! weaver->stitch(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName,
                                                    remapInfo.tiles[tileLocX][tileLocY].tileFileName,
                                                    remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName,
                                                    remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
                                                    _WSM_VERTICAL, matchX, matchY,
                                                    remapInfo.tiles[tileLocX][tileLocY].width,
                                                    remapInfo.tiles[tileLocX][tileLocY].height,
                                                    tempDir, continueDespiteFailure,
                                                    firstTileOfRow))
				{
                                        printf("\nWeaver: Error stitching images %s and %s.",
						remapInfo.tiles[tileLocX][tileLocY].tileFileName,
						remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName);
                                        return -1;
				}

                                // for a successful top tile blend, results are noted only for the first tile of a new row
                                if(firstTileOfRow)
                                {
                                        remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedX + matchX;

                                        remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedY + matchY;
                                }


				// update the remap info with tempfile that was created for this tile
				strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
			}

		} // if (tileLocY >= 1)
		
	}


	// *************************     PIECE FILE CASE    *********************************
	// If a piecefile was specified and loaded we'll go through all the tile files and compute
	if(strcmp(pieceFile, "") != 0)
	{
		for(tileLocY = 0; tileLocY < remapInfo.rows; tileLocY++)
		{
			for(tileLocX = 0; tileLocX < remapInfo.cols; tileLocX++)
			{
                                printf("\nWeaver : Stitching for : %s", remapInfo.tiles[tileLocX][tileLocY].tileFileName);
				fflush(stdout);

				if (tileLocX >= numOfTileCols || tileLocY >= numOfTileRows)
				{
					printf("\nWeaver: Tile location (%dx%d) is not within the bounds of this project. Quitting.", 
						tileLocX, tileLocY);
					return -1;
				}
				
				strcpy(tileFile, remapInfo.tiles[tileLocX][tileLocY].tileFileName);
				// We want the output blended images in a temp dir. So find out whats the path
				// for the temp .tif version of the tileFile
				weaver->convertToTempTifFilePath(tileFile, tempDir, tempTifTileFile);

				// For the special case of tile 0,0 no processing is done .. just a copy over to temp dir
				if(tileLocX == 0 && tileLocY == 0)
				{
					tempCvImg = weaver->loadImage(tileFile);
					if(tempCvImg == NULL)
					{
						printf("\nWeaver: Check file : %s", tileFile);
						return -1;
					}

					remapInfo.tiles[tileLocX][tileLocY].remappedX = 0;
					remapInfo.tiles[tileLocX][tileLocY].remappedY = 0;
					remapInfo.tiles[tileLocX][tileLocY].width = tempCvImg->width;
					remapInfo.tiles[tileLocX][tileLocY].height = tempCvImg->height;

					// save to temp tif file
					if(!weaver->saveImage(tempCvImg, tempTifTileFile))
					{
						printf("\nWeaver: Could not create temp file: ", tempTifTileFile);
						fflush(stdout);
						return -1;
					}
			
					cvReleaseImage(&tempCvImg);

					strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
					
				}

				// Figure out the tile to the left and blend with it. Then figure out the tile on top 
				// and blend with it. Take care of edge cases.
				matchX = matchY = 0;
				if(tileLocX >= 1)
                                {
					if(strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName, "") == 0 ||
						strcmp(remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName, "") == 0)
					{
						printf("\nWeaver: Left tile is missing. Check if processing for left tile failed. Cannot go ahead.");
						fflush(stdout);
						return -1;
					}
					else
					{
                                                if(tileLocX == 0) firstTileOfRow = true;
                                                else
                                                    firstTileOfRow = false;
						// When the blend function successfully finishes, it has written
						// the output temp images to the temp directory
                                                if(! weaver->stitch(remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName,
										remapInfo.tiles[tileLocX][tileLocY].tileFileName,
										remapInfo.tiles[tileLocX - 1][tileLocY].tempTileFileName,
										remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
										_WSM_HORIZONTAL, matchX, matchY,
										remapInfo.tiles[tileLocX][tileLocY].width, 
										remapInfo.tiles[tileLocX][tileLocY].height,
                                                                                tempDir, continueDespiteFailure,
                                                                                firstTileOfRow))
						{
                                                        printf("\nWeaver: Error stitching images %s and %s.",
								remapInfo.tiles[tileLocX][tileLocY].tileFileName,
								remapInfo.tiles[tileLocX - 1][tileLocY].tileFileName);
                                                        return -1;
						}

                                                // Update the remapInfo structure with the matched coordinates
                                                remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                remapInfo.tiles[tileLocX - 1][tileLocY].remappedX + matchX;

                                                remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                remapInfo.tiles[tileLocX - 1][tileLocY].remappedY + matchY;

						// update the remap info with tempfile that was created for this tile
						strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);

					}
				}
				
				matchX = matchY = 0;
				if(tileLocY >= 1)
				{
					// Top tile blend
					if(strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName, "") == 0 || 
						strcmp(remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName, "") == 0)
					{
						printf("\nWeaver: Top tile is missing. Check if processing for top tile failed. Cannot continue.");
						fflush(stdout);
						return -1;
					}
					else
					{
                                                if(tileLocX == 0) firstTileOfRow = true;
                                                else
                                                    firstTileOfRow = false;
						// When the blend function successfully finishes, it has written
						// the output temp images to the temp directory
                                                if(! weaver->stitch(remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName,
										remapInfo.tiles[tileLocX][tileLocY].tileFileName,
										remapInfo.tiles[tileLocX][tileLocY - 1].tempTileFileName,
										remapInfo.tiles[tileLocX][tileLocY].tempTileFileName,
										_WSM_VERTICAL, matchX, matchY,
										remapInfo.tiles[tileLocX][tileLocY].width, 
										remapInfo.tiles[tileLocX][tileLocY].height,
                                                                                tempDir, continueDespiteFailure,
                                                                                firstTileOfRow))
						{
                                                        printf("\nWeaver: Error stitching images %s and %s.",
								remapInfo.tiles[tileLocX][tileLocY].tileFileName,
								remapInfo.tiles[tileLocX][tileLocY - 1].tileFileName);
                                                        return -1;
						}


                                                // Update the remapInfo structure if its the first tile of a new row.
                                                // Assuming it was only blended with its top tile
                                                if(firstTileOfRow)
                                                {
                                                        remapInfo.tiles[tileLocX][tileLocY].remappedX =
                                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedX + matchX;

                                                        remapInfo.tiles[tileLocX][tileLocY].remappedY =
                                                                remapInfo.tiles[tileLocX][tileLocY - 1].remappedY + matchY;

                                                        // update the remap info with tempfile that was created for this tile
                                                        strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
                                                }

						// update the remap info with tempfile that was created for this tile
						strcpy(remapInfo.tiles[tileLocX][tileLocY].tempTileFileName, tempTifTileFile);
					}

				} // if (tileLocY >= 1)

				
				// write remap info to disk for now
				syncRemapInfoToDisk();

			} // for (tileLocX ..
		} // for (tileLocY ..
	} // if(strcmp(pieceFile, "") != 0)


	// If the blends were successful, sync the remapInfo structure to disk.
	syncRemapInfoToDisk();

	// Check if user wants to write the tiles to the final output image file
	bool writeOutFileErr = false;
	if(writeOutput)
	{
		if(tiledTiff)
		{
			if(!weaver->writeMontageToTiledTiffFile(&remapInfo, tempDir, outFile, tiffTileW, tiffTileH))
			{
				printf("\nWeaver: Could not write output to %s", outFile);
				writeOutFileErr = true;
			}
		}
		else
		{
			if(!weaver->writeMontageToFile(&remapInfo, outFile))
			{
				printf("\nWeaver: Could not write output to %s", outFile);
				writeOutFileErr = true;
			}
		}
	}

	// Did the user want to delete the temp dir
	if(!writeOutFileErr && delTempDir) deleteProjectWorkspace();

	clock = (double)cvGetTickCount() - clock;
	printf( "\nWeaver: Exec time = %gms\n", clock/(cvGetTickFrequency()*1000.) );

	delete weaver;
	return 0;

} // End of main


void printUsage()
{
	printf("\nUsage: weaver -o <Output Tiff File> -m <Temp dir (optional)> <Commands>");
        printf("\nUsage: weaver -gui");
	printf("\n\nCommands:\n--------\n");
	printf("\n -n <MxN>: Start a new project which will stitch M columns and N rows of tiles");
	printf("\n -p <filename>: Piece-file containing row-wise list of tiles.");
	printf("\n -t <filename> -l <XxY> : New tile to be added to the project at column X and row Y.");
	printf("\n		");
	printf("\n -m <Temporary dir>: Temporary working directory. Default is data directory");
	printf("\n -w	: Write output to disk without deleting temporary files.");
	printf("\n -mr XxY : Write output as multires tiled TIFF with tile size XxY.");
	printf("\n -d	: Delete temporary files for the project");
        printf("\n -s	: Stop after failure to align a tile. By default the program will prompt for manual alignment on failure");
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

                if(strcmp(_argv[i],"-gui") == 0)
                {
                    weaverProjectDialog *projectDialog = new weaverProjectDialog(0);
                    projectDialog->show();
                    while(projectDialog->isVisible()) QApplication::processEvents();

                    // copy over the user defined fields and return
                    projectDialog->getOutImgFile(outFile);

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

                    // get the temp directory
                    projectDialog->getTempDir(tempDir);
                    if(strcmp(tempDir, "") != 0)
                    {
                        strcat(tempDir,"/");
                        strcat(tempDir, outFileWoPath);
                        strcat(tempDir, ".weaver");
                        sprintf(remapFile, "%s/%s", tempDir, WS_REMAP_FILE);
                    }

                    // Its always a new project with the gui
                    newProject = true;
                    numOfTileCols = projectDialog->getMontageNumCols();
                    numOfTileRows = projectDialog->getMontageNumRows();

                    // get the piecefile
                    projectDialog->getPiecelistFile(pieceFile);

                    // some defaults with the Gui
                    tiledTiff = projectDialog->getMultiResTiffFlag();
                    writeOutput = true;
                    delTempDir = true;

                    continueDespiteFailure = projectDialog->getManualAlignFlag();

                    delete projectDialog;
                    return true;
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
		
		// New input tile 
		if(strcmp(_argv[i], "-t") == 0)
		{
			i++;
			strcpy(tileFile, _argv[i]);

			// Figure out the tile file name without the path. Need it later
			for (count = (int)strlen(tileFile) - 1; count >=0; count --)
			{
				if(tileFile[count] == '/' || tileFile[count] == '\\') break;
			}
			strcpy(tileFileWoPath, tileFile + count + 1);

			i++;
			if(i >= _argc)
			{
				printf("\nError: Expected location of tile in montage not passed");
				return false;
			}

			if(strcmp(_argv[i], "-l") != 0)
			{
				printf("\nError: Expected location of tile in montage not passed");
				return false;
			}
			i++;
			sscanf(_argv[i],"%dx%d", &tileLocX, &tileLocY);

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

                if(strcmp(_argv[i], "-s") == 0)
		{
                        continueDespiteFailure = false;
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

	// open the remapFile
	FILE *remapFptr = fopen(remapFile, "rb");
	if(remapFptr == NULL)
	{
		printf("\nWeaver: Could not open remapFile : %s", remapFile);
		return false;
	}

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

		// Skip line if it was empty
		if(strcmp(remapInfo.tiles[fileCount % cols][(unsigned int)floor((float)fileCount / (float)cols)].tileFileName, " ")
				== 0 ||
			remapInfo.tiles[fileCount % cols][(unsigned int)floor((float)fileCount / (float)cols)].tileFileName[0] == '\0')
		continue;

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

