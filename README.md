![Build Status](https://github.com/cemoktra/argcpp17/workflows/CPP/badge.svg)

# argcpp17
argcpp17 is a command line parser using c++17 features. It supports sub-commands, flags, mandatory and optional arguments and positional arguments.

A simple hello world application could be:
```cpp
#include <argcpp17.h>
int main(int argc, char **args)
{
  parser cmdline;
  cmdline.add_flag({"flag1"}, "description")
         .add_flag({"flag2", "f2"}, "another description")
         .add_option({"option", "o"}, "some double value");
  cmdline.parse(argc, args);
  cmdline.get_flag({"f2"});
  cmdline.get_value<double>({"option"});
}
```
