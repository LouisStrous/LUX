/* jreadw.c, interface with jpeg libraries based on example.c in JPEG
   distribution */
#include "action.h"
#include "jinclude.h"
#include <setjmp.h>

 /* parameters for the current image */
 Int	nx, ny;
 static	Int result_sym;
 static pointer q1;
 Byte	*base;
 /*------------------------------------------------------------------------- */
METHODDEF void input_init (compress_info_ptr cinfo)
/* Initialize for input; return image size and component data. */
{
  cinfo->image_width = nx;		/* width in pixels */
  cinfo->image_height = ny;		/* height in pixels */
  cinfo->input_components = 1;		/* or 1 for grayscale */
  cinfo->in_color_space = CS_GRAYSCALE;	/* or CS_GRAYSCALE for grayscale */
  cinfo->data_precision = 8;		/* bits per pixel component value */
  cinfo->optimize_coding = TRUE;
}
 /*------------------------------------------------------------------------- */
METHODDEF void get_input_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
/* Read next row of pixels into pixel_row[][] */
{
  register JSAMPROW ptr0;
  register long col;
  
  ptr0 = pixel_row[0];
  for (col = 0; col < cinfo->image_width; col++) {
    *ptr0++ = (JSAMPLE) *q1.b++; /* gray */
  }
}
 /*------------------------------------------------------------------------- */
METHODDEF void input_term (compress_info_ptr cinfo)
/* Finish up at the end of the input */
{
  /* This termination routine will very often have no work to do, */
  /* but you must provide it anyway. */
  /* Note that the JPEG code will only call it during successful exit; */
  /* if you want it called during error exit, you gotta do that yourself. */
}
 /*------------------------------------------------------------------------- */
METHODDEF void c_ui_method_selection (compress_info_ptr cinfo)
{
  /* If the input is gray scale, generate a monochrome JPEG file. */
  if (cinfo->in_color_space == CS_GRAYSCALE)
    j_monochrome_default(cinfo);
  /* For now, always select JFIF output format. */
  jselwjfif(cinfo);
}
 /*------------------------------------------------------------------------- */
Int ana_write_jpeg_f(Int narg, Int ps[])
/* a function version that returns 1 if read OK */
{
  Int	ana_write_jpeg(Int, Int []);

  if (ana_write_jpeg(narg, ps) == ANA_OK) 
    return ANA_ONE;
  else
    return ANA_ZERO;
}
 /*------------------------------------------------------------------------- */
Int ana_write_jpeg(Int narg, Int ps[])	/* jpeg write subroutine */
 /* 10/17/92, start with 2-D Byte files */
{
 struct compress_info_struct cinfo;
 struct compress_methods_struct c_methods;
 struct external_methods_struct e_methods;
 char	*name;
 Int	iq, nd, type, jqual;

 iq = ps[0];
 if (!symbolIsNumericalArray(iq))
   return cerror(NEED_NUM_ARR, iq);
 type = array_type(iq);
 q1.l = array_data(iq);
 nd = array_num_dims(iq);
 if (nd != 2 || type != ANA_BYTE)
  return anaerror("WRITE_JPEG only supports 2-D Byte arrays\n", iq);

			 /* second argument must be a string, file name */
 if (!symbolIsString(ps[1]))
   return cerror(NEED_STR, ps[1]);
 name = string_value(ps[1]);
 
 nx = array_dims(iq)[0];
 ny = array_dims(iq)[1];
 		/* optional third argument is the quality level */
 if (narg > 2)
   jqual = int_arg(ps[2]);
 else
   jqual = 75;
 /* Initialize the system-dependent method pointers. */
 cinfo.methods = &c_methods;	/* links to method structs */
 cinfo.emethods = &e_methods;
 jselerror(&e_methods);	/* select std error/trace message routines */
 jselmemmgr(&e_methods);	/* select std memory allocation routines */
 c_methods.input_init = input_init;
 c_methods.get_input_row = get_input_row;
 c_methods.input_term = input_term;
 c_methods.c_ui_method_selection = c_ui_method_selection;
 
 /* Set up default JPEG parameters in the cinfo data structure. */
 j_c_defaults(&cinfo, jqual, FALSE);
 cinfo.input_file = NULL;	/* if no actual input file involved */
 /* the b option in the fopen is ignored for the NeXT, it may be needed
    for some system and may need to be omitted in others */
 if ((cinfo.output_file = fopen(name, "wb")) == NULL) {
   fprintf(stderr, "can't open %s\n", name);
   return ANA_ERROR;
 }
 /* Here we go! */
 jpeg_compress(&cinfo);
 fclose(cinfo.output_file);
 return ANA_OK;
}
 /*------------------------------------------------------------------------- */
 /* These static variables are needed by the error routines. */
 static jmp_buf setjmp_buffer;	/* for return to caller */
 static external_methods_ptr emethods; /* needed for access to message_parm */
 /*------------------------------------------------------------------------- */
