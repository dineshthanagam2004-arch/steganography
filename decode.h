#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char output_fname[30];
    FILE *fptr_output;

    int extn_size;
    char extn_secret_file[10];

    int secret_file_size;
} DecodeInfo;

/* Function Prototypes */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);

Status decode_secret_file_extn(DecodeInfo *decInfo);

Status decode_secret_file_size(DecodeInfo *decInfo);

Status create_output_file(DecodeInfo *decInfo);

Status decode_secret_file_data(DecodeInfo *decInfo);

char decode_byte_from_lsb(char *image_buffer);

int decode_integer_from_lsb(DecodeInfo *decInfo);

#endif