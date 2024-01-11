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
    dims[0] = spec.nchannels;
    dims[1] = spec.width;
    dims[2] = spec.height;
    if (!oiiotype.is_floating_point()) {
      switch (oiiotype.elementsize()) {
      case 1:
        luxtype = LUX_INT8;
        oiiotype = OIIO::TypeDesc::UINT8;
      break;
      case 2:
        luxtype = LUX_INT16;
        oiiotype = OIIO::TypeDesc::INT16;
        break;
      case 4:
        luxtype = LUX_INT32;
        oiiotype = OIIO::TypeDesc::INT32;
        break;
      default:
        luxtype = LUX_FLOAT;
        oiiotype = OIIO::TypeDesc::FLOAT;
        break;
      }
    } else {
      luxtype = LUX_FLOAT;
      oiiotype = OIIO::TypeDesc::FLOAT;
    }
    result = array_scratch(luxtype, sizeof(dims)/sizeof(*dims), dims);
  }
  auto scanlinesize = spec.width*spec.nchannels*lux_type_size[luxtype];
  in->read_image(0, 0, 0, -1,
                 oiiotype, ((char*) array_data(result))
                 + (spec.height - 1)*scanlinesize,
                 OIIO::AutoStride,  // default x stride
                 -scanlinesize,     // special y stride
                 OIIO::AutoStride); // default z stride
  in->close();
  return result;
}
REGISTER(read_image_oiio, f, readimage, 1, 1, NULL);

#endif
