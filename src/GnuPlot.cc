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
#include <vector>

GnuPlot::GnuPlot()
  : m_datablock_count(0)
{
  m_pipe = popen("gnuplot", "w");
  if (!m_pipe)
    luxerror("Unable to start 'gnuplot'", 0);
  m_verbosity = 0;
}

GnuPlot::~GnuPlot()
{
  if (m_verbosity > 0) {
    puts("Closing gnuplot");
  }
  if (m_pipe) {
    pclose(m_pipe);
    m_pipe = 0;
  }
  data_remove();
}

GnuPlot&
GnuPlot::operator=(const GnuPlot& src) {
  m_pipe = src.m_pipe;
  return *this;
}

int
GnuPlot::set_verbosity(int value) {
  int old = m_verbosity;
  m_verbosity = value;
  return old;
}

void
GnuPlot::sendf(const char* format, ...)
{
  if (m_pipe) {
    char buf[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (m_verbosity) {
      size_t n = strlen(buf);
      if (buf[n] == '\n') {
        if (n == 1) {
          puts("Sending to gnuplot: <newline>");
        } else {
          printf("Sending to gnuplot: X '%.*s'\n", n - 1, buf);
        }
      } else {
        printf("Sending to gnuplot: '%s'\n", buf);
      }
    }
    fputs(buf, m_pipe);
  }
}

void
GnuPlot::write(void* data, size_t size)
{
  if (m_pipe) {
    fwrite(data, 1, size, m_pipe);
  }
}

void
GnuPlot::flush()
{
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

void
GnuPlot::remember_for_current_datablock(const char* format, ...)
{
  char buf[1024];
  va_list ap;

  uint32_t index = current_datablock_index();
  if (index > 0) {
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    std::string text = std::string(buf);
    while (m_datablock_plot_elements.size() < index)
      m_datablock_plot_elements.push_back("");
    m_datablock_plot_elements[index - 1] = text;
  }
}

std::string
GnuPlot::construct_plot_or_splot_command(std::string head) const
{
  std::ostringstream oss;

  oss << head << " ";
  bool first = true;
  uint32_t index = 1;
  for (auto it = m_datablock_plot_elements.cbegin();
       it != m_datablock_plot_elements.cend(); ++it) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }
    oss << "$LUX" << index << " " << *it;
    ++index;
  }
  return oss.str();
}

std::string
GnuPlot::construct_plot_command() const
{
  return construct_plot_or_splot_command("plot");
}

std::string
GnuPlot::construct_splot_command() const
{
  return construct_plot_or_splot_command("splot");
}

uint32_t
GnuPlot::current_datablock_index() const
{
  return m_datablock_count;
}

uint32_t
GnuPlot::next_available_datablock_index()
{
  return ++m_datablock_count;
}

void
GnuPlot::discard_datablocks()
{
  if (m_pipe) {
    while (m_datablock_count > 0) {
      sendf("undefine %d;\n", m_datablock_count--);
    }
  }
  m_datablock_plot_elements = DatablockBackendCollection();
}
