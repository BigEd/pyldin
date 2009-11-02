#include <png.h>
#include <time.h>

#ifdef _EE
static int count_pic = 0;
#endif

int savepng(unsigned short *data, int width, int height)
{
    int i;
    unsigned char bufferRow[1024*3];
    char fname[1024];
    png_structp png_ptr;
    png_infop info_ptr;

#ifdef _EE
    sprintf(fname, "sshot-%d.png", count_pic);
    count_pic++;
#else
    struct tm *dt;

    time_t t = time(NULL);
    
    dt = localtime(&t);

#ifdef _PSP
    strftime(fname, 1023, "ms0:/PICTURE/pyldin601-%d%m%y%H%M%S.png", dt);
#else
    strftime(fname, 1023, "sshot-%d%m%y%H%M%S.png", dt);
#endif
#endif
    /* Open the actual file */

    FILE * fp = fopen(fname, "wb");

    if (!fp) 
	return -1;

    /* First try to alloacte the png structures */

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,NULL, NULL);
    if (!png_ptr)
	return -1;

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr) {
	png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
	return -1;
    }
	
    /* Finalize the initing of png library */
    png_init_io(png_ptr, fp);
    png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
		
    /* set other zlib parameters */
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_strategy(png_ptr,Z_DEFAULT_STRATEGY);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);
    png_set_compression_buffer_size(png_ptr, 8192);

    png_set_bgr( png_ptr );
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_ptr, info_ptr);

#ifdef _PSP
    data += (((480 - 320) / 2) + ((272 - 240) / 2) * 512);
#endif

    for (i=0;i<height;i++) {
	void *rowPointer;
	void *srcLine;
	int x;

#ifdef _PSP
	srcLine = &data[ i * 512 ];
#else
	srcLine = &data[ i * width ];
#endif
	for (x=0; x < width; x++) {
	    unsigned int pixel = ((unsigned short *)srcLine)[x];
	    bufferRow[x*3+0] = (((pixel & 0x001f) * 0x21) >>  2);
	    bufferRow[x*3+1] = (((pixel & 0x07e0) * 0x41) >>  9);
	    bufferRow[x*3+2] = (((pixel & 0xf800) * 0x21) >>  13);
	}

	rowPointer=bufferRow;

	png_write_row(png_ptr, (png_bytep)rowPointer);
    }

    /* Finish writing */
    png_write_end(png_ptr, 0);

    /*Destroy PNG structs*/
    png_destroy_write_struct(&png_ptr, &info_ptr);

    /*close file*/
    fclose(fp);

    return 0;
}

