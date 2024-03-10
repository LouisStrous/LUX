#ifndef TYPES_HH__
#  define TYPES_HH__

#include <cstdint>              // for int32_t and friends

#include "luxdefs.hh"

/// The data type of the count of arguments to a LUX subroutine or function
/// call.  It won't be used to hold negative numbers but may for technical
/// reasons be implemented as a signed integer.
using ArgumentCount = int32_t;

/// The integer data type of the size of a dimension of a LUX array.  Also used
/// for an index into a C/C++ array.  It won't be used to hold negative numbers
/// but may for technical reasons be implemented as a signed integer.
using Size = int32_t;

/// The data type of a proxy for a LUX symbol.
using Symbol = int32_t;

#endif
