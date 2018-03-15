#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <vector>

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

    FILE *fpIn = fopen(pFilenameIn, "rb");
    fseek(fpIn, 0, SEEK_END);
    long fileSize = ftell(fpIn);
    fseek(fpIn, 0, SEEK_SET);
    std::vector<uint8_t> entireFile(fileSize);
    fread(entireFile.data(), 1, (size_t)fileSize, fpIn);
    entireFile.push_back(0);
    fseek(fpIn, 0, SEEK_SET);

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

    // fprintf(fpOut, "// static const char* %s =\n", pVariableName);
    // 
    // char row[1024];
    // while (fgets(row, sizeof(row), fpIn))
    // {
    //     row[strlen(row) - 1] = row[strlen(row) - 2] = 0; // remove \r\n
    //     
    //     fprintf(fpOut, "// \"%s\\n\"", row);
    // }
    // 
    // fprintf(fpOut, "// ;\n");

    fprintf(fpOut, "static const char %s[] =\n{", pVariableName);

    for (size_t i = 0; i < entireFile.size(); ++i)
    {
        fprintf(fpOut, "0x%X, ", (uint32_t)entireFile[i]);
        if (i % 16 == 15)
        {
            fprintf(fpOut, "\n");
        }
    }
    fprintf(fpOut, "};\n");

    fclose(fpIn);
    fclose(fpOut);
    return 0;
}

