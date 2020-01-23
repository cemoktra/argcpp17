#ifndef _ARGCPP17_H_
#define _ARGCPP17_H_

#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <inttypes.h>

// ====================================================
// DECLARATIONS
// ====================================================

template<typename T>
T parse_value(const std::string& value);

template<typename T>
std::optional<T> parse_value(const std::optional<std::string>& value);

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
        err_missing_mandatory,
        err_missing_positional,
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
template<typename T>
class subcommand : public argument {
    friend class parser;

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
    friend class parser;

public:
    optional_argument() = delete;
    optional_argument(const keyword& key, const std::string& description);
    optional_argument(const optional_argument& rhs);
    ~optional_argument() = default;

    template<typename T>
    inline std::optional<T> value() { return parse_value<T>(m_value); }

protected:
    void update_value(const std::optional<std::string>& value) override  { m_value = value; }
    void reset() override { argument::reset(); m_value = std::nullopt; }

private:
    std::optional<std::string> m_value;
};


// class representing an mandatory argument
//TODO: as template or std::variant
class mandatory_argument : public argument {
    friend class parser;

public:
    mandatory_argument() = delete;
    mandatory_argument(const keyword& key, const std::string& description);
    mandatory_argument(const mandatory_argument& rhs);
    ~mandatory_argument() = default;

    template<typename T>
    inline T value() { return parse_value<T>(m_value); }

protected:
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
    friend class parser;

public:
    positional_argument() = delete;
    positional_argument(const std::string& name, const std::string& description);
    positional_argument(const positional_argument& rhs);
    ~positional_argument() = default;

    template<typename T>
    inline T value() { return parse_value<T>(m_value); }

protected:
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

    void usage(const std::string& app_name);
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

    template<typename T>
    std::optional<T> get_value(const keyword& key);
    bool get_flag(const keyword& key);

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

    auto parse_argument_value(const std::string& arg);
    template<typename T>
    auto parse_argument_value(const std::string& arg, T begin, T end);

    template<typename T>
    auto find_option(const keyword& key, T begin, T end);

    void check_mandatory();
    void check_positional();
    void check_keyword(const keyword& key);
    auto check_value_type(const std::string& key, const std::string& arg);

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

template<>
std::string parse_value(const std::string& value)
{
    return value;
}


template<>
std::optional<std::string> parse_value(const std::optional<std::string>& value)
{
   return value;
}

template<typename T>
T parse_value(const std::string& value)
{
    std::istringstream iss(value);
    T casted;
    iss >> casted;
    return casted;
}


template<typename T>
std::optional<T> parse_value(const std::optional<std::string>& value)
{
    if (!value.has_value())
        return std::nullopt;
    return parse_value<T>(value.value());
}


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
        case err_missing_mandatory:
            return "missing mandatory argument";
        case err_missing_positional:
            return "missing positional argument";
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
};

optional_argument::optional_argument(const optional_argument& rhs) 
    : argument(rhs)
    , m_value(rhs.m_value)
{}


