#ifndef _ARGCPP17_H_
#define _ARGCPP17_H_

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <functional>


// ====================================================
// DECLARATIONS
// ====================================================

// class representing an argcpp17 exception
class argcpp17_exception : public std::exception
{
public:
    enum argcpp17_error {
        err_unknown,
        err_duplicate_keyword,
        err_unknown_arguments,
        err_missing_positionals,
        err_subcommand_not_found,
        err_missing_madatory,
    };

    argcpp17_exception(argcpp17_error error = err_unknown);

    const char* what() const noexcept override;
    
    inline argcpp17_error error() { return m_error; }

private:
    argcpp17_error m_error;    
};


// class representing a keyword with optional abbreviation
class keyword {
public:
    keyword() = default;
    keyword(const std::string& key, const std::optional<std::string>& abbreviation = std::nullopt);
    keyword(const keyword& rhs);
    ~keyword() = default;

    inline std::string get_key() const { return m_key; }
    inline std::optional<std::string> get_abbreviation() const { return m_abbreviation; }

    bool operator==(const keyword& rhs) const;
    bool operator==(const std::string& rhs) const;

private:
    std::string m_key;
    std::optional<std::string> m_abbreviation;
};


// ostream operator for keyword
std::ostream& operator<<(std::ostream& os, const keyword& key) {
    os << key.get_key();
    auto abbr = key.get_abbreviation();
    if (abbr.has_value())
        os << ", " << abbr.value();
    return os;
}

class argument {
friend class parser;

public:
    argument() = delete;
    ~argument() = default;

    inline keyword get_key() const { return m_key; }
    inline std::string get_description() const { return m_description; }
    inline bool is_parsed() const { return m_parsed; }

    bool operator==(const keyword& rhs) const;
    bool operator==(const std::string& rhs) const;

    //TODO: as template or std::variant
    virtual void update_value(const std::optional<std::string>& value) {};

protected:
    virtual void reset() { m_parsed = false; }

    argument(const keyword& key, const std::string& description);
    argument(const argument& rhs);

    inline void mark_parsed() { m_parsed = true; }
    inline void update_keyword(const keyword& key) { m_key = key; }

private:
    keyword m_key;
    std::string m_description;
    bool m_parsed;
};

// ostream operator for argument
std::ostream& operator<<(std::ostream& os, const argument& command) {
    os << command.get_key() << "    " << command.get_description();
    return os;
}


// class representing a subcommand consisting o f keyword, description and sub parser
// use template here to resolve cyclic dependencies
template<class T>
class subcommand : public argument {
public:
    subcommand() = delete;
    subcommand(const keyword& key, const std::string& description);
    subcommand(const subcommand& rhs);
    ~subcommand() = default;

    T& get_parser();

private:
    T m_parser;
};


// class representing a flag (bool)
class flag : public argument {
public:
    flag() = delete;
    flag(const keyword& key, const std::string& description);
    flag(const flag& rhs);
    ~flag() = default;

    inline bool is_set() const { return is_parsed(); }
};

// class representing an optional argument
//TODO: as template or std::variant
class optional_argument : public argument {
public:
    optional_argument() = delete;
    optional_argument(const keyword& key, const std::string& description);
    optional_argument(const optional_argument& rhs);
    ~optional_argument() = default;

    inline std::optional<std::string> value() const { return m_value; }

    void update_value(const std::optional<std::string>& value) override  { m_value = value; }
    void reset() override { argument::reset(); m_value = std::nullopt; }

private:
    std::optional<std::string> m_value;
};


// class representing an mandatory argument
//TODO: as template or std::variant
class mandatory_argument : public argument {
public:
    mandatory_argument() = delete;
    mandatory_argument(const keyword& key, const std::string& description);
    mandatory_argument(const mandatory_argument& rhs);
    ~mandatory_argument() = default;

    inline std::string value() const { return m_value; }

    void update_value(const std::optional<std::string>& value) override { m_value = value.value(); }
    void reset() override { 
        argument::reset(); 
        m_value = std::string();
    }

private:
    std::string m_value;
};


// class representing a positional argument
//TODO: as template or std::variant
class positional_argument : public argument {
public:
    positional_argument() = delete;
    positional_argument(const std::string& name, const std::string& description);
    positional_argument(const positional_argument& rhs);
    ~positional_argument() = default;

    inline std::string value() const { return m_value; }

    void update_value(const std::optional<std::string>& value) override  { m_value = value.value(); }
    void reset() override { argument::reset(); m_value = std::string(); }

private:
    std::string m_value;
};


