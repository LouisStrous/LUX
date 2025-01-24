#include "config.h"

#include "action.hh"
#include "libraw/libraw.h"
#include <string>

class LibRawProcessor
{
public:
  // destructor

  ~LibRawProcessor()
  {
    m_processor.recycle();
  }

  LibRaw& operator()()
  {
    return m_processor;
  }

  // const methods

  const libraw_iparams_t&
  idata() const
  {
    return m_processor.imgdata.idata;
  }

  const libraw_image_sizes_t&
  sizes() const
  {
    return m_processor.imgdata.sizes;
  }

  // non-const methods

  int
  open_file(const char* path)
  {
    return m_processor.open_file(path);
  }

  int
  unpack()
  {
    return m_processor.unpack();
  }

  int
  dcraw_process()
  {
    return m_processor.dcraw_process();
  }

  libraw_processed_image_t*
  dcraw_make_mem_image(int* error = nullptr)
  {
    return m_processor.dcraw_make_mem_image(error);
  }

private:
  LibRaw m_processor;
};

Symbol
lux_read_raw_image(ArgumentCount narg, Symbol ps[])
{
  if (!symbolIsString(ps[0]))
    return cerror(NEED_STR, ps[0]);

  auto path = string_value(ps[0]);

  LibRawProcessor processor;
  int ret;
  if ((ret = processor.open_file(path)) != LIBRAW_SUCCESS)
    return luxerror("Cannot open %s: %s", ps[0], path, libraw_strerror(ret));

  libraw_iparams_t idata = processor.idata(); // main image parameters
  printf("camera make : %s", idata.make);
  if (strncmp(idata.make, idata.normalized_make, 64))
    printf(" -> %s", idata.normalized_make);
  putchar('\n');
  printf("camera model: %s", idata.model);
  if (strncmp(idata.model, idata.normalized_model, 64))
    printf(" -> %s", idata.normalized_model);
  putchar('\n');
  printf("software    : %s\n", idata.software);
  printf("raw_count   : %d\n", idata.raw_count);
  printf("#colors     : %d\n", idata.colors);
  printf("filters     : %x\n", idata.filters);
  printf("colors desc : %s\n", idata.cdesc);

  if ((ret = processor.unpack()) != LIBRAW_SUCCESS)
    return luxerror("Cannot unpack %s: %s", ps[0], path, libraw_strerror(ret));

  if ((ret = processor.dcraw_process()) != LIBRAW_SUCCESS)
    return luxerror("Cannot dcraw_process %s: %s", ps[0], path,
                     libraw_strerror(ret));

  auto processed_image = processor.dcraw_make_mem_image();

  int32_t dims[3] = {
    processed_image->colors,     // colors per pixel
    processed_image->width,
    processed_image->height
  };

  Symbol iq = array_scratch(LUX_FLOAT, 3, dims);
  float* tgt = static_cast<float*>(array_data(iq));

  size_t n = dims[0]*dims[1]*dims[2];
  if (processed_image->bits == 8)
  {
    auto src = reinterpret_cast<uint8_t const*>(processed_image->data);
    for (int i = 0 ; i < n; ++i)
      *tgt++ = *src++;
  }
  else
  {
    auto src = reinterpret_cast<uint16_t const*>(processed_image->data);
    for (int i = 0 ; i < n; ++i)
      *tgt++ = *src++;
  }
  return iq;
}
REGISTER(read_raw_image, f, readrawimage, 1, 1, nullptr);
