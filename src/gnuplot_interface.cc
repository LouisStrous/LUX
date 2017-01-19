/* This is file gnuplot_interface.cc.

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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "action.hh"
#include "symbols.hh"
#include <stdio.h>              // for popen, fflush, vfprintf
#include <stdarg.h>             // for va_start
#include <limits>
#include <string>

/// A class that provides a buffered interface for writing information
/// to the external "gnuplot" program.
class GnuPlot {
public:
  /// Constructor.  Starts the "gnuplot" program, if available.
  GnuPlot()
  {
    m_pipe = popen("gnuplot", "w");
    if (!m_pipe)
      luxerror("Unable to start 'gnuplot'", 0);
  }

  /// Destructor.  Closes the interface, including the "gnuplot"
  /// process started by the constructor.
  ~GnuPlot()
  {
    if (m_pipe) {
      pclose(m_pipe);
    }
  }

  /// Sends formatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param format is a printf-style format string.
  ///
  /// \param ... are any number of data values.  They get formatted
  /// according to the format string.
  void sendf(const char* format, ...)
  {
    if (m_pipe) {
      va_list ap;

      va_start(ap, format);
      vfprintf(m_pipe, format, ap);
      va_end(ap);
    }
  }

  /// Flush the recently written input to the "gnuplot" program.
  void flush()
  {
    fflush(m_pipe);
  }
private:
  /// A file pointer for the pipe to the "gnuplot" program.
  FILE* m_pipe;
};

/// GNUPLOT,<x>[,<y>][,command=<command>]
///
/// Plots data through gnuplot.
///
/// <x> and <y> provide the x and y coordinates of the data points.
/// They must have the same dimensional structure.
///
/// <command> is an optional gnuplot command to send to gnuplot.  If
/// it is not specified, then "plot '-' notitle with lines;" is sent.
int32_t lux_gnuplot(int32_t narg, int32_t ps[])
{
  static GnuPlot gp;
  std::string gnuplot_command;

  if (narg > 2 && ps[2] && symbolIsString(ps[2])) {
    // gnuplot command
    gnuplot_command = string_value(ps[2]);
    --narg;                     // exclude from count
  } else {
    // standard gnuplot command
    gnuplot_command = "plot '-' notitle with lines;";
  }

  int32_t num_data = narg;

  pointer *data;
  loopInfo *info;

  if (num_data > 2)
    num_data = 2;               // at most 2 values per point

  int32_t iq;
  if (standard_args(num_data, ps, "i^*;i^&?", &data, &info) < 0)
    return LUX_ERROR;

  int32_t nelem = std::numeric_limits<int32_t>::max();
  for (int i = 0; i < num_data; ++i) {
    if (info[i].nelem < nelem)
      nelem = info[i].nelem;       // get the mininum
  }

  gp.sendf("%s\n", gnuplot_command.c_str());

  if (num_data) {
    switch (info[0].type) {
    case LUX_INT8:
      for (int i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%d ", *data[j].b++);
        gp.sendf("\n");
      }
      break;
    case LUX_INT16:
      for (int32_t i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%d ", *data[j].w++);
        gp.sendf("\n");
      }
      break;
    case LUX_INT32:
      for (int32_t i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%d ", *data[j].l++);
        gp.sendf("\n");
      }
      break;
    case LUX_INT64:
      for (int32_t i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%ld ", *data[j].q++);
        gp.sendf("\n");
      }
      break;
    case LUX_FLOAT:
      for (int32_t i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%g ", *data[j].f++);
        gp.sendf("\n");
      }
      break;
    case LUX_DOUBLE:
      for (int32_t i = 0; i < nelem; ++i) {
        for (int j = 0; j < num_data; ++j)
          gp.sendf("%g ", *data[j].d++);
        gp.sendf("\n");
      }
      break;
    default:
      return cerror(ILL_TYPE, ps[0]);
    }
    gp.sendf("e\n");
  }
  gp.flush();

  return LUX_OK;
}
