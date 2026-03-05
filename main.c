#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

// Function declaration
OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    // Step 1: Check minimum arguments
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("For encoding: ./a.out -e <.bmp> <secret_file> [output_file.bmp]\n");
        printf("For decoding: ./a.out -d <stego_image.bmp>\n");
        return 1;
    }

    // Step 2: Determine operation type
    OperationType op_type = check_operation_type(argv[1]);

    if (op_type == e_encode)
    {
        printf("===========================================\n");
        printf("       🔐 Steganography Encoding\n");
        printf("===========================================\n\n");

        EncodeInfo encInfo;

        // Validate encode arguments
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            // Perform encoding
            if (do_encoding(&encInfo) == e_success)
            {
                printf("\n-------------------------------------------\n");
                printf("🎯 Encoding Completed Successfully!\n");
                printf("📦 %s secret file encoded in : %s\n", encInfo.secret_fname,encInfo.stego_image_fname);
                printf("-------------------------------------------\n");
            }
            else
            {
                printf("\n❌ Encoding process failed.\n");
            }
        }
        else
        {
            printf("❌ Validation failed. Please check input arguments.\n");
        }
    }
    else if (op_type == e_decode)
    {
        printf("===========================================\n");
        printf("        🕵️‍♂️ Steganography Decoding\n");
        printf("===========================================\n\n");

        DecodeInfo decInfo;

        // Step 1: Validate decoding arguments
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            // Step 2: Perform decoding
            if (do_decoding(&decInfo) == e_success)
            {
                printf("\n-------------------------------------------\n");
                printf("🎉 Decoding Completed Successfully!\n");
                printf("📄 Secret File Recovered: %s\n", decInfo.output_fname);
                printf("-------------------------------------------\n");
            }
            else
            {
                printf("❌ Decoding failed!\n");
            }
        }
        else
        {
            printf("❌ Invalid decoding arguments.\n");
        }
    }
    else
    {
        printf("❌ Unsupported operation type.\n");
    }

    return 0;
}

// Function Definition
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
        return e_encode;
    else if (strcmp(symbol, "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}
