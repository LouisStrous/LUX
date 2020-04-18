#ifndef EVERYWHERE_HH_
# define EVERYWHERE_HH_

# ifdef __cplusplus             // c++ only
#  include <cstring>            // for memset, memcpy
#  include <type_traits>        // for is_standard_layout

#  ifdef memset
#   error A macro 'memset' is unexpectedly already defined!
#  else

template<class T>
void* checked_memset(T* dest, int ch, std::size_t count)
{
  if (std::is_standard_layout<T>::value) {
    return memset(reinterpret_cast<void*>(dest), ch, count);
  }
  // no else branch, so a compiler error is generated if T is not a
  // standard-layout type.
}

#   define memset(dest,ch,count) checked_memset((dest),(ch),(count))
#  endif // ifdef memset else


#  ifdef memcpy
#   error A macro 'memcpy' is unexpectedly already defined!
#  else

template<class T, class U>
void* checked_memcpy(T* dest, const U* src, std::size_t count)
{
  if (std::is_standard_layout<T>::value
      && std::is_standard_layout<U>::value) {
    return memcpy(reinterpret_cast<void*>(dest),
                  reinterpret_cast<const void*>(src), count);
  }
  // no else branch, so a compiler error is generated if T or U are
  // not standard-layout types.
}

#   define memcpy(dest,src,count) checked_memcpy((dest),(src),(count))
#  endif // ifdef memcpy else


#  ifdef memmove
#   error A macro 'memmove' is unexpectedly already defined!
#  else

template<class T, class U>
void* checked_memmove(T* dest, const U* src, std::size_t count)
{
  if (std::is_standard_layout<T>::value
      && std::is_standard_layout<U>::value) {
    return memmove(reinterpret_cast<void*>(dest),
                  reinterpret_cast<const void*>(src), count);
  }
  // no else branch, so a compiler error is generated if T or U are
  // not standard-layout types.
}

#   define memmove(dest,src,count) checked_memmove((dest),(src),(count))
#  endif // ifdef memmove else
# endif // ifdef __cplusplus
#endif // EVERYWHERE_HH_
