/* This is file GnuPlot.hh.

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

#ifndef HAVE_GNUPLOT_HH_
#define HAVE_GNUPLOT_HH_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "luxdefs.hh"           // for Symboltype
#include <cstddef>              // for size_t
#include <cstdint>              // for int32_t
#include <fstream>              // for ofstream
#include <string>

/// A class that provides an interface for writing information to the
/// external "gnuplot" program.  The interface is based on gnuplot
/// 5.0.
class GnuPlot {
public:
  /// Constructor.  Starts the "gnuplot" program, if available.
  GnuPlot();

  /// Destructor.  Closes the interface, including the "gnuplot"
  /// process started by the constructor.  Removes any remaining
  /// temporary data files.
  ~GnuPlot();

  /// Sends formatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param format is a printf-style format string.
  ///
  /// \param ... are any number of data values.  They get formatted
  /// according to the format string.
  void sendf(const char* format, ...);

  /// Sends unformatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param data points to the beginning of the data to send.
  ///
  /// \param size is the number of bytes of data to send.
  void write(void* data, size_t size);

  /// Flush recently written input to the "gnuplot" program.
  void flush();

  /// Returns an ofstream to the current temporary file that can be
  /// used to store data for use by gnuplot.  The corresponding file
  /// name is available through #data_file_name.
  std::ofstream& data_ofstream();

  /// Returns the name of the current temporary file that can be used
  /// to store data for use by gnuplot.  The corresponding ofstream is
  /// available through #data_ofstream.
  std::string data_file_name();

  /// Remove the current temporary file, if any.
  void data_remove();

  /// Return the data type text for gnuplot that corresponds to the
  /// given LUX data type.
  ///
  /// \param lux_type is the LUX data type
  ///
  /// \returns the corresponding gnuplot data type
  static char const* gnuplot_type(Symboltype lux_type);

private:
  /// Initialize a temporary data file, if needed.
  bool initialize_data_file(void);

  /// A file pointer for the pipe to the "gnuplot" program.
  FILE* m_pipe;

  /// An ofstream to a temporary data file.
  std::ofstream m_data_ofstream;

  /// The name of the current temporary data file, or "" if there
  /// isn't one.
  std::string m_data_file_name;
};

#endif
