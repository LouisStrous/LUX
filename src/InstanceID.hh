#ifndef INCLUDE_INSTANCEID
#define INCLUDE_INSTANCEID

#include <string>

class InstanceID
{
public:
#  if NDEBUG                    // no debug
  void identify(const std::string& id = "") const { }

#  else  // debug
  InstanceID()
    : m_instance_id(++s_instance_id)
  { }

  void
  identify(const std::string& id = "") const
  {
    printf("instance %lu", m_instance_id);
    if (!id.empty())
      printf(" (%s)\n", id.c_str());
    else
      putchar('\n');
  }

private:
  static size_t s_instance_id;
  size_t m_instance_id;
#  endif
};

#endif
