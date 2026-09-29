#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL)
    {
        printf("ERROR: Stego image not provided\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if (argv[3] != NULL)
        strcpy(decInfo->output_fname, argv[3]);
    else
        strcpy(decInfo->output_fname, "output");

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
   printf("---------- Decoding Started ----------\n");

    if (open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }

    printf("Stego image opened successfully\n");

    /* Skip BMP Header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Decode Magic String */
    if (decode_magic_string(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Decode Extension Size */
    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Decode Extension */
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }


      /* Decode Secret File Size */
      if (decode_secret_file_size(decInfo) == e_failure)
       {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
       }
             


        /* Create output file */
        if (create_output_file(decInfo) == e_failure)
        {
            fclose(decInfo->fptr_stego_image);
            return e_failure;
        }

        /* Decode secret file data */
        if (decode_secret_file_data(decInfo) == e_failure)
        {
            fclose(decInfo->fptr_stego_image);
            fclose(decInfo->fptr_output);
            return e_failure;
        }

        fclose(decInfo->fptr_output);
        fclose(decInfo->fptr_stego_image);

    return e_success;
}




char decode_byte_from_lsb(char *image_buffer)
{
    char ch = 0;

    for (int i = 0; i < 8; i++)
    {
        ch = ch << 1;
        ch = ch | (image_buffer[i] & 1);
    }

    return ch;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_str[3];

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        magic_str[i] = decode_byte_from_lsb(image_buffer);
    }

    magic_str[strlen(MAGIC_STRING)] = '\0';

    printf("Decoded Magic String : %s\n", magic_str);

    if (strcmp(magic_str, MAGIC_STRING) == 0)
    {
        printf("Magic String Matched\n");
        return e_success;
    }
    else
    {
        printf("Magic String Not Matched\n");
        return e_failure;
    }
}


int decode_integer_from_lsb(DecodeInfo *decInfo)
{
    int data = 0;

    char image_buffer[32];

    fread(image_buffer,32,1,decInfo->fptr_stego_image);

    for(int i = 0; i < 32; i++)
    {
        data <<= 1;
        data |= (image_buffer[i] & 1);
    }

    return data;
}


Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    decInfo->extn_size = decode_integer_from_lsb(decInfo);

    printf("Extension Size : %d\n",decInfo->extn_size);

    return e_success;
}





Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];

    for(int i = 0; i < decInfo->extn_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf("Extension : %s\n", decInfo->extn_secret_file);

    return e_success;
}




Status decode_secret_file_size(DecodeInfo *decInfo)
{
    decInfo->secret_file_size = decode_integer_from_lsb(decInfo);

    printf("Secret File Size : %d\n", decInfo->secret_file_size);

    return e_success;
}


Status create_output_file(DecodeInfo *decInfo)
{
    char file_name[50];

    strcpy(file_name, decInfo->output_fname);
    strcat(file_name, decInfo->extn_secret_file);

    decInfo->fptr_output = fopen(file_name, "w");

    if (decInfo->fptr_output == NULL)
    {
        printf("ERROR: Unable to create output file\n");
        return e_failure;
    }

    printf("Output File : %s\n", file_name);

    return e_success;
}



Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        ch = decode_byte_from_lsb(image_buffer);

        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    printf("Secret file decoded successfully\n");
    printf("---------- Decoding Completed Successfully ----------\n");

    return e_success;
}