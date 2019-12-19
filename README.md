# argcpp17
argcpp17 is a command line parser using c++17 features. It supports sub-commands, flags, mandatory and optional arguments and positional arguments.

A simple hello world application could be:
```cpp
#include <argcpp17.h>
int main(int argc, char **args)
{
  parser cmdline;
  cmdline.add_flag({"flag1"}, "description")
         .add_flag({"flag2", "f2"}, "another description");
  cmdline.parse(argc, args);
}
```