//mandatory_argument implementations
mandatory_argument::mandatory_argument(const keyword& key, const std::string& description)
    : argument(key, description)
{
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
void parser::usage(const std::string& app_name)
{
    std::cout << app_name << " [sub-command] <mandatory_options> [options/flags]";
    for (auto positional : m_positionals)
        std::cout << " " << positional.m_key.get_key();
    std::cout << std::endl;
    std::cout << std::endl;

    if (m_subcommands.size()) {
        std::cout << "sub-commands:" << std::endl;
        for (auto i : m_subcommands) {
            auto key = i.get_key();
            auto desc = i.get_description();
            auto full_key = key.get_key();
            auto abbr_key = key.get_abbreviation();
            std::cout << "  " << full_key;
            if (abbr_key.has_value())
                std::cout << ", " << abbr_key.value();
            std::cout << ": " << desc;
        }
    }

    if (m_mandatories.size()) {
        std::cout << "mandatory options:" << std::endl;
        for (auto i : m_mandatories) {
            auto key = i.get_key();
            auto desc = i.get_description();
            auto full_key = key.get_key();
            auto abbr_key = key.get_abbreviation();
            std::cout << "  [-" << full_key;
            if (abbr_key.has_value())
                std::cout << ", -" << abbr_key.value();
            std::cout << "]<value>: " << desc;
        }
    }

    if (m_optionals.size()) {
        std::cout << "options:" << std::endl;
        for (auto i : m_mandatories) {
            auto key = i.get_key();
            auto desc = i.get_description();
            auto full_key = key.get_key();
            auto abbr_key = key.get_abbreviation();
            std::cout << "  [-" << full_key;
            if (abbr_key.has_value())
                std::cout << ", -" << abbr_key.value();
            std::cout << "]<value>: " << desc;
        }
    }

    if (m_optionals.size()) {
        std::cout << "flags:" << std::endl;
        for (auto i : m_flags) {
            auto key = i.get_key();
            auto desc = i.get_description();
            auto full_key = key.get_key();
            auto abbr_key = key.get_abbreviation();
            std::cout << "  " << full_key;
            if (abbr_key.has_value())
                std::cout << ", " << abbr_key.value();
            std::cout << ": " << desc;
        }
    }

    if (m_positionals.size()) {
        std::cout << "positional arguments:" << std::endl;
        for (auto i : m_positionals) {
            auto key = i.get_key();
            auto desc = i.get_description();
            auto full_key = key.get_key();
            auto abbr_key = key.get_abbreviation();
            std::cout << "  " << full_key;
            if (abbr_key.has_value())
                std::cout << ", " << abbr_key.value();
            std::cout << ": " << desc;
        }
    }
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

void parser::check_keyword(const keyword& key)
{
    if (std::find(m_keywords.begin(), m_keywords.end(), key) != m_keywords.end())
        throw argcpp17_exception(argcpp17_exception::err_duplicate_keyword);
    m_keywords.push_back(key);
}

parser& parser::add_subcommand(const std::string& key, const std::string& description)
{
    keyword kw = { key };
    check_keyword(kw);
    m_subcommands.push_back(subcommand<parser>(kw, description));
    return m_subcommands.back().get_parser();
}

parser& parser::add_flag(const keyword& key, const std::string& description)
{
    check_keyword(key);
    m_flags.push_back(flag(key, description));
    return *this;
}

parser& parser::add_mandatory_argument(const keyword& key, const std::string& description)
{
    check_keyword(key);
    m_mandatories.push_back(mandatory_argument(key, description));
    return *this;
}

parser& parser::add_optional_argument(const keyword& key, const std::string& description)
{
    check_keyword(key);
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

    if (args.size())
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

auto parser::check_value_type(const std::string& key, const std::string& arg)
{
    if (arg.substr(key.length(), 1) == "=")
        return std::make_pair<>(parser::equal_sign, arg.substr(key.length() + 1));
    else if (arg.substr(key.length(), 1) == ":")
        return std::make_pair<>(parser::colon, arg.substr(key.length() + 1));
    else
        return std::make_pair<>(parser::one_string, arg.substr(key.length()));
}

template<typename T>
auto parser::parse_argument_value(const std::string& arg, T begin, T end)
{
    argument_value_type value_type = none;
    std::string value;
    auto a = std::find_if(begin, end, [&](const argument& item) 
    { 
        auto keyword = verify_argument_key(item.get_key());
        auto key = keyword.get_key();
        auto abbr = keyword.get_abbreviation();

        // argument key and value seperated by whitespace
        if (key == arg || (abbr.has_value() && abbr.value() == arg)) {
            value_type = parser::whitespace;
            return true;
        }

        // argument as one string or with seperating char ('=' or ':')
        if (arg.length() > key.length() && arg.substr(0, key.length()) == key) {
            std::tie(value_type, value) = check_value_type(key, arg);
            return true;
        }
        
        if (!abbr.has_value())
            return false;
        auto abbr_val = abbr.value();
        if (arg.length() > abbr_val.length() && arg.substr(0, abbr_val.length()) == abbr_val) {
            std::tie(value_type, value) = check_value_type(abbr_val, arg);
            return true;
        }
        return false;
    });

    return std::make_tuple<>(a != end ? (argument*) &(*a) : nullptr, value_type, value);
}

auto parser::parse_argument_value(const std::string& arg)
{    
    auto [mandatory_arg, mandatory_type, madatory_value] = parse_argument_value(arg, m_mandatories.begin(), m_mandatories.end());
    auto [optional_arg, optional_type, optional_value]  = parse_argument_value(arg, m_optionals.begin(), m_optionals.end());
    return std::make_tuple<>((argument*)((uintptr_t) mandatory_arg | (uintptr_t) optional_arg), mandatory_type | optional_type, mandatory_arg ? madatory_value : optional_value);
}

void parser::parse_options(std::vector<std::string>& args)
{
    auto it = args.begin();
    while (it != args.end()) {
        auto [argument, type, value] = parse_argument_value(*it);
        if (argument) {
            argument->mark_parsed();
            switch (type)
            {
            case one_string:
            case equal_sign:
            case colon:
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
        pos->mark_parsed();
        arg++;
        pos++;
    }
    check_positional();
}

void parser::check_positional()
{
    for (auto &positional : m_positionals)
        if (!positional.is_parsed())
            throw argcpp17_exception(argcpp17_exception::err_missing_positional);
}

void parser::check_mandatory()
{
    for (auto &mandatory : m_mandatories)
        if (!mandatory.is_parsed())
            throw argcpp17_exception(argcpp17_exception::err_missing_mandatory);
}

template<typename T>
auto parser::find_option(const keyword& key, T begin, T end)
{
    return std::find_if(begin, end, [&](const argument& item) 
    { 
        return item == key;
    });
}

template<typename T>
std::optional<T> parser::get_value(const keyword& key)
{
    {
        auto it = find_option(key, m_optionals.begin(), m_optionals.end());
        if (it != m_optionals.end())
            return it->value<T>();
    }
    {
        auto it = find_option(key, m_mandatories.begin(), m_mandatories.end());
        if (it != m_mandatories.end())
            return it->value<T>();
    }
    {
        auto it = find_option(key, m_positionals.begin(), m_positionals.end());
        if (it != m_positionals.end())
            return it->value<T>();
    }
    return std::nullopt;
}

bool parser::get_flag(const keyword& key)
{
    auto it = find_option(key, m_flags.begin(), m_flags.end());
    if (it != m_flags.end())
        return it->is_set();
    return false;
}

#endif