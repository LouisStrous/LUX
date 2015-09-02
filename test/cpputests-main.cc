#if HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_LIBCPPUTEST

# include "CppUTest/CommandLineTestRunner.h"

int main(int argc, char** argv)
{
  return CommandLineTestRunner::RunAllTests(argc, argv);
}

#else

int main(int argc, char** argv)
{
  return 77;       // skip these tests, because CppUTest not available
}

#endif
