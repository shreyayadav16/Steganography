#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "decode.h"
#include "common.h"
#include "types.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *dot;

    // Step 1: Validate stego image
    dot = strstr(argv[2], ".");
    if (argv[2][0] != '.' && dot && strcmp(dot, ".bmp") == 0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("❌ Error: Stego image should have .bmp extension\n");
        return e_failure;
    }

    // Step 2: Optional output file name
    if (argv[3] == NULL)
    {
        decInfo->output_fname = "decoded_output";
    }
    else
    {
        decInfo->output_fname = argv[3];
    }

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch |= ((image_buffer[i] & 1) << i);
    }
    return ch;
}

int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        size |= ((image_buffer[i] & 1) << i);
    }
    return size;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded_magic[strlen(MAGIC_STRING) + 1];
    decoded_magic[strlen(MAGIC_STRING)] = '\0';

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // skip header

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decoded_magic[i] = decode_byte_from_lsb(buffer);
    }

    if (strcmp(decoded_magic, MAGIC_STRING) == 0)
        return e_success;
    else
        return e_failure;
}

Status decode_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->extn_size = decode_size_from_lsb(buffer);
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->size_secret_file = decode_size_from_lsb(buffer);
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    // create output file
    char output_filename[50];
    sprintf(output_filename, "%s%s", decInfo->output_fname, decInfo->extn_secret_file);
    decInfo->fptr_output = fopen(output_filename, "w");

    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to create output file\n");
        return e_failure;
    }

    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        ch = decode_byte_from_lsb(buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    fclose(decInfo->fptr_output);
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
     if (open_decode_files(decInfo) == e_success)
    {
        printf("\n%-40s ✅\n", "-> Stego image opened");
        usleep(300000);

        if (decode_magic_string(decInfo) == e_success)
        {
            printf("%-40s ✅\n", "-> Magic string verified");
            usleep(300000);

            decode_extn_size(decInfo);
            printf("%-40s ✅ (%d)\n", "-> Extension size decoded", decInfo->extn_size);
            usleep(300000);

            decode_secret_file_extn(decInfo);
            printf("%-40s ✅ (%s)\n", "-> Extension decoded", decInfo->extn_secret_file);
            usleep(300000);

            decode_secret_file_size(decInfo);
            printf("%-40s ✅ (%ld bytes)\n", "-> Secret file size decoded", decInfo->size_secret_file);
            usleep(300000);

            if (decode_secret_file_data(decInfo) == e_success)
            {
                printf("%-40s ✅\n", "-> Secret file data decoded successfully");
                usleep(300000);
                printf("\n🎉 Decoding completed successfully!\n\n");
                return e_success;
            }
            else
            {
                printf("❌ Error: Failed to decode file data\n");
                return e_failure;
            }
        }
        else
        {
            printf("❌ Error: Magic string mismatch — no hidden data found!\n");
            return e_failure;
        }
    }
    else
    {
        printf("❌ Error: Could not open stego image\n");
        return e_failure;
    }
}
