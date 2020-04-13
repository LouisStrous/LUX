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
#include <vector>

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

  /// Assignment.
  GnuPlot& operator=(const GnuPlot& src);

  /// Sends formatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param format is a printf-style format string.
  ///
  /// \param ... are any number of data values.  They get formatted
  /// according to the format string.
  ///
  /// \returns a reference to the GnuPlot object.
  const GnuPlot& sendf(const char* format, ...) const;

  /// Sends text to the "gnuplot" program.  Don't forget to #flush
  /// when the command is complete.
  ///
  /// \param text is the text to send.
  ///
  /// \returns a reference to the GnuPlot object.
  const GnuPlot& send(const std::string& text) const;

  /// Sends text to the "gnuplot" program with a newline appended.
  /// Don't forget to #flush when the command is complete.
  ///
  /// \param text is the text to send.
  ///
  /// \returns a reference to the GnuPlot object.
  const GnuPlot& sendn(const std::string& text) const;

  /// Sends unformatted information to the "gnuplot" program.  Don't
  /// forget to #flush when the command is complete.
  ///
  /// \param data points to the beginning of the data to send.
  ///
  /// \param size is the number of bytes of data to send.
  ///
  /// \returns a reference to the GnuPlot object.
  const GnuPlot& write(void* data, size_t size) const;

  /// Flush recently written input to the "gnuplot" program.
  ///
  /// \returns a reference to the GnuPlot object.
  const GnuPlot& flush() const;

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

  /// Returns the next available datablock index.  The first returned
  /// index is 1, and the index is incremented for each call.  It gets
  /// reset by #discard_datablocks.
  ///
  /// The LUX \c gplot command uses datablocks with names equal to
  /// '$LUX' followed by the datablock index.
  uint32_t next_available_datablock_index();

  /// Returns the current datablock index, or 0 if none has been
  /// reserved yet through a call to #next_available_datablock_index
  /// since the creation of this instance or since the last call to
  /// #discard_datablocks.
  uint32_t current_datablock_index() const;

  /// Construct formatted text and remember the text for the current
  /// datablock.
  ///
  /// \param format is a printf-style format string.
  ///
  /// \param ... represents any number of data values.  They get
  /// formatted according to the format string.
  void remember_for_current_datablock(const char* format, ...);

  /// Remember the text for the current datablock.
  ///
  /// \param text is the text to remember.
  void remember_for_current_datablock(const std::string& text);

  bool have_datablock_plot_elements() const;

  /// Construct a gnuplot 'plot' command based on the remembered data
  /// blocks.
  std::string construct_plot_command() const;

  /// Construct a gnuplot 'splot' command for a line plot based on the
  /// remembered data blocks.
  std::string construct_splot_command() const;

  /// Issues gnuplot commands to discard the remembered '$LUX'
  /// datablocks, forgets the gnuplot commands associated with that
  /// datablock, and resets the index.
  void discard_datablocks();

  /// Sets the verbosity level.  Currently only 0 and non-0 are
  /// distinguished.
  ///
  /// \param value is the new verbosity level.
  ///
  /// \returns the previous verbosity level.
  bool set_verbosity(bool value);

private:
  /// Initialize a temporary data file, if needed.
  bool initialize_data_file(void);

  /// Construct a plot or splot command.
  std::string construct_plot_or_splot_command(std::string head) const;

  /// A file pointer for the pipe to the "gnuplot" program.
  FILE* m_pipe;

  /// An ofstream to a temporary data file.
  std::ofstream m_data_ofstream;

  /// The name of the current temporary data file, or "" if there
  /// isn't one.
  std::string m_data_file_name;

  /// The number of data blocks in use for gnuplot's 'plot' command.
  uint32_t m_datablock_count;

  /// The verbosity level
  bool m_verbosity;

  typedef std::vector<std::string> DatablockBackendCollection;

  DatablockBackendCollection m_datablock_plot_elements;
};

#endif
