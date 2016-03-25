// Stringify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s TEXT_FILENAME VARIABLE_NAME OUTPUT_FILE\n", argv[0]);
        exit(1);
    }

    const char* pFilenameIn = argv[1];
    const char* pVariableName = argv[2];
    const char *pFilenameOut = argv[3];

    FILE *fpIn = fopen(pFilenameIn, "r");
    if (!fpIn)
    {
        fprintf(stderr, "Error: Failed to open %s\n", pFilenameIn);
        exit(1);
    }

    FILE *fpOut =  fopen(pFilenameOut, "w");
    if (!fpOut)
    {
        fprintf(stderr, "Error: Failed to open %s\n", pFilenameOut);
        exit(1);
    }

    fprintf(fpOut, "static const char* %s =\n", pVariableName);

    char row[1024];
    while (fgets(row, sizeof(row), fpIn))
    {
        row[strlen(row) - 1] = 0; // remove \n

        fprintf(fpOut, "\"%s\\n\"\n", row);
    }

    fprintf(fpOut, ";\n");

    fclose(fpIn);
    fclose(fpOut);
    return 0;
}

