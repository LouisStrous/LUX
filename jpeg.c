/* ANA support of the Independent JPEG Group's JPEG */
/* compression/decompression software, version 6b. */

#include <stdio.h>		/* for FILE, fopen(), fclose(), printf() */
#include <setjmp.h>		/* for setjmp(), longjmp() */
#include <ctype.h>		/* for isprint() */
#include <jpeglib.h>		/* for IJG JPEG v6b stuff */
#include <string.h>		/* for memcpy() */
#include "action.h"		/* for ANA-specific stuff */

/* a structure for our own error handler */
struct my_error_mgr {
  struct jpeg_error_mgr	pub;
  jmp_buf	setjmp_buffer;
};
typedef struct my_error_mgr *my_error_ptr;

/*--------------------------------------------------------------------------*/
METHODDEF(void) my_error_exit(j_common_ptr cinfo)
/* our own error handler; following the example in example.c in the IJG */
/* JPEG v6b source */
{
  my_error_ptr	myerr = (my_error_ptr) cinfo->err;

  /* display the message */
  (*cinfo->err->output_message)(cinfo);

  /* return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}
/*--------------------------------------------------------------------------*/
int read_jpeg6b(int narg, int ps[], int isFunc)
/* JREAD,<x>,<file>[,<header>,SHRINK=<shrink>][,/GREYSCALE] */
{
  char	*filename, *p;
  struct jpeg_decompress_struct	cinfo;
  struct my_error_mgr		jerr;
  FILE	*infile;
  int	result, dims[3], i, stride, n;
  JSAMPROW	row_pointer[1];	/* pointer to a single row */
  JSAMPLE	*image;

  if (!symbolIsString(ps[1]))
    return isFunc? ANA_ERROR: cerror(NEED_STR, ps[1]);
  filename = string_value(ps[1]);
  if (!(infile = fopen(filename, "rb")))
    return isFunc? ANA_ERROR: cerror(ERR_OPEN, ps[1]);

  /* 1. initialize */
  cinfo.err = jpeg_std_error(&jerr.pub);/* default error handler */
  jerr.pub.error_exit = my_error_exit; /* override error exit behavior */
  if (setjmp(jerr.setjmp_buffer)) {
    /* if we get here, then the JPEG code found a fatal error */
    /* an appropriate message was displayed by the error handler */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return ANA_ERROR;
  }

  jpeg_create_decompress(&cinfo); /* decompression structure */

  /* 2. specify data source */
  jpeg_stdio_src(&cinfo, infile);

  /* 3. get image info */
  jpeg_save_markers(&cinfo, JPEG_COM, 0xffff);/* we look for a comment */
  jpeg_read_header(&cinfo, TRUE);

  if (internalMode & 1)		/* user wants greyscale output */
    cinfo.out_color_space = JCS_GRAYSCALE;
  if (narg > 3 && ps[3]) {	/* have <shrink> */
    /* the user wants to shrink the image.  shrink factors 1, 2, 4, and 8 */
    /* are allowed; if the user supplies a different value, then the next */
    /* smaller allowed value is used.  The sign of the shrink factor is */
    /* ignored. */
    i = int_arg(ps[3]);
    if (i < 0)
      i = -i;
    if (i >= 8)
      i = 8;
    else if (i >= 4)
      i = 4;
    else if (i >= 2)
      i = 2;
    else
      i = 1;
    cinfo.scale_num = 1;
    cinfo.scale_denom = i;
  }

  /* 4. start decompression */
  jpeg_start_decompress(&cinfo);

  /* 5. generate output symbol */
  i = 0;
  if (cinfo.output_components > 1)
    dims[i++] = cinfo.output_components;
  dims[i++] = cinfo.output_width;
  dims[i++] = cinfo.output_height;
  if (to_scratch_array(*ps, ANA_BYTE, i, dims) == ANA_ERROR)
    goto read_jpeg6b_1;
  image = (JSAMPLE *) array_data(*ps);
  stride = cinfo.output_width*cinfo.output_components;

  /* 6. read data */
  while (cinfo.output_scanline < cinfo.output_height) {
    row_pointer[0] = image + (cinfo.output_height - cinfo.output_scanline - 1)*stride;
    jpeg_read_scanlines(&cinfo, row_pointer, 1);
  }

  /* treat the comment */
  if (cinfo.marker_list) {
    /* it's possible that the JPEG file creator put non-printable */
    /* garbage in the comment, even though that is bad form. */
    /* We replace non-printable characters by asterisks. */
    n = cinfo.marker_list->data_length;
    p = (char *) cinfo.marker_list->data;
    while (n--) {
      if (!isprint(*p))
	*p = '*';
      p++;
    }
    if (narg > 2 && ps[2]) {	/* user wants comment in variable */
      if (symbolIsNamed(ps[2])) {
	n = cinfo.marker_list->data_length;
	redef_string(ps[2], n);
	memcpy(string_value(ps[2]), cinfo.marker_list->data, n);
	string_value(ps[2])[n] = '\0'; /* terminate properly */
      } else if (!isFunc)
	cerror(NEED_NAMED, ps[2]);
    } else {			/* just print it */
      p = (char *) cinfo.marker_list->data;
      n = cinfo.marker_list->data_length;
      while (n--)
	putchar(*p++);
      putchar('\n');
    }
  } else if (narg > 2 && ps[2]) {
    if (symbolIsNamed(ps[2])) {
      redef_string(ps[2], 0);
      string_value(ps[2])[0] = '\0';
    } else if (!isFunc)
      cerror(NEED_NAMED, ps[2]);
  }

  /* 7. finish up nicely */
  jpeg_finish_decompress(&cinfo);

  read_jpeg6b_1:
  fclose(infile);
  jpeg_destroy_decompress(&cinfo);

  return ANA_OK;
}
/*--------------------------------------------------------------------------*/
int ana_read_jpeg6b(int narg, int ps[])
{
  return read_jpeg6b(narg, ps, 0);
}
/*--------------------------------------------------------------------------*/
int ana_read_jpeg6b_f(int narg, int ps[])
{
  return (read_jpeg6b(narg, ps, 1) == ANA_OK)? ANA_ONE: ANA_ZERO;
}
/*--------------------------------------------------------------------------*/
int write_jpeg6b(int narg, int ps[], int isFunc)
/* JWRITE,<x>,<file>[,<header>,<quality>] */
{
  int	nx, ny, i, nd, quality, stride;
  struct jpeg_compress_struct	cinfo;
  struct my_error_mgr		jerr;
  FILE	*outfile;
  char	*header;
  JSAMPROW	row_pointer[1];
  JSAMPLE	*image;

  /* get image info */
  if (!symbolIsNumericalArray(ps[0]))
    return isFunc? ANA_ERROR: cerror(NEED_NUM_ARR, ps[0]);
  nd = array_num_dims(ps[0]);
  if (nd < 2 || nd > 3)
    return isFunc? ANA_ERROR: cerror(ILL_NUM_DIM, ps[0]);
  if (nd == 3 && array_dims(ps[0])[0] != 3)
    return isFunc? ANA_ERROR: anaerror("When saving an RGB image, the first dimension must have 3 elements", ps[0]);
  if (array_type(ps[0]) != ANA_BYTE)
    return isFunc? ANA_ERROR: anaerror("Only JPEG compression of BYTE arrays is supported", ps[0]);
  nx = array_dims(ps[0])[0 + (nd == 3)];
  ny = array_dims(ps[0])[1 + (nd == 3)];

  if (narg > 2 && ps[2]) {	/* have a <header> */
    if (!symbolIsString(ps[2]))
      return isFunc? ANA_ERROR: cerror(NEED_STR, ps[2]);
    header = string_value(ps[2]);
  } else
    header = NULL;

  if (!symbolIsString(ps[1]))
    return isFunc? ANA_ERROR: cerror(NEED_STR, ps[1]);
  outfile = fopen(string_value(ps[1]), "wb");
  if (!outfile)
    return isFunc? ANA_ERROR: cerror(ERR_OPEN, ps[1]);

  /* 1. initialize */
  cinfo.err = jpeg_std_error(&jerr.pub); /* default error handler */
  jerr.pub.error_exit = my_error_exit; /* override error exit behavior */
  if (setjmp(jerr.setjmp_buffer)) {
    /* if we get here, then the JPEG code found a fatal error */
    /* an appropriate message was displayed by the error handler */
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    return ANA_ERROR;
  }
  
  jpeg_create_compress(&cinfo);	/* compression structure */

  /* 2. specify data target */
  jpeg_stdio_dest(&cinfo, outfile);

  /* 3. specify image characteristics */
  cinfo.image_width = nx;
  cinfo.image_height = ny;
  if (nd == 3) {		/* RGB image */
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
  } else {			/* greyscale image */
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
  }

  /* 4. select compression parameters */
  jpeg_set_defaults(&cinfo);

  if (narg > 3 && ps[3]) {	/* have <quality> */
    quality = int_arg(ps[3]);
    if (quality < 0)
      quality = 0;
    if (quality > 100)
      quality = 100;
  } else
    quality = 75;

  jpeg_set_quality(&cinfo, quality, TRUE);

  /* 5. start compressing */
  jpeg_start_compress(&cinfo, TRUE);

  if (header)
    jpeg_write_marker(&cinfo, JPEG_COM, (void *) header, string_size(ps[2]));

  /* 6. compress each scanline */
  image = (JSAMPLE *) array_data(*ps);
  stride = cinfo.image_width*cinfo.input_components;
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = image + (cinfo.image_height - cinfo.next_scanline - 1)*stride;
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* 7. finish up nicely */
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(outfile);

  return ANA_OK;
}
/*--------------------------------------------------------------------------*/
int ana_write_jpeg6b(int narg, int ps[])
{
  return write_jpeg6b(narg, ps, 0);
}
/*--------------------------------------------------------------------------*/
int ana_write_jpeg6b_f(int narg, int ps[])
{
  return (write_jpeg6b(narg, ps, 1) == ANA_OK)? ANA_ONE: ANA_ZERO;
}
/*--------------------------------------------------------------------------*/
