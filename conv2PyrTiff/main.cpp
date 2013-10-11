
#include <stdio.h>
#include <string.h>
#include "pyrTiffUtil.h"


// global variables
unsigned short compression = COMPRESSION_NONE;
unsigned int tileW = PTU_TIFFTILE_W, tileH = PTU_TIFFTILE_H;
char inFileName[512] = "";
char outFileName[512] = "";

//fwd decls
void printUsage();
bool parseCmdLine(int _argc, char **_argv);


int main(int argc, char ** argv)
{
        char errStr[512];
        if(argc < 3 || !parseCmdLine(argc, argv))
        {
                printUsage();
                return -1;
        }

        printf("\nconv2PyrTiff: Converting %s to pyramidal TIFF. Please Wait ...", inFileName);
        fflush(stdout);
        if(!convToPyrTiff(inFileName, outFileName, compression,
                   tileW, tileH, NULL, errStr))
        {
                printf("\nError: %s", errStr);
                return -1;
        }
        printf("Done");
        printf("\n");

        return 0;

}// end of main


void printUsage()
{
        printf("\nUsage: conv2PyrTiff <Options> Input_file Output_Tif_file");
        printf("\n\nOptions :\n========\n");
        printf("\n-c     :        Compression options - none, lzw, jpeg, deflate (Default : none)");
        printf("\n-t XxY :        Tile size in pixels. (Default 256x256)");
        printf("\n-h     :        Print this help.");

        printf("\n\n");

        return;
} // end of printUsage


bool parseCmdLine(int _argc, char **_argv)
{
        int i;
        for(i = 1; i < _argc; i++)
        {
                if(strcmp(_argv[i],"-h") == 0) return false;

                if(strcmp(_argv[i], "-c") == 0)
                {
                        ++i;
                        compression = 0;
                        if(strcmp(_argv[i],"none") == 0) compression = COMPRESSION_NONE;
                        if(strcmp(_argv[i],"jpeg") == 0) compression = COMPRESSION_JPEG;
                        if(strcmp(_argv[i],"lzw") == 0) compression = COMPRESSION_LZW;
                        if(strcmp(_argv[i],"deflate") == 0) compression = COMPRESSION_ADOBE_DEFLATE;

                        if(compression == 0)
                        {
                                printf("\nconv2PyrTiff: Compression scheme not understood");
                                return false;
                        }

                        continue;
                }

                if(strcmp(_argv[i], "-t") == 0)
                {
                        ++i;
                        sscanf(_argv[i],"%dx%d", &tileW, &tileH);

                        continue;
                }

                // If the code reached here, its probably just the input and output file
                strcpy(inFileName, _argv[i]);
                strcpy(outFileName, _argv[++i]);
        }


        return true;

} // End of parseCmd Line

