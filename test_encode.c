#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

int main(int argc, char *argv[])
{
    if(check_operation_type(argv) == e_encode)
    {
        printf("User selected encoding\n");
        EncodeInfo stignography;
        if(read_and_validate_encode_args(argv, &stignography) == e_success)
        {
            printf("Successfully validate the encode arguments\n");
            printf("--------------Started Encoding------------------\n");
            if(do_encoding(&stignography) == e_success)
            {
                printf("Successfully completed the encoding\n");

            }
            else
            {
                printf("Failed to perform Encoding\n ");
                return -1;
            }
        }
        else
        {
           printf("Failed to validate the encode arguments\n");
           return -1;
        }
        
    }
    else if(check_operation_type(argv) == e_decode)
{
    printf("User selected decoding\n");

    DecodeInfo decInfo;

    if(read_and_validate_decode_args(argv, &decInfo) == e_success)
    {
        printf("Successfully validated the decode arguments\n");

        if(do_decoding(&decInfo) == e_success)
        {
            printf("Successfully completed decoding\n");
        }
        else
        {
            printf("Failed to perform decoding\n");
        }
    }
    else
    {
        printf("Failed to validate decode arguments\n");
    }
}
    else
    {
        printf("Invalid argument\n Please pass : \n");
        printf("For encoding: ./a.out -e beutiful.bmp secret.txt [stego.bmp]\n");
        printf("For decoding: ./a.out -d stego.bmp [output.txt]\n");

    }

    return 0;
}

OperationType check_operation_type(char *argv[])
{

    if(strcmp(argv[1], "-e") == 0)
    return e_encode;

    else if(strcmp(argv[1], "-d") == 0)
    return e_decode;

    else
    return e_unsupported;


}