METHODDEF void trace_message (const char *msgtext)
 {
  fprintf(stderr, msgtext,
	  emethods->message_parm[0], emethods->message_parm[1],
	  emethods->message_parm[2], emethods->message_parm[3],
	  emethods->message_parm[4], emethods->message_parm[5],
	  emethods->message_parm[6], emethods->message_parm[7]);
  fprintf(stderr, "\n");	/* there is no \n in the format string! */
 }
 /*------------------------------------------------------------------------- */
METHODDEF void
 error_exit (const char *msgtext)
 {
  trace_message(msgtext);	/* report the error message */
  (*emethods->free_all) ();	/* clean up memory allocation & temp files */
  longjmp(setjmp_buffer, 1);	/* return control to outer routine */
 }
 /*------------------------------------------------------------------------- */
METHODDEF void output_init (decompress_info_ptr cinfo)
/* This routine should do any setup required */
{
 Int	dim[2], iq;

 nx = cinfo->image_width;		/* width in pixels */
 ny = cinfo->image_height;		/* height in pixels */
 dim[0] = nx;	dim[1] = ny;
 /*
 printf("jpeg file fields, nx, ny = %d %d, bits = %d\n",nx, ny,
 	cinfo->data_precision);
 printf("# of components = %d, colorspace = ", cinfo->color_out_comps);
 switch (cinfo->out_color_space) {
 default:
 case CS_UNKNOWN: printf("unknown"); break;
 case CS_GRAYSCALE: printf("grayscale"); break;
 case CS_RGB: printf("rgb"); break;
 case CS_YCbCr: printf("YCbCr"); break;
 case CS_YIQ: printf("yiq"); break;
 case CS_CMYK: printf("cmyk"); break;
 }

 */
						 /* create the output array */
 iq = result_sym;	/* result_sym was determined by ana_read_jpeg */
 if (redef_array(iq, ANA_BYTE, 2, dim) != 1) {
   ERREXIT(cinfo->emethods, "can't create result array");
 }
 q1.l = array_data(iq);
 base = q1.b;	/* for checks */
}
 /*------------------------------------------------------------------------- */
METHODDEF void
put_color_map (decompress_info_ptr cinfo, Int num_colors, JSAMPARRAY colormap)
/* Write the color map */
{
  /* You need not provide this routine if you always set cinfo->quantize_colors
   * FALSE; but a safer practice is to provide it and have it just print an
   * error message, like this:
   */
  fprintf(stderr, "put_color_map called: there's a bug here somewhere!\n");
}
 /*------------------------------------------------------------------------- */
