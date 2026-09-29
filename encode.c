#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
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
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
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


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strcmp( strchr( argv[2], '.'), ".bmp") == 0)
    encInfo -> src_image_fname = argv[2];
    else
    return e_failure;

    if(strcmp( strchr( argv[3], '.'), ".txt") == 0)
    encInfo -> secret_fname = argv[3];
    else
    return e_failure;

    if(argv[4] == NULL)
    encInfo -> stego_image_fname = "stego.bmp";
    else
    encInfo -> stego_image_fname = argv[4];

    return e_success;

}

Status check_capacity(EncodeInfo *encInfo)
{
    //get size od beautiful.bmp
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    //get the size of secret.txt
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    
    if(encInfo -> image_capacity > ((2+4+4+4+encInfo -> size_secret_file)*8))
    return e_success;
    
    else
    return e_failure;
}
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(str, 54, 1, fptr_src_image);
    fwrite(str, 54, 1, fptr_dest_image);
    return e_success;
}


Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
   encode_data_to_image(magic_string, 2, encInfo);
   return e_success;
}

Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo)

{
    for(int i = 0; i < size; i++)
    {
        //raed 8 bytes from beautiful.bmp
        fread(encInfo -> image_data, 8, 1, encInfo -> fptr_src_image);

        //encode 1 character in those 8 bytes
        encode_byte_to_lsb(data[i], encInfo -> image_data);
        //write these 8 bytes to stego.bmp
        fwrite(encInfo -> image_data, 8, 1, encInfo -> fptr_stego_image);
        
    }
    return e_success;
}

//------------------image buffer-------------------//


Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7 -i)) & 1);
        
    }
    return e_success;
}



Status encode_size_of_extension(int size,EncodeInfo *encInfo)
{
    encode_integer_to_lsb(size, encInfo);
    return e_success;
}



Status encode_integer_to_lsb(int size, EncodeInfo *encInfo)
{
    char str[32];
    //read 32 bytes from beautiful.bmp
    fread(str, 32, 1, encInfo -> fptr_src_image);
    //encode 1 integer in those 32 bytes
 for(int i = 0; i < 32; i++)
    {
        str[i] = (str[i] & 0xFE) | ((size >> (31 -i)) & 1);
    }
    //write these 32 bytes to stego.bmp
     fwrite(str, 32, 1, encInfo -> fptr_stego_image);
     return e_success;


}




Status encode_secret_file_extn( char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn,4, encInfo);
    return e_success;
}


Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_integer_to_lsb(file_size, encInfo);
    return e_success;
}




Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo -> size_secret_file];
    fseek(encInfo -> fptr_secret, 0, SEEK_SET);
    fread(str, 1, encInfo -> size_secret_file, encInfo -> fptr_secret);
    encode_data_to_image(str, encInfo -> size_secret_file, encInfo);
    return e_success;
}





Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    int count = 0;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
        count++;
    }

    printf("Remaining bytes copied = %d\n", count);
    return e_success;
}





Status do_encoding (EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("Opened all required files in required mode \n");
    }
    else
    {
        printf("Failed to open required files\n");
        return e_failure;
    }

    //      CHECK CAPACITY            //

    if(check_capacity(encInfo) == e_success)
    {
        printf("The file %s is suitable for encoding\n", encInfo -> src_image_fname);
    }

    else
    {
        printf("The file %s is  not suitable for encoding\n",encInfo -> src_image_fname);
        return e_failure;
    }

    // ---------------COPY SRC IMAGE  HEADER TO STEGO IMAGE HEADER -------------------//

    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
    {
       printf("Copied bmp header as it is from %s to %s\n",
       encInfo->src_image_fname,
       encInfo->stego_image_fname);

    }
    else
    {
        printf("Failed to copy bmp header as it is from %s to %s\n", encInfo -> src_image_fname, encInfo -> stego_image_fname);
        return e_failure;
    }

    //--------------------------MAGIC STRING---------------------------------//

    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("Encoded the magic string Successfully\n");
    }

    else
    {
        printf("Failed to encode magic string\n");
        return e_failure;
    }

    strcpy(encInfo -> extn_secret_file , strchr(encInfo -> secret_fname, '.'));
    if(encode_size_of_extension(4, encInfo) == e_success)
    {
        printf("Encodede the Extension file Successfully\n");
    }
    else
    {
       printf("Filed to encode the extension size\n");
       return e_failure;
    }



    if(encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
    {
        printf("Encoded secret file extn Successfully\n");
    }
    else
    {
        printf("Failed to encode secret file extn\n");
        return e_failure;
    }
    

    if(encode_secret_file_size(encInfo -> size_secret_file, encInfo) == e_success)
    {
        printf("Encoded secret file successfully\n");
    }
    else
    {
        printf("Failed to encode secret file\n");
        return e_failure;
    }


    if(encode_secret_file_data(encInfo) == e_success)
    {
        printf("Encoded secret data successfully\n");
    }
    else
    {
        printf("Failed to ecoded secret data \n");
        return e_failure;

    }


    printf("Source position = %ld\n", ftell(encInfo->fptr_src_image));
printf("Destination position = %ld\n", ftell(encInfo->fptr_stego_image));




    if(copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
    {
        printf("Copied remaining data as it is \n");

    }
    else
    {
        printf("Failed to copy remaining data\n");
        return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);





    return e_success;

}