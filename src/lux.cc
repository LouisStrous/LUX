/* This is file lux.c.

Copyright 2013-2024 Louis Strous

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

/// The main function.
///
/// \param argc is the count of arguments with which the program was started
/// from the command line.
///
/// \param argv points to an array of the arguments.
///
/// \returns 0 if all is well, non-0 otherwise.
int main(int argc, char **argv)
{
  int do_main(int, char **);

  return do_main(argc, argv);
}

/// \page ext-lib External Libraries
///
/// LUX makes use of various external libraries.  If any of those libraries
/// aren't linked into your copy of LUX then you cannot use the that depend on
/// the missing libraries.
///
/// \section ext-lib-gsl GNU Scientific Library
///
/// Functions that are marked as shown below require the <a
/// href="https://www.gnu.org/software/gsl/">GNU Scientific Library</a> to be
/// compiled into LUX:
///
/// \warning Requires \ref ext-lib-gsl.
