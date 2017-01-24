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
    sendf("set auto fix\n"
          "set offsets graph 0.05, graph 0.05, graph 0.05, graph 0.05\n");
    flush();
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
  void
  sendf(const char* format, ...)
  {
    if (m_pipe) {
      va_list ap;

      char buf[1024];

      va_start(ap, format);
      vsnprintf(buf, sizeof(buf), format, ap);
      va_end(ap);
      if (buf[0] != '\n')
        printf("Sending to gnuplot: %s\n", buf);
      fputs(buf, m_pipe);
    }
  }

  /// Sends unformatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param data points to the beginning of the data to send.
  ///
  /// \param size is the number of bytes of data to send.
  void write(void* data, size_t size)
  {
    if (m_pipe) {
      fwrite(data, 1, size, m_pipe);
    }
  }

  /// Flush the recently written input to the "gnuplot" program.
  void
  flush()
  {
    fflush(m_pipe);
  }

  static const char*
  gnuplot_type(Symboltype lux_type)
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

private:
  /// A file pointer for the pipe to the "gnuplot" program.
  FILE* m_pipe;
};

static GnuPlot gp;

/// gplot[,<x>[,<y>]][,command=<command>]
///
/// Plots data through gnuplot.
///
/// <x> and <y> provide the x and y coordinates of the data points.
/// They must have the same dimensional structure.
///
/// <command> is an optional gnuplot command to send to gnuplot.
int32_t lux_gnuplot(int32_t narg, int32_t ps[])
{
  std::string gnuplot_command;
  StandardArguments standard_args;

  if (narg > 2 && ps[2] && symbolIsString(ps[2])) {
    // gnuplot command
    gnuplot_command = string_value(ps[2]);
    --narg;
  }

  int32_t num_data = 0;
  for (int i = 0; i < narg; ++i) {
    if (ps[i])
      ++num_data;
  }

  if (num_data) {
    Pointer *data;
    loopInfo *info;

    if (standard_args.set(num_data, ps, "i^*;i^&?", &data, &info) < 0)
      return LUX_ERROR;

    if (gnuplot_command.empty()) {
      switch (num_data) {
      case 1:
        gnuplot_command
          = "plot '-' binary array=(%d) format=\"%s\" notitle with lines;";
        break;
      case 2:
        gnuplot_command
          = "plot '-' binary record=(%d) format=\"%s\" "
          "using 1:2 notitle with lines;";
        break;
      }
    }

    int32_t nelem = std::numeric_limits<int32_t>::max();
    for (int i = 0; i < num_data; ++i) {
      if (info[i].nelem < nelem)
        nelem = info[i].nelem;       // get the mininum
    }

    const char* gnuplot_type = GnuPlot::gnuplot_type(info[0].type);
    if (!gnuplot_type)
      return cerror(ILL_TYPE, ps[0]);

    switch (num_data) {
    case 1:
      gp.sendf(gnuplot_command.c_str(), nelem, gnuplot_type);
      gp.sendf("\n");
      gp.write(&data[0].b[0], nelem*lux_type_size[info[0].type]);
      break;
    case 2:
      gp.sendf(gnuplot_command.c_str(), nelem, gnuplot_type);
      gp.sendf("\n");
      gp.write(&data[0].b[0], nelem*lux_type_size[info[0].type]);
      gp.write(&data[1].b[0], nelem*lux_type_size[info[1].type]);
      break;
    }
  } else if (!gnuplot_command.empty()) {
    gp.sendf("%s\n", gnuplot_command.c_str());
  }
  gp.flush();

  return LUX_OK;
}
REGISTER(gnuplot, s, gplot, 1, 3, "::command");

int32_t lux_gnuplot_with_image(int32_t narg, int32_t ps[],
                               std::string gnuplot_command_fmt) {
  StandardArguments standard_args;
  Pointer *data;
  loopInfo *info;

  if (standard_args.set(narg, ps, "i>B>1,>1", &data, &info) < 0)
    return LUX_ERROR;

  const char* gnuplot_type = GnuPlot::gnuplot_type(info[0].type);
  if (!gnuplot_type)
    return cerror(ILL_TYPE, ps[0]);

  gp.sendf(gnuplot_command_fmt.c_str(), info[0].dims[0], info[0].dims[1],
           gnuplot_type);
  gp.sendf("\n");
  gp.write(&data[0].b[0], info[0].nelem*lux_type_size[info[0].type]);
  gp.flush();

  return LUX_OK;
}

/// gtv,<x>
///
/// Display an image through gnuplot.
///
/// <x> must have two dimensions.
int32_t lux_gnutv(int32_t narg, int32_t ps[])
{
  return lux_gnuplot_with_image(narg, ps,
                                "plot '-' binary array=(%d,%d) format=\"%s\" "
                                "notitle with image;");
}
REGISTER(gnutv, s, gtv, 1, 1, ":command");

/// gplot3d,<x>
///
/// Display a 3D plot of image <x>, using gnuplot.
///
/// <x> must have two dimensions.
int32_t lux_gnuplot3d(int32_t narg, int32_t ps[])
{
  return lux_gnuplot_with_image(narg, ps,
                                "splot '-' binary array=(%d,%d) format=\"%s\" "
                                "notitle with pm3d;");
}
REGISTER(gnuplot3d, s, gplot3d, 1, 1, ":command");

/// gcontour,x[,/equalxy][,/image]
///
/// Displays a contour plot of image <x>, using gnuplot.
///
/// If option /equalxy is given, then the contour plot has the same
/// aspect ratio as the image.  Otherwise, the contour plot has the
/// standard aspect ratio.
///
/// If option /image is given, then the contour plot is shown on top
/// of the image.  Otherwise, only the contour plot is shown.
int32_t lux_gnucontour(int32_t narg, int32_t ps[])
{
  std::string gnuplot_command_fmt;

  if (internalMode & 1)       // /equalxy
    // "set view equal xy" leads to plots with no contours
    gnuplot_command_fmt += "set size ratio -1; ";
  gnuplot_command_fmt += "set view map; ";
  if (internalMode & 2)         // /image
    gnuplot_command_fmt += "set surface; set contour surface; ";
  else
    gnuplot_command_fmt += "unset surface; set contour base; ";
  gnuplot_command_fmt
    += "splot '-' binary array=(%d,%d) format=\"%s\" title '' ";
  if (internalMode & 2)
    gnuplot_command_fmt += "with pm3d;"; // with image => no contours
  else
    gnuplot_command_fmt += "with lines;";

  return lux_gnuplot_with_image(narg, ps, gnuplot_command_fmt);
}
REGISTER(gnucontour, s, gcontour, 1, 1, ":1equalxy:2image");

/*

  set view ratio -1   => x and y axis have same scale unit (better than
  set view equal xy)

  set view map  => vertical view
  unset surface  => don't draw 3D surface (e.g., only contours)
          can add to splot command
  set contour base|surface|both  => draw contours
  set hidden3d  => hidden line removal

  plot '-' binary array=(30) format="%float" with lines; => y only
  plot '-' binary record=(30) format="%float" using 1:2 with lines => x and y

  splot '-' binary array=(30,40) with lines  => 3D wire mesh plot
  splot '-' binary array=(30,40) with image  => flat projected image

  splot '-' binary array=(30,40) with pm3d (146) => draw solid surface

 */
