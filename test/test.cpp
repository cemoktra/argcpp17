#include <gtest/gtest.h>
#include <argcpp17.h>


static const std::string KEY = "my_key";
static const std::string ABBR = "my_abbr";
static const std::string DESC = "my_desc";
static const std::string ANOTHER_KEY = "another_key";
static const std::string VALUE = "my_value";


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}


TEST(keyword_test, constructor_default)
{
    keyword kw;
    EXPECT_EQ(kw.get_key(), "");
    EXPECT_EQ(kw.get_abbreviation(), std::nullopt);
}

TEST(keyword_test, constructor_params)
{
    {
        keyword kw (KEY);
        EXPECT_EQ(kw.get_key(), KEY);
        EXPECT_EQ(kw.get_abbreviation(), std::nullopt);
    }

    {
        keyword kw (KEY, ABBR);
        EXPECT_EQ(kw.get_key(), KEY);
        EXPECT_EQ(kw.get_abbreviation(), ABBR);
    }

    {
        keyword kw = { KEY, ABBR };
        EXPECT_EQ(kw.get_key(), KEY);
        EXPECT_EQ(kw.get_abbreviation(), ABBR);
    }
}

TEST(keyword_test, copy_constructor)
{
    keyword kw1 (KEY);
    keyword kw2 (kw1);
    EXPECT_EQ(kw2.get_key(), KEY);
    EXPECT_EQ(kw2.get_abbreviation(),  std::nullopt);
    EXPECT_EQ(kw2.get_key(), kw1.get_key());
    EXPECT_EQ(kw2.get_abbreviation(), kw1.get_abbreviation());
}

TEST(keyword_test, operator_equal_keyword)
{
    keyword kw1 (KEY, ABBR);
    {
        keyword kw2 (KEY);
        EXPECT_TRUE(kw1 == kw2);
    }
    {
        keyword kw2 (ABBR);
        EXPECT_TRUE(kw1 == kw2);
    }
    {
        keyword kw2 (ANOTHER_KEY, ABBR);
        EXPECT_TRUE(kw1 == kw2);
    }
    {
        keyword kw2 (ANOTHER_KEY);
        EXPECT_FALSE(kw1 == kw2);
    }  
}

TEST(keyword_test, operator_equal_string)
{
   
    keyword kw1 (KEY, ABBR);
    EXPECT_TRUE(kw1 == KEY);
    EXPECT_TRUE(kw1 == ABBR);
    EXPECT_FALSE(kw1 == ANOTHER_KEY);
}

class argument_test : public ::testing::Test {
public:
    argument_test()
        : sut({KEY, ABBR}, DESC)
    {};
    ~argument_test() = default;

    class derived_argument : public argument
    {
    public:
        derived_argument(const keyword& key, const std::string& description) : argument(key, description) {};
        derived_argument(const derived_argument& rhs) : argument(rhs) {};

        using argument::mark_parsed;
    };

protected:
    derived_argument sut;
};

TEST_F(argument_test, constructor_params)
{
    EXPECT_FALSE(sut.is_parsed());
    EXPECT_EQ(sut.get_key().get_key(), KEY);
    EXPECT_EQ(sut.get_key().get_abbreviation(), ABBR);
    EXPECT_EQ(sut.get_description(), DESC);
}

TEST_F(argument_test, copy_constructor)
{
    derived_argument copy_sut(sut);
    EXPECT_EQ(copy_sut.is_parsed(), sut.is_parsed());
    EXPECT_EQ(copy_sut.get_key().get_key(), sut.get_key().get_key());
    EXPECT_EQ(copy_sut.get_key().get_abbreviation(), sut.get_key().get_abbreviation());
    EXPECT_EQ(copy_sut.get_description(), sut.get_description());
}

TEST_F(argument_test, mark_parsed)
{
    EXPECT_FALSE(sut.is_parsed());
    sut.mark_parsed();
    EXPECT_TRUE(sut.is_parsed());
}

