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
#include "GnuPlot.hh"
#include <iostream>             // for cout
#include <limits>               // for numeric_limits

static GnuPlot gp;

int32_t lux_gnuplot_command(int32_t narg, int32_t ps[])
{
  if (!symbolIsString(ps[0]))
    cerror(NEED_STR, ps[0]);
  gp.sendf(string_value(ps[0]));
  gp.flush();
  return LUX_OK;
}
REGISTER(gnuplot_command, s, gcommand, 1, 1, NULL);

/// gplot[,<x>],<y>
///
/// Plots data through gnuplot.
///
/// <x> and <y> provide the x and y coordinates of the data points.
/// They must have the same dimensional structure.
int32_t lux_gnuplot(int32_t narg, int32_t ps[])
{
  std::string gnuplot_command;
  StandardArguments standard_args;

  Pointer *data;
  loopInfo *info;

  if (standard_args.set(narg, ps, "i^*;i^&?", &data, &info) < 0)
    return LUX_ERROR;

  gnuplot_command = "set auto fix; "
    "set offsets graph 0.05, graph 0.05, graph 0.05, graph 0.05; ";
  switch (narg) {
  case 1:
    gnuplot_command
      += "plot '-' binary array=(%d) format=\"%s\" notitle with lines;";
    break;
  case 2:
    gnuplot_command
      += "plot '-' binary record=(%d) format=\"%s\" "
      "using 1:2 notitle with lines;";
    break;
  }

  int32_t nelem = std::numeric_limits<int32_t>::max();
  for (int i = 0; i < narg; ++i) {
    if (info[i].nelem < nelem)
      nelem = info[i].nelem;       // get the mininum
  }

  const char* gnuplot_type = GnuPlot::gnuplot_type(info[0].type);
  if (!gnuplot_type)
    return cerror(ILL_TYPE, ps[0]);

  switch (narg) {
  case 1:
    gp.sendf(gnuplot_command.c_str(), nelem, gnuplot_type);
    gp.sendf("\n");
    gp.write(&data[0].b[0], nelem*lux_type_size[info[0].type]);
    break;
  case 2:
    gp.sendf(gnuplot_command.c_str(), nelem, gnuplot_type);
    gp.sendf("\n");
    uint8_t* p1 = &data[0].b[0];
    uint8_t* p2 = &data[1].b[0];
    size_t size = lux_type_size[info[0].type];
    for (int i = 0; i < nelem; ++i) {
      gp.write(p1, size);  p1 += size;
      gp.write(p2, size);  p2 += size;
    }
    break;
  }
  gp.flush();

  return LUX_OK;
}
REGISTER(gnuplot, s, gplot, 1, 2, NULL);

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
REGISTER(gnutv, s, gtv, 1, 1, NULL);

