#ifndef DECODE_H
#define DECODE_H

#include "types.h" // for Status, etc.
#include <stdio.h>

// Structure to store decoding info
typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char *output_fname;
    FILE *fptr_output;

    char extn_secret_file[10];
    int extn_size;
    long size_secret_file;

} DecodeInfo;

// Function declarations
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);
Status decode_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

char decode_byte_from_lsb(char *image_buffer);
int decode_size_from_lsb(char *image_buffer);

Status do_decoding(DecodeInfo *decInfo);

#endif