TEST_F(argument_test, operator_equal_keyword)
{
    EXPECT_TRUE(sut == keyword(KEY, ABBR));
    EXPECT_TRUE(sut == keyword(KEY));
    EXPECT_TRUE(sut == keyword(ABBR));    
    EXPECT_TRUE(sut == keyword(ANOTHER_KEY, ABBR));
    EXPECT_FALSE(sut == keyword(ANOTHER_KEY));
}

TEST_F(argument_test, operator_equal_string)
{
    EXPECT_TRUE(sut == KEY);
    EXPECT_TRUE(sut == ABBR);
    EXPECT_FALSE(sut == ANOTHER_KEY);
}


class flag_test : public ::testing::Test {
public:
    flag_test()
        : sut({KEY, ABBR}, DESC)
    {};
    ~flag_test() = default;

    class derived_flag : public flag
    {
    public:
        derived_flag(const keyword& key, const std::string& description) : flag(key, description) {};
        derived_flag(const derived_flag& rhs) : flag(rhs) {};

        using argument::mark_parsed;
    };

protected:
    derived_flag sut;
};

TEST_F(flag_test, is_set)
{
    EXPECT_FALSE(sut.is_set());
    sut.mark_parsed();
    EXPECT_TRUE(sut.is_set());
}

TEST(subcommand_test, get_parser)
{
    subcommand<parser> sub(KEY, DESC);
    auto p = sub.get_parser();
    EXPECT_EQ(p.subcommands(), 0);
    EXPECT_EQ(p.flags(), 0);
    EXPECT_EQ(p.mandatories(), 0);
    EXPECT_EQ(p.optionals(), 0);
    EXPECT_EQ(p.positionals(), 0);
}

class optional_test : public ::testing::Test {
public:
    optional_test()
        : sut({KEY, ABBR}, DESC)
    {};
    ~optional_test() = default;

    class derived_optional : public optional_argument
    {
    public:
        derived_optional(const keyword& key, const std::string& description) : optional_argument(key, description) {};
        derived_optional(const derived_optional& rhs) : optional_argument(rhs) {};

        using argument::update_value;
    };

protected:
    derived_optional sut;
};

TEST_F(optional_test, constructor_param)
{
    EXPECT_EQ(sut.value<std::string>(), std::nullopt);
}

TEST_F(optional_test, update_value)
{
    EXPECT_EQ(sut.value<std::string>(), std::nullopt);
    sut.update_value(VALUE);
    EXPECT_EQ(sut.value<std::string>(), VALUE);
}

class mandatory_test : public ::testing::Test {
public:
    mandatory_test()
        : sut({KEY, ABBR}, DESC)
    {};
    ~mandatory_test() = default;

    class derived_mandatory : public mandatory_argument
    {
    public:
        derived_mandatory(const keyword& key, const std::string& description) : mandatory_argument(key, description) {};
        derived_mandatory(const derived_mandatory& rhs) : mandatory_argument(rhs) {};

        using argument::update_value;
    };

protected:
    derived_mandatory sut;
};

TEST_F(mandatory_test, constructor_param)
{
    EXPECT_TRUE(sut.value<std::string>().empty());
}

TEST_F(mandatory_test, update_value)
{
    EXPECT_TRUE(sut.value<std::string>().empty());
    sut.update_value(VALUE);
    EXPECT_FALSE(sut.value<std::string>().empty());
    EXPECT_EQ(sut.value<std::string>(), VALUE);
}


class positional_test : public ::testing::Test {
public:
    positional_test()
        : sut(KEY, DESC)
    {};
    ~positional_test() = default;

    class derived_positional : public positional_argument
    {
    public:
        derived_positional(const std::string& name, const std::string& description) : positional_argument(name, description) {};
        derived_positional(const derived_positional& rhs) : positional_argument(rhs) {};

        using argument::update_value;
    };

protected:
    derived_positional sut;
};


