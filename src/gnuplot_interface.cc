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
#include <cmath>                // for fmodf
#include <cstring>              // for strcmp
#include <iostream>             // for cout
#include <limits>               // for numeric_limits
#include <sstream>              // for ostringstream

static GnuPlot* gp = 0;

int32_t lux_gcommand(int32_t narg, int32_t ps[])
{
  if (!gp)
    gp = new GnuPlot;

  if (narg > 1 && ps[1]) {
    gp->set_verbosity(int_arg(ps[1]));
  }
  if (symbolIsString(ps[0])) {
    char* command = string_value(ps[0]);
    if (!strncmp(command, "exit", 4)) {
      delete gp;
      gp = new GnuPlot();       // get a new one; closes the old one
    } else {
      gp->sendf("%s\n", string_value(ps[0]));
      gp->flush();
    }
  } else if (ps[0]) {
    cerror(NEED_STR, ps[0]);
  }
  return LUX_OK;
}
REGISTER(gcommand, s, gcommand, 1, 2, ":verbose");

/// Plots or overplots data through gnuplot.  This function gets
/// called for the LUX "gplot" and "goplot" subroutines.  See there
/// for details of the LUX arguments.
///
/// \par clear says whether to start a new plot (if #clear is true) or
/// to add to the previous plot (if #clear is false).
///
/// \par narg is the number of LUX arguments
///
/// \par ps is an array of LUX arguments.
///
/// \returns #LUX_OK for success, #LUX_ERROR for failure.
int32_t lux_gplot_or_goplot(bool clear, int32_t narg, int32_t ps[])
{
  // LUX offers subroutines "plot" and "oplot" based on X11.  "plot"
  // generates a new plot, and "oplot" adds another curve to an
  // existing plot.  We want corresponding subroutines "gplot" and
  // "goplot" based on gnuplot.  However, gnuplot doesn't have a
  // facility to add more data to an existing plot.  It requires
  // specifying all data necessary for the entire plot in a single
  // command.
  //
  // Our way around this limitation of gnuplot is as follows:
  //
  // For gplot:
  //
  //   Clear any existing data blocks and associated 'plot elements'
  //   generated through gplot/goplot.  Send the data to an unused
  //   gnuplot data block.  Remember the 'plot element' for the data
  //   block.  Send a plot command for the data block and plot element
  //   to gnuplot.
  //
  // For goplot:
  //
  //   Send the data to an unused gnuplot data block.  Remember the
  //   'plot element' for the data block.  Send a plot command for all
  //   data blocks and plot elements to gnuplot.
  //
  // All setting up of the plot (e.g., defining titles) is done for
  // gplot and does not need to be remembered for the data block.

  StandardArguments standard_args;
  Pointer *data;
  loopInfo *info;

  // the first three arguments indicate <x>, <y>, <z>
  int32_t enarg = narg - 3;
  int32_t* eps = ps + 3;

  if (enarg < 0)
    enarg = 0;

  if (!gp)
    gp = new GnuPlot;

  if (clear) {                  // gplot
    gp->discard_datablocks();
  }

  // reserve the next data block for this call
  int32_t datablock_index = gp->next_available_datablock_index();

  int style = 1;
  if (enarg) {
    if (*eps) {               // style
      style = int_arg(*eps);
    }
    --enarg;
    ++eps;
  }

  bool done = false;
  if (clear) {                  // for gplot
    if (enarg) {
      if (*eps) {               // xtitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp->sendf("set xlabel \"%s\"\n", text);
            done = 1;
          }
        }
        else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp->sendf("set xlabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // ytitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp->sendf("set ylabel \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp->sendf("set ylabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // ztitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp->sendf("set zlabel \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp->sendf("set zlabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // title
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp->sendf("set title \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp->sendf("set title\n");

    gp->sendf("set auto fix; "
             "set offsets graph 0.05, graph 0.05, graph 0.05, graph 0.05; "
             "set tics; ");

    extern float plims[];

    if (plims[0] != plims[1])
      gp->sendf("set xrange [%f:%f]\n", plims[0], plims[1]);
    else
      gp->sendf("set xrange [*:*]\n");

    if (plims[2] != plims[3])
      gp->sendf("set yrange [%f:%f]\n", plims[2], plims[3]);
    else
      gp->sendf("set yrange [*:*]\n");

    if (internalMode & 2)         // logarithmic x axis
      gp->sendf("set logscale x\n");
    else
      gp->sendf("unset logscale x\n");

    if (internalMode & 4)         // logarithmic y axis
      gp->sendf("set logscale y\n");
    else
      gp->sendf("unset logscale y\n");

    if (internalMode & 8)         // logarithmic z axis
      gp->sendf("set logscale z\n");
    else
      gp->sendf("unset logscale z\n");

    if (internalMode & 14)
      gp->sendf("set format \"%%g\"\n");
  }

  char* legend = 0;
  if (enarg) {
    if (*eps) {                 // legend
      if (symbolIsString(*eps))
        legend = string_value(*eps);
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  }

  // transform the coordinates to type float
  int32_t myps[3];
  int32_t ndata = 0;
  for (int32_t i = 0; i < 3 && i < narg; ++i) {
    if (ps[i]) {
      myps[i] = lux_float(1, &ps[i]);
      ++ndata;
    } else
      break;
  }

  if (standard_args.set(ndata, myps, "iF*;iF&?;iF&?", &data, &info) < 0)
    return LUX_ERROR;

  // send the data to a gnuplot data block
  gp->sendf("$LUX%d << EOD\n", datablock_index);
  switch (ndata) {
  case 1:
    for (int i = 0; i < info[0].nelem; ++i) {
      gp->sendf("%g\n", *data[0].f++);
    }
    break;
  case 2:
    for (int i = 0; i < info[0].nelem; ++i) {
      gp->sendf("%g %g\n", *data[0].f++, *data[1].f++);
    }
    break;
  case 3:
    for (int i = 0; i < info[0].nelem; ++i) {
      gp->sendf("%g %g %g\n", *data[0].f++, *data[1].f++, *data[2].f++);
    }
    break;
  }
  gp->sendf("EOD\n");

  // positive styles: lines + symbols
  // styles < -1 : symbols only
  // style == -1 : dots only
  std::string kind_text;
  std::string style_text;
  if (style >= 0) {
    kind_text = "lines";
    style_text = "dashtype";
  } else if (style < -1) {
    kind_text = "points";
    style_text = "pointtype";
    style = -style - 1;
  }

  switch (ndata) {
  case 1:
    if (style_text.empty())
      gp->remember_for_current_datablock("using 1 with dots title \"%s\"",
                                         legend? legend: "");
    else
      gp->remember_for_current_datablock("using 1 with %s title \"%s\" %s %d",
                                         kind_text.c_str(),
                                         legend? legend: "",
                                         style_text.c_str(), style);
    break;
  case 2:
    if (style_text.empty())
      gp->remember_for_current_datablock("using 1:2 with dots title \"%s\"",
                                         legend? legend: "");
    else
      gp->remember_for_current_datablock("using 1:2 with %s title \"%s\" %s %d",
                                         kind_text.c_str(),
                                         legend? legend: "",
                                         style_text.c_str(), style);
    break;
  case 3:
    if (style_text.empty())
      gp->remember_for_current_datablock("using 1:2:3 with dots title \"%s\"",
                                         legend? legend: "");
    else
      gp->remember_for_current_datablock("using 1:2:3 with %s title \"%s\" %s %d",
                                         kind_text.c_str(),
                                         legend? legend: "",
                                         style_text.c_str(), style);
    break;
  }

  switch (ndata) {
  case 1: case 2:
    gp->sendf("%s;\n", gp->construct_plot_command().c_str());
    break;
  case 3:
    gp->sendf("%s;\n", gp->construct_splot_command().c_str());
    break;
  }

  gp->flush();

  return LUX_OK;
}

/// gplot[,<x>],<y>[,<z>][,style=<style>,xtitle=<xtitle>,ytitle=<ytitle>,
///      ztitle=<ztitle>,title=<title>,legend=<legend>]
///      [,/lii,/lio,/loi,/loo]
///      [,/liii,/lioi,/loii,/looi,/liio,/lioo,/loio,/looo]
///
/// Plots data through gnuplot.
///
/// <x>, <y>, and <z> provide the x, y, and z coordinates of the data
/// points.  They must have the same dimensional structure.
///
/// <style> is an integer that specifies the type of line (solid,
/// dashed, etc.).
///
/// <xtitle> is the title for the x axis.
///
/// <ytitle> is the title for the y axis.
///
/// <ztitle> is the title for the z axis.
///
/// <title> is the main title, displayed above the plot.
///
/// <legend> is the text to display in the legend.
///
/// /lii asks for linear x and y axes; /lio asks for linear x and
/// logarithmic y axes; /loi asks for logarithmic x and linear y axes;
/// and /loo asks for logarithmic x and y axes.  /liii through /looo
/// likewise specify linear or logarithmic x, y, and z axes.
int32_t lux_gplot(int32_t narg, int32_t ps[]) {
  lux_gplot_or_goplot(true, narg, ps);
}
REGISTER(gplot, s, gplot, 1, 9, ":::style:xtitle:ytitle:ztitle:title:legend:0lii:2loi:4lio:6loo:0liii:2loii:4lioi:6looi:8liio:10loio:12lioo:14looo");

/// gplot[,<x>],<y>[,style=<style>,legend=<legend>]
///
/// Add another curve to the previous plot through gnuplot.
///
/// <x> and <y> provide the x and y coordinates of the data points.
/// They must have the same dimensional structure.
///
/// <style> is an integer that specifies the type of line (solid,
/// dashed, etc.).
///
/// <legend> is the text to display in the legend.
int32_t lux_goplot(int32_t narg, int32_t ps[]) {
  lux_gplot_or_goplot(false, narg, ps);
}
REGISTER(goplot, s, goplot, 1, 5, ":::style:legend");

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

  if (!gp)
    gp = new GnuPlot;

  gp->sendf(gnuplot_command_fmt.c_str(), info[0].dims[0], info[0].dims[1],
           gnuplot_type);
  gp->sendf("\n");
  gp->write(&data[0].b[0], info[0].nelem*lux_type_size[info[0].type]);
  gp->flush();

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
                                "unset xtics; unset ytics; "
                                "plot '-' binary array=(%d,%d) format=\"%s\" "
                                "notitle with image;");
}
REGISTER(gnutv, s, gtv, 1, 1, NULL);

/// gplot3d[,<x>,<y>],<z>
///    [,xtitle=<xtitle>,ytitle=<ytitle>,ztitle=<ztitle>,title=<title>
///    [,rotx=<rotx>,rotz=<rotz>]
///    [,contours=<contours>]
///    [,/flat]
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
///
/// <xtitle> is the title for the x axis.
///
/// <ytitle> is the title for the y axis.
///
/// <ztitle> is the title for the z axis.
///
/// <title> is the main title, displayed above the plot.
///
/// <rotx> is the rotation angle (in degrees) around the x axis.
///
/// <rotz> is the rotation angle (in degrees) around the z axis (after
/// applying the rotation around the x axis, if any).
///
/// <contours> specifies contour levels.  If <contours> is a scalar
/// equal to integer 1 (e.g., /contours), then it says to show
/// automatically selected contours.  Otherwise, if <contours> is a
/// real scalar, then it specifies the desired number of contours to
/// display, but the levels are selected automatically.  Otherwise, if
/// <contours> is an array, then it specifies the contour levels to
/// use.
///
/// /flat requests an orthographic view from above; i.e., a flat
/// image.
///
/// /lii requests linear x and y axes; /loi requests logarithmic x and
/// linear y axes; /lio requests linear x and logarithmic y axes; /loo
/// requests logarithmic x and y axes
int32_t lux_gnuplot3d(int32_t narg, int32_t ps[])
{
  int32_t enarg = narg - 3;
  int32_t* eps = ps + 3;

  if (enarg < 0)
    enarg = 0;

  if (!gp)
    gp = new GnuPlot;

  if (enarg) {
    if (*eps) {                 // xtitle
      if (symbolIsString(*eps))
        gp->sendf("set xlabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp->sendf("set xlabel\n");

  if (enarg) {
    if (*eps) {                 // ytitle
      if (symbolIsString(*eps))
        gp->sendf("set ylabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp->sendf("set ylabel\n");

  if (enarg) {
    if (*eps) {                 // ztitle
      if (symbolIsString(*eps))
        gp->sendf("set zlabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp->sendf("set zlabel\n");

  if (enarg) {
    if (*eps) {                 // title
      if (symbolIsString(*eps))
        gp->sendf("set title \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp->sendf("set title\n");

  float angle_x = 60;
  float angle_z = 30;

  if (enarg) {
    if (*eps) {                 // rotx
      if (symbolIsRealScalar(*eps)) {
        angle_x = fmodf(float_arg(*eps), 180);
        if (angle_x < 0)
          angle_x += 180;
      } else
        return cerror(NEED_REAL_SCAL, *eps);
    }
    --enarg;
    ++eps;
  }

  if (enarg) {
    if (*eps) {                 // rotz
      if (symbolIsRealScalar(*eps)) {
        angle_z = fmodf(float_arg(*eps), 360);
        if (angle_z < 0)
          angle_z += 360;
      } else
        return cerror(NEED_REAL_SCAL, *eps);
    }
    --enarg;
    ++eps;
  }

  if (enarg) {
    if (*eps) {                 // contours
      bool done = false;
      if (symbolIsRealScalar(*eps)) {
        if (symbolIsInteger(*eps)) {
          int32_t value = int_arg(*eps);
          if (value == 1) { // fully automatic contours
            // display automatically selected contours
            gp->sendf("set contour surface; set cntrparam levels auto;\n");
            done = true;
          } else if (value > 1) { // desired contour count
            gp->sendf("set contour surface; set cntrparam levels auto %d;\n",
                     value);
            done = true;
          } else if (value == 0) { // no contours
            gp->sendf("unset contour\n");
            done = true;
          } else
            return luxerror("Negative contour count %d not supported",
                            *eps, value);
        }
      }
      if (!done) {
        int32_t contours_count;
        Pointer contours;
        int32_t iq = lux_float(1, eps);
        if (numerical(iq, NULL, NULL, &contours_count, &contours) < 0)
          return LUX_ERROR;
        gp->sendf("set contour surface; set cntrparam levels discrete ");
        for (int i = 0; i < contours_count; ++i) {
          if (i)
            gp->sendf(",");
          gp->sendf("%f", *contours.f++);
        }
        gp->sendf("\n");
      }
    } else
      gp->sendf("unset contour;\n");
    --enarg;
    ++eps;
  } else
    gp->sendf("unset contour;\n");

  if (internalMode & 1)         // /flat
    gp->sendf("set view map\n");
  else {
    gp->sendf("unset view\n");
    gp->sendf("set view %f,%f\n", angle_x, angle_z);
  }

  if (internalMode & 2)         // logarithmic x axis
    gp->sendf("set logscale x\n");
  else
    gp->sendf("unset logscale x\n");

  if (internalMode & 4)         // logarithmic y axis
    gp->sendf("set logscale y\n");
  else
    gp->sendf("unset logscale y\n");

  gp->sendf("set tics; set auto fix\n");

  int ndata = 0;
  for (int i = 0; i < narg && i < 3; ++i)
    if (ps[i])
      ++ndata;
    else
      break;

  switch (ndata) {
  case 1:
    return lux_gnuplot_with_image(ndata, ps,
                                  "splot '-' binary array=(%d,%d) format=\"%s\" "
                                  "notitle with pm3d;");
  case 3:
    {
      StandardArguments standard_args;
      Pointer *data;
      loopInfo *info;
      int32_t myps[3];

      for (int i = 0; i < ndata; ++i)
        myps[i] = lux_float(1, &ps[i]);
      if (standard_args.set(ndata, myps, "iF*;iF*;iF>1,*", &data, &info) < 0)
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

          gp->data_remove();     // previous data file, if any
          std::ofstream& tmp = gp->data_ofstream();
          if (tmp.is_open()) {
            // std::cout << "Writing to " << gp->data_file_name() << std::endl;
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

            gp->sendf("splot '%s' binary nonuniform matrix using 1:2:3 notitle with pm3d;\n",
                     gp->data_file_name().c_str());
            gp->flush();
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
    return luxerror("Expected 1 or 3 data arguments, found %d", 0, ndata);
  }
  return LUX_OK;
}
REGISTER(gnuplot3d, s, gplot3d, 1, 10, ":::xtitle:ytitle:ztitle:title:rotx:rotz:contours:1flat:0lii:2loi:4lio:6loo");

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
