#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <unistd.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Step 1: Source Image File
    char* dot = strstr(argv[2],".");
    if(argv[2][0] != '.' && dot != NULL && strcmp(dot, ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("❌ Error: Source file should have .bmp extension and valid name\n");
        return e_failure;
    }
    // Step 2: Secret File
    if (argv[3][0] != '.')
    {
        dot = strstr(argv[3], ".");
        if (dot && (strcmp(dot, ".txt") == 0 || strcmp(dot, ".c") == 0 || 
                    strcmp(dot, ".h") == 0 || strcmp(dot, ".sh") == 0))
        {
            encInfo->secret_fname = argv[3];
            strcpy(encInfo->extn_secret_file, dot);
            }
        else
        {
            printf("❌ Error: Secret file extension should be .txt, .c, .h or .sh\n");
            return e_failure;
        }
    }
    else
    {
        printf("❌ Error: Content file name should not start with '.'\n");
        return e_failure;
    }



    // Step 3: Stego Image File
     if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "default.bmp";
    }
    else
    {
        if (argv[4][0] != '.')
        {
            dot = strstr(argv[4], ".");
            if (dot && strcmp(dot, ".bmp") == 0)
            {
                encInfo->stego_image_fname = argv[4];
            }
            else
            {
                printf("❌ Error: Output file should have .bmp extension\n");
                return e_failure;
            }
        }
        else
        {
            printf("❌ Error: Output file name should not start with '.'\n");
            return e_failure;
        }
    }

    return e_success;

}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
     encInfo -> image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    // char *extn = strstr(encInfo -> secret_fname, ".txt");
    int ext_size = strlen(encInfo->extn_secret_file);
    int total_bytes = 54 + (strlen(MAGIC_STRING) * 8) + 32 + (ext_size * 8) + 32 + (encInfo ->size_secret_file * 8);
    if(encInfo->image_capacity>total_bytes)
        return e_success;
    else
        return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char ptr[54]; 
	fseek(fptr_src_image,0,SEEK_SET);
	fread(ptr,54,1,fptr_src_image);   // read 54 bytes from source image
	fwrite(ptr,54,1,fptr_dest_image);  // write 54 bytes in the destination file
	return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for(int i=0; i<strlen(magic_string); i++)
    {
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i],buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    return e_success;
    else
    return e_failure;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    return e_success;
    else
    return e_failure;

}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for(int i=0; i<strlen(file_extn); i++)
    {
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image); 
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    return e_success;
    else
    return e_failure;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
     char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    return e_success;
    else
    return e_failure;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer1[encInfo->size_secret_file];

rewind(encInfo->fptr_secret);
fread(buffer1, encInfo->size_secret_file, 1, encInfo->fptr_secret);

char buffer[8];
for (int i = 0; i < encInfo->size_secret_file; i++)
{
    fread(buffer, 8, 1, encInfo->fptr_src_image);
    encode_byte_to_lsb(buffer1[i], buffer);
    fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
}

if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    return e_success;
else
    return e_failure;

}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
  char buffer[1];
    while(fread(buffer,1,1,fptr_src) == 1)
    {
        // fread(buffer,1,1,fptr_src);
        fwrite(buffer,1,1,fptr_dest);
    }
    if(ftell(fptr_src) == ftell(fptr_dest))
        return e_success;
    return e_failure;   
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0; i<8; i++)
    {
        image_buffer[i] = ((image_buffer[i] & ~1) | ((data >> i) & 1));
    }
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
     for(int i=0; i<32; i++)
    {
        imageBuffer[i] = ((imageBuffer[i] & ~1) | ((size >> i) & 1));
    }
}

Status do_encoding(EncodeInfo *encInfo)
{
        // call open_files(encInfo);
if (open_files(encInfo) == e_success)
    {
        usleep(300000);
        printf("\n%-40s %2s", "-> Files opened", "✅\n");
        usleep(300000);

        if (check_capacity(encInfo) == e_success)
        {
            printf("%-40s %2s", "-> Image capacity verified", "✅\n");
            usleep(300000);

            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("%-40s %2s", "-> BMP header copied", "✅\n");
                usleep(300000);

                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("%-40s %2s", "-> Magic string encoded", "✅\n");
                    usleep(300000);

                    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
                    {
                        printf("%-40s %2s", "-> Secret file extn size encoded", "✅\n");
                        usleep(300000);

                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("%-40s %2s", "-> Secret file extension encoded", "✅\n");
                            usleep(300000);

                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("%-40s %2s", "-> Secret file size encoded", "✅\n");
                                usleep(300000);

                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("%-40s %2s", "-> Secret file data encoded", "✅\n");
                                    usleep(300000);

                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("%-40s %2s", "-> Remaining image data copied", "✅\n");
                                        usleep(300000);
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf(" ❌ Error: copying remaining image data failed!\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf(" ❌ Error: encoding secret file data failed!\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf(" ❌ Error: encoding secret file size failed!\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf(" ❌ Error: encoding secret file extension failed!\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf(" ❌ Error: encoding secret file extn size failed!\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf(" ❌ Error: encoding magic string failed!\n");
                    return e_failure;
                }
            }
            else
            {
                printf(" ❌ Error: copying BMP header failed!\n");
                return e_failure;
            }
        }
        else
        {
            printf(" ❌ Error: insufficient image capacity in %s!\n", encInfo->src_image_fname);
            return e_failure;
        }
    }
    else
    {
        printf(" ❌ Error: failed to open files!\n");
        return e_failure;
    }
}