TEST_F(positional_test, constructor_param)
{
    EXPECT_EQ(sut.get_key().get_key(), KEY);
    EXPECT_EQ(sut.get_key().get_abbreviation(), std::nullopt);
    EXPECT_TRUE(sut.value<std::string>().empty());
}

TEST_F(positional_test, update_value)
{
    EXPECT_TRUE(sut.value<std::string>().empty());
    sut.update_value(VALUE);
    EXPECT_FALSE(sut.value<std::string>().empty());
    EXPECT_EQ(sut.value<std::string>(), VALUE);
}

class parser_test : public ::testing::Test {
public:
    parser_test() = default;
    ~parser_test() = default;

    class derived_parser : public parser
    {
    public:
        derived_parser() = default;
        derived_parser(const derived_parser& rhs) = default;
        ~derived_parser() = default;

        using parser::parse_vector;
    };

protected:
    derived_parser sut;
};

TEST_F(parser_test, create_subcommands)
{
    sut.add_subcommand(KEY, DESC);
    EXPECT_EQ(sut.subcommands(), 1);
    sut.add_subcommand(ANOTHER_KEY, DESC);
    EXPECT_EQ(sut.subcommands(), 2);
    EXPECT_THROW(sut.add_subcommand(KEY, DESC), argcpp17_exception);
}


TEST_F(parser_test, create_flags)
{
    sut.add_flag({KEY, ABBR}, DESC);
    EXPECT_EQ(sut.flags(), 1);
    EXPECT_THROW(sut.add_flag({ANOTHER_KEY, ABBR}, DESC), argcpp17_exception);
}

TEST_F(parser_test, create_argument)
{
    sut.add_argument({KEY, ABBR}, DESC);
    EXPECT_EQ(sut.optionals(), 1);
    EXPECT_EQ(sut.mandatories(), 0);
    sut.add_argument({ANOTHER_KEY}, DESC, false);
    EXPECT_EQ(sut.optionals(), 1);
    EXPECT_EQ(sut.mandatories(), 1);
    EXPECT_THROW(sut.add_argument({KEY}, DESC, false), argcpp17_exception);
}

TEST_F(parser_test, create_optional)
{
    sut.add_optional_argument({KEY, ABBR}, DESC);
    EXPECT_EQ(sut.optionals(), 1);
    EXPECT_THROW(sut.add_optional_argument({ANOTHER_KEY, ABBR}, DESC), argcpp17_exception);
}

TEST_F(parser_test, create_mandatory)
{
    sut.add_mandatory_argument({KEY, ABBR}, DESC);
    EXPECT_EQ(sut.mandatories(), 1);
    EXPECT_THROW(sut.add_mandatory_argument({ANOTHER_KEY, ABBR}, DESC), argcpp17_exception);
}

TEST_F(parser_test, create_positional)
{
    sut.add_positional(KEY, DESC);
    EXPECT_EQ(sut.positionals(), 1);
    sut.add_positional(ANOTHER_KEY, DESC);
    EXPECT_EQ(sut.positionals(), 2);
    EXPECT_NO_THROW(sut.add_positional(ANOTHER_KEY, DESC));
    EXPECT_EQ(sut.positionals(), 3);
}

TEST_F(parser_test, create_flag_and_subcommand_with_identical_flags)
{
    EXPECT_NO_THROW(
        sut.add_flag({KEY}, DESC)
           .add_subcommand({ANOTHER_KEY}, DESC)
           .add_flag({KEY}, DESC)
    );

    EXPECT_EQ(sut.subcommands(), 1);
    EXPECT_EQ(sut.flags(), 1);

    auto sub_sut = sut.get_subcommand_parser({ANOTHER_KEY});
    EXPECT_EQ(sub_sut.subcommands(), 0);
    EXPECT_EQ(sub_sut.flags(), 1);

    EXPECT_THROW(sut.get_subcommand_parser({KEY}), argcpp17_exception);
}