// main argument parser class
class parser {
public:
    parser() = default;
    parser(const parser& rhs) = default;
    ~parser() = default;

    void usage();
    void parse(int argc, char **args);
    
    inline size_t subcommands() { return m_subcommands.size(); }
    inline size_t flags() { return m_flags.size(); }
    inline size_t mandatories() { return m_mandatories.size(); }
    inline size_t optionals() { return m_optionals.size(); }
    inline size_t positionals() { return m_positionals.size(); }

    parser& get_subcommand_parser(const keyword& key);

    parser& add_subcommand(const std::string& key, const std::string& description);
    parser& add_flag(const keyword& key, const std::string& description);
    parser& add_mandatory_argument(const keyword& key, const std::string& description);
    parser& add_optional_argument(const keyword& key, const std::string& description);
    parser& add_argument(const keyword& key, const std::string& description, bool optional = true);
    parser& add_positional(const std::string& name, const std::string& description);

protected:
    void parse_vector(std::vector<std::string>& args);
    
private:
    enum argument_value_type {
        none,
        one_string,
        equal_sign,
        whitespace,
        colon,
    };

    auto get_subcommand(const keyword& key);
    bool parse_subcommand(std::vector<std::string>& args);
    void parse_flags(std::vector<std::string>& args);
    void parse_options(std::vector<std::string>& args);
    void parse_positionals(std::vector<std::string>& args);

    auto find_option(const std::string& arg);
    template<class T>
    auto find_option(const std::string& arg, T begin, T end);

    void check_mandatory();

    std::vector<keyword> m_keywords;
    std::vector<subcommand<parser>> m_subcommands;
    std::vector<flag> m_flags;
    std::vector<mandatory_argument> m_mandatories;
    std::vector<optional_argument> m_optionals;
    std::vector<positional_argument> m_positionals;
};




// ====================================================
// IMPLEMENTATIONS
// ====================================================

//argcpp17_exception implementations
argcpp17_exception::argcpp17_exception(argcpp17_error error) 
    : m_error(error) 
{};

const char* argcpp17_exception::what() const noexcept
{
    switch (m_error) {
        case err_duplicate_keyword:
            return "keyword already used";
        case err_unknown_arguments:
            return "found unknown arguments";
        case err_missing_positionals:
            return "missing positional argumenzs";
        case err_subcommand_not_found:
            return "subcommand not found";
        case err_missing_madatory:
            return "missing mandatory argument";
        default:
            return "unknown error in argcpp17";
    }
}


//keyword implementations
keyword::keyword(const std::string& key, const std::optional<std::string>& abbreviation) 
    : m_key(key)
    , m_abbreviation(abbreviation)
{}

keyword::keyword(const keyword& rhs)
    : m_key(rhs.m_key)
    , m_abbreviation(rhs.m_abbreviation)
{}

bool keyword::operator==(const keyword& rhs) const
{
    return m_key == rhs.m_key || 
           m_key == rhs.m_abbreviation ||
           m_abbreviation == rhs.m_key ||
           (m_abbreviation.has_value() && m_abbreviation == rhs.m_abbreviation);
}

bool keyword::operator==(const std::string& rhs) const
{
    return m_key == rhs || 
           m_abbreviation == rhs;
}



//argument implementations
argument::argument(const keyword& key, const std::string& description)
    : m_key(key)
    , m_description(description)
    , m_parsed(false)
{};

argument::argument(const argument& rhs) 
    : m_key(rhs.m_key)
    , m_description(rhs.m_description)
    , m_parsed(rhs.m_parsed)
{}

bool argument::operator==(const keyword& rhs) const
{
    return m_key == rhs;
}

bool argument::operator==(const std::string& rhs) const
{
    return m_key == rhs;
}


//subcommand implementations
template<>
subcommand<parser>::subcommand(const keyword& key, const std::string& description)
    : argument(key, description)
{};

template<>
subcommand<parser>::subcommand(const subcommand<parser>& rhs) 
    : argument(rhs)
    , m_parser(rhs.m_parser)
{}

template<>
parser& subcommand<parser>::get_parser() 
{ 
    return m_parser; 
}


//flag implementations
flag::flag(const keyword& key, const std::string& description)
    : argument(key, description)
{};

flag::flag(const flag& rhs) 
    : argument(rhs)
{}

