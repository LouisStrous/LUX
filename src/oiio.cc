#include "config.h"

#if HAVE_LIBOPENIMAGEIO
#include "action.hh"
#include <OpenImageIO/imageio.h>

// <x> = readimage(<file>)
int32_t
lux_read_image_oiio(int32_t narg, int32_t ps[])
{
  char* filename;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, ps[0]);
  filename = string_value(ps[0]);

  auto in = OIIO::ImageInput::open(filename);
  if (!in)
    return luxerror("Cannot open image file '%s' via OIIO", ps[0], filename);
  const OIIO::ImageSpec& spec = in->spec();
  uint32_t result;
  auto oiiotype = spec.format;
  {
    int dims[3];
    dims[0] = spec.nchannels;
    dims[1] = spec.width;
    dims[2] = spec.height;
    Symboltype luxtype = LUX_INT16;
    switch (oiiotype.basetype) {
    case OIIO::TypeDesc::UINT8:
    case OIIO::TypeDesc::INT8:
      luxtype = LUX_INT8;
      oiiotype = OIIO::TypeDesc::UINT8;
      break;
    case OIIO::TypeDesc::UINT16:
    case OIIO::TypeDesc::INT16:
      luxtype = LUX_INT16;
      oiiotype = OIIO::TypeDesc::INT16;
      break;
    case OIIO::TypeDesc::UINT32:
    case OIIO::TypeDesc::INT32:
      luxtype = LUX_INT32;
      oiiotype = OIIO::TypeDesc::INT32;
      break;
    default:
      luxtype = LUX_FLOAT;;
      oiiotype = OIIO::TypeDesc::FLOAT;
      break;
    }
    result = array_scratch(luxtype, sizeof(dims)/sizeof(*dims), dims);
  }
  in->read_image(oiiotype, array_data(result));
  in->close();
  return result;
}
REGISTER(read_image_oiio, f, readimage, 1, 1, NULL);

#endif