/// gplot3d[,<x>,<y>],<z>
///
/// Display a 3D plot of height values <z>, using gnuplot.
///
/// <z> must have two dimensions.
///
/// If <x> and <y> are also specified, then they indicate the x and y
/// coordinates.  <x> and <y> must then be one-dimensional arrays.
/// <x> contains the x coordinate of every column, and <y> contains
/// the y coordinate of every row.  Then the number of elements of <x>
/// must be equal to the number of columns of <z> (dimen(z,0)), and
/// the number of elements of <y> must be equal to the number of rows
/// of <z> (dimen(z,1)).
int32_t lux_gnuplot3d(int32_t narg, int32_t ps[])
{
  switch (narg) {
  case 1:
    return lux_gnuplot_with_image(narg, ps,
                                  "splot '-' binary array=(%d,%d) format=\"%s\" "
                                  "notitle with pm3d;");
  case 3:
    {
      StandardArguments standard_args;
      Pointer *data;
      loopInfo *info;
      int32_t myps[3];

      for (int i = 0; i < narg; ++i)
        myps[i] = lux_float(1, &ps[i]);
      if (standard_args.set(narg, myps, "iF*;iF*;iF>1,>1", &data, &info) < 0)
        return LUX_ERROR;

      if (info[0].ndim < 1 || info[0].ndim > 2)
        return luxerror("Expected 1 or 2 dimensions, found %d", ps[0],
                        info[0].ndim);

      if (info[1].ndim != info[0].ndim)
        return luxerror("Expected same dimension count as for X", ps[1]);

      switch (info[0].ndim) {
      case 1:
        {
          if (info[0].dims[0] != info[2].dims[0])
            return luxerror("Size should be equal to 1st dimension of Z",
                            ps[0]);

          if (info[1].dims[0] != info[2].dims[1])
            return luxerror("Size should be equal to 2nd dimension of Z",
                            ps[1]);

          // We cannot send a nonuniform matrix to gnuplot through a
          // pipe, because gnuplot 5.0 uses fseek and ftell on the
          // data, and those don't work on pipes (at least on
          // GNU/Linux).  So we send the data as text to a temporary
          // file, then tell gnuplot to read the file.
          //
          // We want to clean up after ourselves and remove that file
          // when we don't need it anymore, However, our communication
          // with gnuplot is one-way; We don't get a signal when
          // gnuplot is done creating the plot and doesn't need the
          // file anymore.  So, we must not delete the file
          // immediately after sending the command to gnuplot, because
          // gnuplot may not have finished creating the plot yet.
          //
          // Our current solution is to delete the file at the
          // beginning of the next command that needs such a file.
          // This fails if the next such command is issued before the
          // previous one has been completed by gnuplot (e.g., in
          // batch mode).

          gp.data_remove();     // previous data file, if any
          std::ofstream& tmp = gp.data_ofstream();
          if (tmp.is_open()) {
            std::cout << "Writing to " << gp.data_file_name() << std::endl;
            // write the number of columns
            float n = info[2].dims[0];
            tmp.write((char*) &n, sizeof(n));
            // write the x coordinates
            tmp.write((char*) &data[0].f[0], info[2].dims[0]*sizeof(data[0].f[0]));

            // treat all rows
            char* z = (char*) &data[2].f[0];
            size_t size = info[2].dims[0]*sizeof(data[2].f[0]);
            for (int i = 0; i < info[2].dims[1]; ++i) {
              // write the y coordinate
              tmp.write((char*) &data[1].f[i], sizeof(data[1].f[i]));
              // write the row of z coordinates
              tmp.write(z, size);
              z += size;
            }
            tmp.close();

            gp.sendf("splot '%s' binary nonuniform matrix using 1:2:3 notitle with pm3d;\n",
                     gp.data_file_name().c_str());
            gp.flush();
          } else
            return luxerror("Could not open temp file for transferring data to gnuplot", 0);
        }
        break;
      case 2:
        for (int i = 0; i < 2; ++i) {
          for (int j = 0; j < 2; ++j) {
            if (info[i].dims[j] != info[2].dims[j])
              return luxerror("Dimensions should be equal to that of Z",
                              ps[i]);
          }
        }
        break;
      }
    }
    break;
  default:
    return luxerror("Expected 1 or 3 arguments, found %d", 0, narg);
  }
  return LUX_OK;
}
REGISTER(gnuplot3d, s, gplot3d, 1, 3, NULL);

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
    += "set cntrparam cubicspline; "
    "splot '-' binary array=(%d,%d) format=\"%s\" title '' ";
  if (internalMode & 2)
    gnuplot_command_fmt += "with pm3d;"; // with image => no contours
  else
    gnuplot_command_fmt += "with lines;";
  gnuplot_command_fmt += "reset; ";

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

  nonuniform matrix:

  nx    x0  x1  x2  x3
  y0   v00 v10 v20 v30
  y1   v01 v11 v21 v31

  must be float

  splot '-' nonuniform matrix using 1:2:3
  splot '-' binary matrix using 1:2:3

 */