keyword verify_argument_key(const keyword& key)
{
    keyword updated_key = key;
    if (updated_key.get_key().substr(2) != "--")
        updated_key = keyword("--" + updated_key.get_key(), updated_key.get_abbreviation());
    else if (updated_key.get_key().substr(1) != "-")
        updated_key = keyword("-" + updated_key.get_key(), updated_key.get_abbreviation());
    if (updated_key.get_abbreviation().has_value() && (updated_key.get_abbreviation().value().substr(1) != "-"))
        updated_key = keyword(updated_key.get_key(), "-" + updated_key.get_abbreviation().value());
    return updated_key;    
}

//optional_argument implementations
optional_argument::optional_argument(const keyword& key, const std::string& description)
    : argument(key, description)
{
    update_keyword(verify_argument_key(key));
};

optional_argument::optional_argument(const optional_argument& rhs) 
    : argument(rhs)
    , m_value(rhs.m_value)
{}


//mandatory_argument implementations
mandatory_argument::mandatory_argument(const keyword& key, const std::string& description)
    : argument(key, description)
{
    update_keyword(verify_argument_key(key));
};

mandatory_argument::mandatory_argument(const mandatory_argument& rhs) 
    : argument(rhs)
    , m_value(rhs.m_value)
{}


//positional_argument implementations
positional_argument::positional_argument(const std::string& name, const std::string& description)
    : argument(name, description)
{};

positional_argument::positional_argument(const positional_argument& rhs) 
    : argument(rhs)
    , m_value(rhs.m_value)
{}


//parser implementations
void parser::usage()
{
}

void parser::parse(int argc, char **args) {
    // create string vector from argument and remove first argument
    auto arg_vector = std::vector<std::string>(&args[1], &args[1] + argc - 1);
    parse_vector(arg_vector);
}

auto parser::get_subcommand(const keyword& key)
{
    auto it = std::find_if(m_subcommands.begin(), m_subcommands.end(), [&](const subcommand<parser>& item) { return item == key; });
    if (it == m_subcommands.end())
        throw argcpp17_exception(argcpp17_exception::err_subcommand_not_found);
    return it;
}

parser& parser::get_subcommand_parser(const keyword& key)
{
    return get_subcommand(key)->get_parser();
}

parser& parser::add_subcommand(const std::string& key, const std::string& description)
{
    keyword kw = { key };
    if (std::find(m_keywords.begin(), m_keywords.end(), kw) != m_keywords.end())
        throw argcpp17_exception(argcpp17_exception::err_duplicate_keyword); 
    m_keywords.push_back(kw);
    m_subcommands.push_back(subcommand<parser>(kw, description));
    return m_subcommands.back().get_parser();
}

parser& parser::add_flag(const keyword& key, const std::string& description)
{
    if (std::find(m_keywords.begin(), m_keywords.end(), key) != m_keywords.end())
        throw argcpp17_exception(argcpp17_exception::err_duplicate_keyword); 
    m_keywords.push_back(key);
    m_flags.push_back(flag(key, description));
    return *this;
}

parser& parser::add_mandatory_argument(const keyword& key, const std::string& description)
{
    if (std::find(m_keywords.begin(), m_keywords.end(), key) != m_keywords.end())
        throw argcpp17_exception(argcpp17_exception::err_duplicate_keyword); 
    m_keywords.push_back(key);
    m_mandatories.push_back(mandatory_argument(key, description));
    return *this;
}

parser& parser::add_optional_argument(const keyword& key, const std::string& description)
{
    if (std::find(m_keywords.begin(), m_keywords.end(), key) != m_keywords.end())
        throw argcpp17_exception(argcpp17_exception::err_duplicate_keyword); 
    m_keywords.push_back(key);
    m_optionals.push_back(optional_argument(key, description));
    return *this;
}

parser& parser::add_argument(const keyword& key, const std::string& description, bool optional)
{
    return optional ? add_optional_argument(key, description) : add_mandatory_argument(key, description);
}

parser& parser::add_positional(const std::string& name, const std::string& description)
{    
    m_positionals.push_back(positional_argument(name, description));
    return *this;
}

void parser::parse_vector(std::vector<std::string>& args)
{
    // reset
    for (auto &it : m_subcommands) it.reset();
    for (auto &it : m_flags) it.reset();
    for (auto &it : m_optionals) it.reset();
    for (auto &it : m_mandatories) it.reset();
    for (auto &it : m_positionals) it.reset();

    // no arguments, return
    if (!args.size())
        return;

    //  we hit a subcommand, so we are done here
    if (parse_subcommand(args))
        return;
    parse_options(args);
    parse_flags(args);
    parse_positionals(args);
}

