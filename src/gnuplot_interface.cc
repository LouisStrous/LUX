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

enum GplotStatus
  {
   gplot,
   goplot,
   gaplot
  };

static GnuPlot gp;

int32_t lux_gcommand(int32_t narg, int32_t ps[])
{
  if (narg > 1 && ps[1]) {
    gp.set_verbosity(int_arg(ps[1]));
  }
  if (symbolIsString(ps[0])) {
    char* command = string_value(ps[0]);
    if (!strncmp(command, "exit", 4)) {
      gp = GnuPlot();           // get a new one; closes the old one
    } else {
      gp.sendn(string_value(ps[0])).flush();
    }
  } else if (ps[0]) {
    cerror(NEED_STR, ps[0]);
  }
  return LUX_OK;
}
REGISTER(gcommand, s, gcommand, 1, 2, ":verbose");

int32_t lux_gterm(int32_t narg, int32_t ps[])
{
  int terminal_number = int_arg(ps[0]);
  if (terminal_number < 0)
    luxerror("Need a nonnegative argument.\n", ps[0]);
  gp.sendf("set term @GPVAL_TERM %d\n", terminal_number)
    .flush();
  return LUX_OK;
}
REGISTER(gterm, s, gterm, 1, 1, "");

/// Plots or overplots data through gnuplot.  This function gets
/// called for the LUX "gplot", "goplot", and "gaplot" subroutines.
/// See there for details of the LUX arguments.
///
/// \par status says what plotting mode is used.
///
/// \par narg is the number of LUX arguments
///
/// \par ps is an array of LUX arguments.
///
/// \returns #LUX_OK for success, #LUX_ERROR for failure.
int32_t lux_gplot_backend(GplotStatus status, int32_t narg, int32_t ps[])
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
  // For gaplot:
  //
  //   If there is data, then send it to an unused gnuplot data block, and
  //   remember the 'plot element' for the data block, but do not plot anything.
  //
  //   If there is no data, then send a plot command for all data blocks and
  //   plot elements to gnuplot.
  //
  // All setting up of the plot (e.g., defining titles) is done for
  // gplot and does not need to be remembered for the data block.

  int32_t datablock_index;
  bool first_gaplot = false;

  switch (status) {
  case GplotStatus::gplot:
    gp.discard_datablocks();
    datablock_index = gp.next_available_datablock_index();
    break;
  case GplotStatus::goplot:
    datablock_index = gp.next_available_datablock_index();
    break;
  case GplotStatus::gaplot:
    first_gaplot = !gp.have_datablock_plot_elements();
    if (narg)                   // not emitting saved elements
      datablock_index = gp.next_available_datablock_index();
    break;
  }

  // the first three arguments indicate <x>, <y>, <z>
  int32_t enarg = narg - 3;
  int32_t* eps = ps + 3;

  if (enarg < 0)
    enarg = 0;

  int linetype = 0;
  if (enarg) {
    if (*eps) {               // linetype
      linetype = int_arg(*eps);
    }
    --enarg;
    ++eps;
  }

  int pointtype = 0;
  if (enarg) {
    if (*eps) {               // pointtype
      pointtype = int_arg(*eps);
    }
    --enarg;
    ++eps;
  }

  int dashtype = 0;
  if (enarg) {
    if (*eps) {               // dashtype
      dashtype = int_arg(*eps);
    }
    --enarg;
    ++eps;
  }

  char* color = 0;
  if (enarg) {
    if (*eps) {                 // color
      if (symbolIsString(*eps)) {
        color = string_value(*eps);
      } else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
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

  int breaks_count = 0;
  Pointer breaks;
  if (enarg) {
    if (*eps) {                 // breaks
      int iq = lux_long(1, eps);
      if (iq == LUX_ERROR)
        return LUX_ERROR;
      if (numerical(iq, NULL, NULL, &breaks_count, &breaks)
          == LUX_ERROR)
        return LUX_ERROR;
    }
    --enarg;
    ++eps;
  }

  gp.sendf("set xtics; set ytics;\n");

  bool done = false;
  if (status == GplotStatus::gplot || first_gaplot) {
    if (enarg) {
      if (*eps) {               // xtitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp.sendf("set xlabel \"%s\"\n", text);
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
      gp.send("set xlabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // ytitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp.sendf("set ylabel \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp.send("set ylabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // ztitle
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp.sendf("set zlabel \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp.send("set zlabel\n");

    done = false;
    if (enarg) {
      if (*eps) {               // title
        if (symbolIsString(*eps)) {
          char* text = string_value(*eps);
          if (*text) {
            gp.sendf("set title \"%s\"\n", text);
            done = 1;
          }
        } else
          return cerror(NEED_STR, *eps);
      }
      --enarg;
      ++eps;
    }
    if (!done)
      gp.send("set title\n");

    extern float plims[];

    // prevent gnuplot widening the plot to the nearest round values
    gp.send("set auto fix\n");

    // set the distance between the data and the axes to 5% of the
    // graph size if no explicit plot limits are given, and to 0
    // otherwise
    gp.send("set offsets graph ");
    if (plims[0] == plims[1] && !plims[0])
      gp.send("0.05, graph 0.05");
    else
      gp.send("0, graph 0");
    gp.send(", graph ");
    if (plims[2] == plims[3] && !plims[2])
      gp.send("0.05, graph 0.05");
    else
      gp.send("0, graph 0");
    gp.send("\n");

    {
      std::ostringstream oss;
      oss << "set xrange [";
      if (plims[0])
        oss << plims[0];
      else
        oss << "*";
      oss << ":";
      if (plims[1])
        oss << plims[1];
      else
        oss << "*";
      oss << "]\n";
      gp.send(oss.str());
    }

    {
      std::ostringstream oss;
      oss << "set yrange [";
      if (plims[2])
        oss << plims[2];
      else
        oss << "*";
      oss << ":";
      if (plims[3])
        oss << plims[3];
      else
        oss << "*";
      oss << "]\n";
      gp.send(oss.str());
    }

    {
      std::ostringstream oss;
      oss << "set zrange [";
      if (plims[2])
        oss << plims[2];
      else
        oss << "*";
      oss << ":";
      if (plims[3])
        oss << plims[3];
      else
        oss << "*";
      oss << "]\n";
      gp.send(oss.str());
    }

    if (internalMode & 2)         // logarithmic x axis
      gp.send("set logscale x\n");
    else
      gp.send("unset logscale x\n");

    if (internalMode & 4)         // logarithmic y axis
      gp.send("set logscale y\n");
    else
      gp.send("unset logscale y\n");

    if (internalMode & 8)         // logarithmic z axis
      gp.send("set logscale z\n");
    else
      gp.send("unset logscale z\n");

    if (internalMode & 14)
      gp.send("set format \"%g\"\n");
  }

  // transform the coordinates to type double
  int32_t myps[3];
  int32_t ndata = 0;
  for (int32_t i = 0; i < 3 && i < narg; ++i) {
    if (ps[i]) {
      myps[i] = lux_double(1, &ps[i]);
      ++ndata;
    } else
      break;
  }

  if (narg) {
    Pointer *data;
    LoopInfo *info;

    StandardArguments sa(ndata, myps, "iD*;iD&?;iD&?", &data, &info);
    if (sa.result() < 0)
      return LUX_ERROR;

    // send the data to a gnuplot data block
    gp.sendf("$LUX%d << EOD\n", datablock_index);
    switch (ndata) {
    case 1:
      for (int i = 0; i < info[0].nelem; ++i) {
        gp.sendf("%.10g\n", *data[0].d++);
        if (breaks_count && i == *breaks.i32) {
          gp.sendf("\n");       // empty line means break in plot
          ++breaks.i32;
          --breaks_count;
        }
      }
      break;
    case 2:
      for (int i = 0; i < info[0].nelem; ++i) {
        gp.sendf("%.10g %.10g\n", *data[0].d++, *data[1].d++);
        if (breaks_count && i == *breaks.i32) {
          gp.sendf("\n");       // empty line means break in plot
          ++breaks.i32;
          --breaks_count;
        }
      }
      break;
    case 3:
      for (int i = 0; i < info[0].nelem; ++i) {
        gp.sendf("%.10g %.10g %.10g\n", *data[0].d++, *data[1].d++,
                 *data[2].d++);
        if (breaks_count && i == *breaks.i32) {
          gp.sendf("\n");       // empty line means break in plot
          ++breaks.i32;
          --breaks_count;
        }
      }
      break;
    }
    gp.send("EOD\n");
  }

  if (narg) {
    // what to do with linetype, pointtype, dashtype:
    // if none are specified, or if only linetype and/or dashtype is
    // specified, then use "with lines"
    // if only pointtype is specified, then use "with points"
    // if pointtype and one or both of linetype and dashtype are specified,
    // then use "with linespoint"

    std::string with_text;
    if (pointtype > 0) {
      if (linetype > 0 || dashtype > 0)
        with_text = "linespoints";
      else
        with_text = "points";
    } else if (pointtype < 0) {
      with_text = "points";
    } else {
      with_text = "lines";
    }

    std::string type_text;
    {
      std::ostringstream oss;
      if (linetype > 0)
        oss << " linetype " << linetype;
      if (pointtype > 0)
        oss << " pointtype " << pointtype;
      else if (pointtype < 0)
        oss << " pointtype 6 pointsize " << pow(10, pointtype*0.1) << " ";
      if (dashtype > 0)
        oss << " dashtype " << dashtype;
      type_text = oss.str();
    }

    extern int current_pen;

    std::ostringstream using_oss;
    using_oss << "using ";

    switch (ndata) {
    case 1:                       // y only
      using_oss << "1";
      break;
    case 2:                       // x,y
      using_oss << "1:2";
      break;
    case 3:                       // x,y,z
      using_oss << "1:2:3";
      break;
    }

    using_oss << " with " << with_text
              << " lw " << current_pen;

    if (!type_text.empty())
      using_oss << " " << type_text;

    if (color)
      using_oss << " linecolor " << color;

    using_oss << " title \"" << (legend? legend: "") << "\"";

    gp.remember_for_current_datablock(using_oss.str());
  }

  if (status != GplotStatus::gaplot || !narg) {
    if (ndata == 3)
      gp.sendn(gp.construct_splot_command());
    else
      gp.sendn(gp.construct_plot_command());
  }

  gp.flush();

  if (!narg && status == GplotStatus::gaplot) { // gplot
    gp.discard_datablocks();
  }

  return LUX_OK;
}

/// gplot[,<x>],<y>[,<z>]
///      [,linetype=<ltype>,pointtype=<ptype>,dashtype=<dtype>
///      ,color=<color>
///      ,xtitle=<xtitle>,ytitle=<ytitle>,ztitle=<ztitle>,title=<title>
///      ,legend=<legend>,breaks=<breaks>]
///      [,/lii,/lio,/loi,/loo]
///      [,/liii,/lioi,/loii,/looi,/liio,/lioo,/loio,/looo]
///
/// Plots data through gnuplot.
///
/// <x>, <y>, and <z> provide the x, y, and z coordinates of the data
/// points.  They must have the same dimensional structure.
///
/// <ltype> is an integer that specifies the type of line to draw.
///
/// <ptype> is an integer that specifies the type of points to draw.
///
/// <dtype> is an integer that specified the type of dashes to draw.
///
/// <color> is text that identifies the color to use
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
/// <breaks> is a sorted list of 0-based indexes of points after which the graph
/// should have a break: the segment to the next point is not drawn.
///
/// /lii asks for linear x and y axes; /lio asks for linear x and
/// logarithmic y axes; /loi asks for logarithmic x and linear y axes;
/// and /loo asks for logarithmic x and y axes.  /liii through /looo
/// likewise specify linear or logarithmic x, y, and z axes.
int32_t lux_gplot(int32_t narg, int32_t ps[]) {
  return lux_gplot_backend(GplotStatus::gplot, narg, ps);
}
REGISTER(gplot, s, gplot, 1, 13, ":::linetype:pointtype:dashtype:color:legend:breaks:xtitle:ytitle:ztitle:title:0lii:2loi:4lio:6loo:0liii:2loii:4lioi:6looi:8liio:10loio:12lioo:14looo");

/// goplot[,<x>],<y>[,linetype=<ltype>,pointtype=<ptype>,dashtype=<dtype>,
///     color=<color>,legend=<legend>]
///
/// Add another curve to the previous plot through gnuplot.
///
/// <x> and <y> provide the x and y coordinates of the data points.
/// They must have the same dimensional structure.
///
/// <ltype> is an integer that specifies the type of line to draw.
///
/// <ptype> is an integer that specifies the type of points to draw.
///
/// <dtype> is an integer that specified the type of dashes to draw.
///
/// <color> is the color to use.
///
/// <legend> is the text to display in the legend.
///
/// <breaks> is a sorted list of 0-based indexes of points after which the graph
/// should have a break: the segment to the next point is not drawn.
int32_t lux_goplot(int32_t narg, int32_t ps[]) {
  return lux_gplot_backend(GplotStatus::goplot, narg, ps);
}
REGISTER(goplot, s, goplot, 1, 9, ":::linetype:pointtype:dashtype:color:legend:breaks");

int32_t lux_gaplot(int32_t narg, int32_t ps[]) {
  return lux_gplot_backend(GplotStatus::gaplot, narg, ps);
}
REGISTER(gaplot, s, gaplot, 0, 13, ":::linetype:pointtype:dashtype:color:legend:breaks:xtitle:ytitle:ztitle:title:0lii:2loi:4lio:6loo:0liii:2loii:4lioi:6looi:8liio:10loio:12lioo:14looo");

int32_t lux_gnuplot_with_image(int32_t narg, int32_t ps[],
                               std::string gnuplot_command_fmt) {
  Pointer *data;
  LoopInfo *info;

  StandardArguments sa(narg, ps, "i>B>1,>1", &data, &info);
  if (sa.result() < 0)
    return LUX_ERROR;

  const char* gnuplot_type = GnuPlot::gnuplot_type(info[0].type);
  if (!gnuplot_type)
    return cerror(ILL_TYPE, ps[0]);

  gp.sendf(gnuplot_command_fmt.c_str(), info[0].dims[0], info[0].dims[1],
           gnuplot_type)
    .send("\n")
    .write(&data[0].ui8[0], info[0].nelem*lux_type_size[info[0].type])
    .flush();

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

  if (enarg) {
    if (*eps) {                 // xtitle
      if (symbolIsString(*eps))
        gp.sendf("set xlabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp.send("set xlabel\n");

  if (enarg) {
    if (*eps) {                 // ytitle
      if (symbolIsString(*eps))
        gp.sendf("set ylabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp.send("set ylabel\n");

  if (enarg) {
    if (*eps) {                 // ztitle
      if (symbolIsString(*eps))
        gp.sendf("set zlabel \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp.send("set zlabel\n");

  if (enarg) {
    if (*eps) {                 // title
      if (symbolIsString(*eps))
        gp.sendf("set title \"%s\"\n", string_value(*eps));
      else
        return cerror(NEED_STR, *eps);
    }
    --enarg;
    ++eps;
  } else
    gp.send("set title\n");

  double angle_x = 60;
  double angle_z = 30;

  if (enarg) {
    if (*eps) {                 // rotx
      if (symbolIsRealScalar(*eps)) {
        angle_x = fmodf(double_arg(*eps), 180);
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
        angle_z = fmodf(double_arg(*eps), 360);
        if (angle_z < 0)
          angle_z += 360;
      } else
        return cerror(NEED_REAL_SCAL, *eps);
    }
    --enarg;
    ++eps;
  }

  bool have_contour_labels = false;
  if (enarg) {
    if (*eps) {                 // contours
      bool done = false;
      gp.send("set contour base; unset surface; set view map;\n");
      if (symbolIsRealScalar(*eps)) {
        if (symbolIsInteger(*eps)) {
          int32_t value = int_arg(*eps);
          if (value == 1) { // fully automatic contours
            // display automatically selected contours
            gp.send("set cntrparam levels auto;\n");
            have_contour_labels = true;
            done = true;
          } else if (value > 1) { // desired contour count
            gp.sendf("set cntrparam levels auto %d;\n", value);
            have_contour_labels = true;
            done = true;
          } else if (value == 0) { // no contours
            gp.send("unset contour\n");
            done = true;
          } else
            return luxerror("Negative contour count %d not supported",
                            *eps, value);
        }
      }
      if (!done) {
        int32_t contours_count;
        Pointer contours;
        int32_t iq = lux_double(1, eps);
        if (numerical(iq, NULL, NULL, &contours_count, &contours) < 0)
          return LUX_ERROR;
        gp.send("set cntrparam levels discrete ");
        for (int i = 0; i < contours_count; ++i) {
          if (i)
            gp.sendf(",");
          gp.sendf("%g", *contours.d++);
        }
        gp.send("\n");
        have_contour_labels = true;
      }
    } else
      gp.send("unset contour;\n");
    --enarg;
    ++eps;
  } else
    gp.send("unset contour;\n");

  if (!have_contour_labels) {
    if (internalMode & 1)         // /flat
      gp.send("set view map\n");
    else {
      gp.send("unset view\n");
      gp.sendf("set view %f,%f\n", angle_x, angle_z);
    }

    if (internalMode & 2)         // logarithmic x axis
      gp.send("set logscale x\n");
    else
      gp.send("unset logscale x\n");

    if (internalMode & 4)         // logarithmic y axis
      gp.send("set logscale y\n");
    else
      gp.send("unset logscale y;\n");

    if (internalMode & 8) {       // logarithmic z axis
      gp.send("set logscale z; set logscale cb;\n");
    } else {
      gp.send("unset logscale z; unset logscale cb;\n");
    }
  }

  // gp.send("set tics; set auto fix;\n");

  int ndata = 0;
  for (int i = 0; i < narg && i < 3; ++i)
    if (ps[i])
      ++ndata;
    else
      break;

  switch (ndata) {
  case 1:
    return lux_gnuplot_with_image
      (ndata, ps,
       "splot '-' binary array=(%d,%d) format=\"%s\" "
       "notitle with pm3d;");
  case 3:
    {
      Pointer *data;
      LoopInfo *info;
      int32_t myps[3];

      // gnuplot expects 32-bit IEEE floating-point values
      for (int i = 0; i < ndata; ++i)
        myps[i] = lux_float(1, &ps[i]);

      StandardArguments sa(ndata, myps, "iF*;iF*;iF>1,*", &data, &info);
      if (sa.result()< 0)
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
            // std::cout << "Writing to " << gp.data_file_name() << std::endl;
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

            if (have_contour_labels)
              gp.sendf("splot '%s' binary nonuniform matrix using 1:2:3 notitle with lines;\n",
                     gp.data_file_name().c_str())
              .flush();
            else
              gp.sendf("splot '%s' binary nonuniform matrix using 1:2:3 notitle with pm3d;\n",
                     gp.data_file_name().c_str())
              .flush();
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
REGISTER(gnuplot3d, s, gplot3d, 1, 10, ":::xtitle:ytitle:ztitle:title:rotx:rotz:contours:1flat:0lii:2loi:4lio:6loo:0liii:2loii:4lioi:6looi:8liio:10loio:12lioo:14looo");

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
  std::ostringstream oss;

  if (internalMode & 1)       // /equalxy
    // "set view equal xy" leads to plots with no contours
    oss << "set size ratio -1; ";
  oss << "set view map; ";
  if (internalMode & 2)         // /image
    oss << "set surface; set contour surface; ";
  else
    oss << "unset surface; set contour base; ";
  oss << "set cntrparam cubicspline; "
    "splot '-' binary array=(%d,%d) format=\"%s\" title '' ";
  if (internalMode & 2)
    oss << "with pm3d;"; // with image => no contours
  else
    oss << "with lines;";
  oss << "reset; ";

  return lux_gnuplot_with_image(narg, ps, oss.str());
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

  plot '-' binary array=(30) format="%double" with lines; => y only
  plot '-' binary record=(30) format="%double" using 1:2 with lines => x and y

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
