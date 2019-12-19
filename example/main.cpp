#include <argcpp17.h>
#include <iostream>

int main(int argc, char **args)
{
    parser p;


    p.add_subcommand("sub1", "first subcommand")
        .add_flag({"flag1", "f1"}, "subcommand flag");
    p.add_subcommand("sub2", "second subcommand");

    p.add_flag({"flag1", "f1"}, "first flag")
     .add_flag({"flag2", "f2"}, "second flag")
     .add_optional_argument({"option", "o"}, "optional value")
     .add_mandatory_argument({"mandatory", "m"}, "mandatory value")
     .add_positional("pos1", "first positional")
     .add_positional("pos2", "second positional");

    p.parse(argc, args);

    return 0;
}