bool parser::parse_subcommand(std::vector<std::string>& args)
{
    try {
        auto sub_command = get_subcommand(args.front());
        args.erase(args.begin());
        sub_command->get_parser().parse_vector(args);
        sub_command->mark_parsed();
        return true;
    } catch (argcpp17_exception& e) {
        // first argument is no subcommand
        // TODO: 
        // - maybe should rethrow this exception if a subcommand MUST be specified
        // - parser flag or automatic if no other arguments found in parser
    }
    return false;
}

void parser::parse_flags(std::vector<std::string>& args)
{
    auto it = args.begin();
    while (it != args.end()) {
        auto f = std::find_if(m_flags.begin(), m_flags.end(), [&](const flag& item) { return item == *it; });
        if (f == m_flags.end())
            it++;
        else {
            f->mark_parsed();
            it = args.erase(it);
        }
    }
}

template<class T>
auto parser::find_option(const std::string& arg, T begin, T end)
{
    argument_value_type value_type = none;
    std::string value;
    auto a = std::find_if(begin, end, [&](const argument& item) 
    { 
        auto key = item.get_key().get_key();
        auto abbr = item.get_key().get_abbreviation();

        // argument key and value seperated by whitespace
        if (key == arg || (abbr.has_value() && abbr.value() == arg)) {
            value_type = parser::whitespace;
            return true;
        }

        // argument as one string or with seperating char ('=' or ':')
        if (arg.length() > key.length() && arg.substr(0, key.length()) == key) {
            // TODO: create function for this duplicate code
            if (arg.substr(key.length(), 1) == "=") {
                value_type = parser::equal_sign;
                value = arg.substr(key.length() + 1);
            }
            else if (arg.substr(key.length(), 1) == ":") {
                value_type = parser::colon;
                value = arg.substr(key.length() + 1);
            }
            else {
                value_type = parser::one_string;
                value = arg.substr(key.length());
            }
            return true;
        }
        
        if (!abbr.has_value())
            return false;
        auto abbr_val = abbr.value();
        if (arg.length() > abbr_val.length() && arg.substr(0, abbr_val.length()) == abbr_val) {
            // TODO: create function for this duplicate code
            if (arg.substr(abbr_val.length(), 1) == "=") {
                value_type = parser::equal_sign;
                value = arg.substr(abbr_val.length() + 1);
            }
            else if (arg.substr(abbr_val.length(), 1) == ":") {
                value_type = parser::colon;
                value = arg.substr(abbr_val.length() + 1);
            }
            else {
                value_type = parser::one_string;
                value = arg.substr(abbr_val.length());
            }
            return true;
        }
        return false;
    });

    return std::make_tuple<>(a != end ? (argument*) &(*a) : nullptr, value_type, value);
}

auto parser::find_option(const std::string& arg)
{    
    auto [mandatory_arg, mandatory_type, madatory_value] = find_option(arg, m_mandatories.begin(), m_mandatories.end());
    auto [optional_arg, optional_type, optional_value]  = find_option(arg, m_optionals.begin(), m_optionals.end());
    return std::make_tuple<>((argument*)((uintptr_t) mandatory_arg | (uintptr_t) optional_arg), mandatory_type | optional_type, mandatory_arg ? madatory_value : optional_value);
}

void parser::parse_options(std::vector<std::string>& args)
{
    auto it = args.begin();
    while (it != args.end()) {
        auto [argument, type, value] = find_option(*it);
        if (argument) {
            argument->mark_parsed();
            switch (type)
            {
            case one_string:
            case equal_sign:
            case colon:
                // TODO: add value
                break;
            case whitespace:
                it = args.erase(it);
                value = *it;
                break;
            default:
                break;
            }
            argument->update_value(value);
            it = args.erase(it);
        } else 
            it++;
    }
    check_mandatory();
}

void parser::parse_positionals(std::vector<std::string>& args)
{
    if (args.size() > m_positionals.size())
        throw argcpp17_exception(argcpp17_exception::err_unknown_arguments);
    else if (args.size() < m_positionals.size())
        throw argcpp17_exception(argcpp17_exception::err_missing_positionals);

    auto arg = args.begin();
    auto pos = m_positionals.begin();
    while (arg != args.end()) {
        pos->update_value(*arg);
        arg++;
        pos++;
    }
}

void parser::check_mandatory()
{
    for (auto &mandatory : m_mandatories)
        if (!mandatory.is_parsed())
            throw argcpp17_exception(argcpp17_exception::err_missing_madatory);
}

#endif