TEST_F(parser_test, parse_subcommand)
{
    std::vector<std::string> args;

    sut.add_subcommand(KEY, DESC)
       .add_flag({ANOTHER_KEY, ABBR}, DESC);

    args = {ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);

    args = {KEY, ANOTHER_KEY};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {};
    EXPECT_NO_THROW(sut.parse_vector(args));
}

TEST_F(parser_test, parse_optionals)
{
    static const std::string OPT = "OPT";
    static const std::string ABBR = "O";
    static const std::string FLAG = "OVERLOAD";

    std::vector<std::string> args;

    sut.add_optional_argument({OPT, ABBR}, DESC)
       .add_flag({FLAG}, DESC);

    args = {FLAG, "--" + OPT + "value"};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {"--" + OPT + "value", FLAG};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {FLAG, "-" + ABBR + "value"};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {"-" + ABBR + "value", FLAG};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {FLAG};
    EXPECT_NO_THROW(sut.parse_vector(args));
}

TEST_F(parser_test, parse_mandatories)
{
    static const std::string OPT = "OPT";
    static const std::string ABBR = "O";
    static const std::string FLAG = "OVERLOAD";

    std::vector<std::string> args;

    sut.add_mandatory_argument({OPT, ABBR}, DESC)
       .add_flag({FLAG}, DESC);

    args = {FLAG, "--" + OPT + "value"};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {"--" + OPT + "value", FLAG};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {FLAG, "-" + ABBR + "value"};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {"-" + ABBR + "value", FLAG};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {FLAG};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);
}

TEST_F(parser_test, parse_flags)
{
    std::vector<std::string> args;

    sut.add_flag({KEY, ABBR}, DESC);

    args = {KEY};
    EXPECT_NO_THROW(sut.parse_vector(args));

    args = {KEY, ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);
}

TEST_F(parser_test, parse_posistionals)
{
    std::vector<std::string> args;

    sut.add_positional(KEY, DESC)
       .add_positional(ANOTHER_KEY, DESC);

    args = {KEY, ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);

    args = {ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);

    args = {KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);

    args = {ABBR, KEY, ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);

    sut.add_flag({ABBR}, DESC);
    args = {ABBR, KEY, ANOTHER_KEY};
    EXPECT_THROW(sut.parse_vector(args), argcpp17_exception);
}

TEST_F(parser_test, parse_values)
{
    std::vector<std::string> args;

    sut.add_flag({"flag", "f"}, "")
       .add_optional_argument({"double", "d"}, "")
       .add_optional_argument({"string", "s"}, "")
       .add_optional_argument({"uintmax", "u"}, "")
       .add_optional_argument({"int32", "i"}, "");
    

    args = {"f", 
            "-d", "3.14",
            "-shello world",
            "-u=10",
            "-i:-3"
            };
    EXPECT_NO_THROW(sut.parse_vector(args));

    // check flags
    EXPECT_TRUE(sut.get_flag({"flag"}));
    EXPECT_TRUE(sut.get_flag({"f"}));
    EXPECT_TRUE(sut.get_flag({"flag", "f"}));
    EXPECT_FALSE(sut.get_flag({"another_flag"}));

    EXPECT_FALSE(sut.get_value<double>({"abother_option"}).has_value());
    // check double option
    {
        auto value = sut.get_value<double>({"d"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), 3.14);
    }
    {
        auto value = sut.get_value<double>({"double"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), 3.14);
    }

    // check uintmax option
    {
        auto value = sut.get_value<uintmax_t>({"u"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), 10);
    }
    {
        auto value = sut.get_value<uintmax_t>({"uintmax"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), 10);
    }

    // check string option
    {
        auto value = sut.get_value<std::string>({"s"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), "hello world");
    }
    {
        auto value = sut.get_value<std::string>({"string"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), "hello world");
    }

        // check uintmax option
    {
        auto value = sut.get_value<int32_t>({"i"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), -3);
    }
    {
        auto value = sut.get_value<int32_t>({"int32"});
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), -3);
    }
}