METHODDEF void
put_pixel_rows (decompress_info_ptr cinfo, Int num_rows, JSAMPIMAGE pixel_data)
/* Write some rows of output data */
{
  /* This example shows how you might write full-color RGB data (3 components)
   * to an output file in which the data is stored 3 bytes per pixel.
   */
 register JSAMPROW ptr0;
 register long col;
 register Int row;
  
 /* printf("put_pixel_rows: num_rows, cinfo->image_width = %d, %d\n",
 	num_rows, cinfo->image_width); */
 /* printf("base, q1.b = %d, %d\n", base, q1.b); */
 if ( q1.b - base > (nx*ny) ) {
 	printf("array address problem\n");
	ERREXIT(cinfo->emethods, "array index out of range");  }
 for (row = 0; row < num_rows; row++) {
    ptr0 = pixel_data[0][row];
    for (col = 0; col < cinfo->image_width; col++) {
     *q1.b++ = *ptr0++;
    }
  }
}
 /*------------------------------------------------------------------------- */
METHODDEF void output_term (decompress_info_ptr cinfo)
 /* Finish up at the end of the output */
 {
  /* printf("output_term called, we do nothing\n"); */
 }
 /*------------------------------------------------------------------------- */
METHODDEF void d_ui_method_selection (decompress_info_ptr cinfo)
 {
 /* try forcing everybody to grayscale */
    cinfo->out_color_space = CS_GRAYSCALE;

  /* select output routines */
  cinfo->methods->output_init = output_init;
  cinfo->methods->put_color_map = put_color_map;
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
 }
 /*------------------------------------------------------------------------- */
Int ana_read_jpeg_f(Int narg, Int ps[])
/* a function version that returns 1 if read OK */
{
  Int	ana_read_jpeg(Int, Int []);

  if (ana_read_jpeg(narg, ps) == ANA_OK)
    return ANA_ONE;
  else
    return ANA_ZERO;
}
 /*------------------------------------------------------------------------- */
Int ana_read_jpeg(Int narg, Int ps[])	/* jpeg read subroutine */
 /* 10/17/92, start with 2-D Byte files */
 {
 char	*name;
 struct decompress_info_struct cinfo;
 struct decompress_methods_struct dc_methods;
 struct external_methods_struct e_methods;

	 /* first arg is the variable to load, second is name of file */
 if (!symbolIsString(ps[1]))
   return cerror(NEED_STR, ps[1]);
 name = string_value(ps[1]);
 if ((cinfo.input_file = fopen(name, "rb")) == NULL) {
   fprintf(stderr, "can't open %s\n", name);
   return ANA_ERROR;
 }
 result_sym = ps[0];	/* result, created in out */
 cinfo.output_file = NULL;	/* if no actual output file involved */
  /* Initialize the system-dependent method pointers. */
 cinfo.methods = &dc_methods;	/* links to method structs */
 cinfo.emethods = &e_methods;
  /* Here we supply our own error handler; compare to use of standard error
   * handler in the previous write_JPEG_file example.
   */
 emethods = &e_methods;	/* save struct addr for possible access */
 e_methods.error_exit = error_exit; /* supply error-exit routine */
 e_methods.trace_message = trace_message; /* supply trace-message routine */
 e_methods.trace_level = 0;

  /* prepare setjmp context for possible exit from error_exit */
 if (setjmp(setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * Memory allocation has already been cleaned up (see free_all call in
     * error_exit), but we need to close the input file before returning.
     * You might also need to close an output file, etc.
     */
   printf("error in JPEG code\n");
   fclose(cinfo.input_file);
   return ANA_ERROR;
 }
 jselmemmgr(&e_methods);	/* select std memory allocation routines */

  /* Here, set up the pointer to your own routine for post-header-reading
   * parameter selection.  You could also initialize the pointers to the
   * output data handling routines here, if they are not dependent on the
   * image type.
   */
 dc_methods.d_ui_method_selection = d_ui_method_selection;

  /* Set up default decompression parameters. */
 j_d_defaults(&cinfo, TRUE);
	/* Force grayscale output. */
 cinfo.out_color_space = CS_GRAYSCALE;
 /* Set up to read a JFIF or baseline-JPEG file. */
  /* This is the only JPEG file format currently supported. */
 jselrjfif(&cinfo);
  /* Here we go! */
 jpeg_decompress(&cinfo);
 fclose(cinfo.input_file);
 return ANA_OK;			/* indicate success */
}
