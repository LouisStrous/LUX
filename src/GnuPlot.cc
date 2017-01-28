/* This is file GnuPlot.cc.

Copyright 2017 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "GnuPlot.hh"

#include "luxdefs.hh"           // for Symboltype
#include <cstddef>              // for size_t
#include <cstdint>              // for int32_t
#include <cstring>              // for strcmp
#include <ctime>                // for time
#include <fstream>              // for ofstream
#include <iostream>             // for cout
#include <sstream>              // for ostringstream
#include <string>

GnuPlot::GnuPlot()
{
  m_pipe = popen("gnuplot", "w");
  if (!m_pipe)
    luxerror("Unable to start 'gnuplot'", 0);
}

GnuPlot::~GnuPlot()
{
  if (m_pipe) {
    pclose(m_pipe);
  }
  data_remove();
}

void
GnuPlot::sendf(const char* format, ...)
{
  if (m_pipe) {
    va_list ap;

    char buf[1024];

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    printf("Sending to gnuplot: '%s'\n", strcmp(buf, "\n")? buf: "<newline>");
    fputs(buf, m_pipe);
  }
}

void
GnuPlot::write(void* data, size_t size)
{
  if (m_pipe) {
    unsigned char* datac = (unsigned char*) data;
    printf("Sending %d bytes to gnuplot:\n", size);
    for (int i = 0; i < size; ++i)
      printf(" %02x", datac[i]);
    putchar('\n');
    fwrite(data, 1, size, m_pipe);
  }
}

void
GnuPlot::flush()
{
  puts("Flushing pipe to gnuplot");
  fflush(m_pipe);
}

std::ofstream&
GnuPlot::data_ofstream()
{
  initialize_data_file();
  return m_data_ofstream;
}

std::string
GnuPlot::data_file_name()
{
  if (m_data_file_name.empty())
    initialize_data_file();
  return m_data_file_name;
}

void
GnuPlot::data_remove()
{
  if (!m_data_file_name.empty()) {
    if (m_data_ofstream.is_open())
      m_data_ofstream.close();
    std::cout << "Removing file " << m_data_file_name << std::endl;
    if (std::remove(m_data_file_name.c_str())) {
      std::cout << "Unable to delete file " << m_data_file_name << std::endl;
    }
    m_data_file_name.clear();
  }
}

char const*
GnuPlot::gnuplot_type(Symboltype lux_type)
{
  switch (lux_type) {
  case LUX_INT8:
    return "%ubyte";
    break;
  case LUX_INT16:
    return "%int16";
    break;
  case LUX_INT32:
    return "%int32";
    break;
  case LUX_INT64:
    return "%int64";
    break;
  case LUX_FLOAT:
    return "%float32";
    break;
  case LUX_DOUBLE:
    return "%float64";
    break;
  default:
    return NULL;
  }
}

bool
GnuPlot::initialize_data_file(void)
{
  if (m_data_ofstream.is_open())
    return true;

  std::ostringstream oss;
  oss << ".gpi_" << std::time(0);
  m_data_ofstream = std::ofstream(oss.str().c_str(), (std::ios_base::out));
  if (m_data_ofstream.is_open()) {
    m_data_file_name = oss.str();
    return true;
  } else
    return false;
}
