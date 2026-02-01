#include "config.h"

#if HAVE_LIBOPENIMAGEIO
#include "action.hh"
#include <OpenImageIO/imageio.h>

// <x> = readimage(<file>)
int32_t
lux_read_image_oiio(ArgumentCount narg, Symbol ps[])
{
  char* filename;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, ps[0]);
  filename = string_value(ps[0]);

  auto in = OIIO::ImageInput::open(filename);
  if (!in)
    return LUX_MINUS_ONE;
  const auto& spec = in->spec();
  uint32_t result;
  auto oiiotype = spec.format;
  Symboltype luxtype;
  {
    int dims[3];
    int ndim;
    if (spec.nchannels > 1)
    {
      dims[0] = spec.nchannels;
      dims[1] = spec.width;
      dims[2] = spec.height;
      ndim = 3;
    } else {
      dims[0] = spec.width;
      dims[1] = spec.height;
      ndim = 2;
    }
    result = array_scratch(LUX_FLOAT, ndim, dims);
  }
  in->read_image(0, 0, 0, -1, OIIO::TypeDesc::FLOAT, array_data(result));
  in->close();
  return result;
}
REGISTER(read_image_oiio, f, readimage, 1, 1, NULL, HAVE_LIBOPENIMAGEIO);

Symbol
lux_write_image_oiio(ArgumentCount narg, Symbol ps[])
{
  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (array_num_dims(ps[0]) != 2 && array_num_dims(ps[0]) != 3)
    return luxerror("Need 2D or 3D array", ps[0]);

  if (!symbolIsScalar(ps[1]))
    return cerror(NEED_SCAL, ps[1]);
  int type = int_arg(ps[1]);

  if (!symbolIsStringScalar(ps[2]))
    return cerror(NEED_STR, ps[2]);
  auto filename = string_value(ps[2]);

  OIIO::TypeDesc oiiotype;
  switch (type) {
  case 1:
    oiiotype = OIIO::TypeDesc::UINT8;
    break;
  case 2:
    oiiotype = OIIO::TypeDesc::UINT16;
    break;
  case 4:
    oiiotype = OIIO::TypeDesc::UINT32;
    break;
  default:
    oiiotype = OIIO::TypeDesc::FLOAT;
    break;
  }

  auto out = OIIO::ImageOutput::create(filename);
  if (!out)
    return luxerror("Cannot write file %s", ps[2], filename);

  auto ndim = array_num_dims(ps[0]);
  auto dims = array_dims(ps[0]);
  OIIO::ImageSpec spec(ndim >= 3? dims[1]: dims[0], // width
                       ndim >= 3? dims[2]: dims[1], // height
                       ndim >= 3? dims[0]: 1);      // channels
  spec.format = oiiotype;
  out->open(filename, spec);
  out->write_image(OIIO::TypeDesc::FLOAT, array_data(ps[0]));
  out->close();
  return LUX_OK;
}
REGISTER(write_image_oiio, s, writeimage, 3, 3, NULL, HAVE_LIBOPENIMAGEIO);

#endif
