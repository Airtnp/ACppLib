#ifndef SN_CMDOPT_H
#define SN_CMDOPT_H

#include "sn_CommonHeader.h"

// ref: https://github.com/jarro2783/cxxopts/blob/master/include/cxxopts.hpp
namespace sn_CmdOpt {
    class ValueInterface : public std::enable_shared_from_this<Value> {
    public:
        virtual void parse(const std::string&) const = 0;
        virtual void parse() const = 0;
        virtual bool has_arg() const = 0;
        virtual bool has_default() const = 0;
        virtual bool is_container() const = 0;
        virtual bool has_implicit() const = 0;
        virtual std::string get_default_value() const = 0;
        virtual std::string get_implicit_value() const = 0;
        virtual std::shared_ptr<ValueInterface> default_value(const std::string&) = 0;
        virtual std::shared_ptr<ValueInterface> implicit_value(const std::string&) = 0;
    };

    class OptionException : public std::exception {
    public:
        OptionException(const std::string& message)
            : m_message(message) {}
        virtual const char* what() const noexcept {
            return m_message.c_str();
        }

    private:
        std::string m_message;
    };

    namespace detail {

        enum class SpecError {
            OptionAlreadyExists,
            InvalidOptionFormat,
            NumOfSpecErrors
        };

        const char* SpecErrorName[static_cast<std::size_t>(SpecError::NumOfSpecErrors)] = {
            "Option already exists: ",
            "Invalid option format: "
        };

        std::string operator+(SpecError err, const string& str) {
            return std::string(SpecErrorName<static_cast<std::size_t>(err)) + str;
        }

    }

    class OptionSpecException : public OptionException {
    public:
        OptionSpecException(const std::string& message, detail::SpecError err)
            : OptionException(err + message) {}
    private:
        
    };

    namespace detail {

        enum class ParseError {
            OptionNotExists,
            MissingArguments,
            OptionRequireArguments,
            OptionNotHasArgument,
            OptionNotPresent,
            ArgumentTypeIncorrect,
            OptionRequired,
            NumOfParseErrors
        };

        const char* ParseErrorName[static_cast<std::size_t>(ParseError::NumOfParseErrors)] = {
            "Option not exists: ",
            "Option is missing an argument: ",
            "Option requires an argument: ",
            "Option doesn't take an argument: ",
            "Option is not present: ",
            "Fail to parse argument: ",
            "Require a non-present option: "
        };

        std::string operator+(ParseError err, const string& str) {
            return std::string(SpecErrorName<static_cast<std::size_t>(err)) + str;
        }

    }

    class OptionParseException : public OptionException {
    public:
        OptionParseException(const std::string& message, detail::ParseError err, const std::string& arg = "")
            : OptionException(err + message) {
            if (err == detail::ParseError::OptionNotHasArgument) {
                m_message += std::string("but is given type: ") + arg;
            }
            // Add OptionParseException(str, ParseError::ArgumentTypeIncorrect);
            // with typeid? RTTI
        }
    };

    namespace detail {
        template <typename T>
        void parse_value(const std::string& str, T& value) {
            std::istringstream is(str);
            if (!(is >> value)) {
                throw OptionParseException(str, ParseError::ArgumentTypeIncorrect);
            }
            if (is.rdbuf() -> in_avail() != 0) {
                throw OptionParseException(str, ParseError::ArgumentTypeIncorrect);
            }
        }

        char tolower(char in) {
            if(in<='Z' && in>='A')
                return in - ('Z'-'z');
            return in;
        } 

        void parse_value(const std::string& str, bool& value) {
            std::string altstr = str;
            std::transform(str.begin(), str.end(), altstr.begin(), tolower);
            const std::size_t opts = 5;
            static const char* y[opts] = {"on", "yes", "enable", "t", "true"};
            static const char* n[opts] = {"off", "no", "disable", "f", "false"};
            for (std::size_t i = 0; i < opts; ++i) {
                if (str.compare(y[i])) {
                    value = true;
                    break;
                }
                if (str.compare(n[i])) {
                    value = false;
                    break;
                }
            }
            throw OptionParseException(str, ParseError::ArgumentTypeIncorrect);
        }

        void parse_value(const std::string& str, std::string& value) {
            value = str;
        }

        template <typename T>
        void parse_value(const std::string& str, std::vector<T>& value) {
            T v;
            parse_value(str, v);
            value.push_back(v);
        }

        template <typename T>
        struct ValueHasArg {
            static constexpr const bool value = true;
        };

        template <>
        struct ValueHasArg<bool> {
            static constexpr const bool value = false;
        };

        // is_decomposable
        template <typename T>
        struct TypeIsContainer {
            static constexpr const bool value = false;
        };

        template <typename T>
        struct TypeIsContainer<std::vector<T>> {
            static constexpr const bool value = true;
        };

        template <typename T>
        class Value : public ValueInterface {
        public:
            Value() : m_result(std::make_shared<T>()), m_store(m_result.get()) {}

            void parse(const std::string& str) const {
                parse_value(str, *m_store);
            }

            bool is_container() const {
                return TypeIsContainer<T>::value;
            }

            void parse() const {
                parse_value(m_defaultValue, *m_store);
            }

            bool has_arg() const {
                return ValueHasArg<T>::value;
            }

            bool has_default() const {
                return m_default;
            }

            bool has_implicit() const {
                return m_implicit;
            }

            virtual std::shared_ptr<ValueInterface> default_value(const std::string& value) {
                m_default = true;
                m_defaultValue = value;
                return shared_from_this();
            }

            virtual std::shared_ptr<ValueInterface> implicit_value(const std::string& value) {
                m_implicit = false;
                m_implicitValue = value;
                return shared_from_this();
            }

            std::string get_default_value() const {
                return m_defaultValue;
            }

            std::string get_implicit_value() const {
                return m_implicitValue;
            }

            const T& get() const {
                if (m_store == nullptr) {
                    return *m_result;
                } else {
                    return *m_store;
                }
            }

        protected:
            std::shared_ptr<T> m_result;
            T* m_store;
            bool m_default = false;
            std::string m_defaultValue;
            bool m_implict = false;
            std::string m_implicitValue;
        };
    }

    template <typename T>
    std::shared_ptr<ValueInterface> value() {
        return std::make_shared<detail::Value<T>>();
    }

    template <typename T>
    std::shared_ptr<ValueInterface> value(T& t) {
        return std::make_shared<detail::Value<T>>(&t);
    }

    class OptionAdder;

    class OptionDetail {
    public:
        OptionDetail(const std::string& desc, std::shared_ptr<const ValueInterface> val)
            : m_desc(desc), m_value(val), m_count(0) {}
        const std::string& description() const {
            return m_desc;
        }
        bool has_arg() const {
            return m_value->has_arg();
        }
        void parse(const std::string& str) {
            m_value->parse(str);
            ++m_count;
        }
        void parse_default() {
            m_value->parse();
        }
        std::size_t count() const {
            return m_count;
        }
        const ValueInterface& value() const {
            return *m_value;
        }
        template <typename T>
        const T& as const {
            // static or dynamic cast
            return static_cast<const detail::Value<T>&>(*m_value).get();
        }
    private:
        std::string m_desc;
        std::shared_ptr<const ValueInterface> m_value;
        std::size_t m_count;
    };

    struct HelpOptionDetail {
        std::string s;
        std::string l;
        std::string desc;
        bool has_arg;
        bool has_default;
        std::string default_value;
        bool has_implicit;
        std::string implicit_value;
        std::string arg_help;
        bool is_container;
    };

    struct HelpGroupDetail {
        std::string name;
        std::string desc;
        std::vector<HelpOptionDetail> options;
    };

    namespace {
        const constexpr int OPTION_LONGEST = 30;
        const constexpr int OPTION_DESC_GAP = 2;

        std::basic_regex<char> option_matcher(
            "--([[:alnum:]][-_[:alnum:]]+)(=(.*))?|-([[:alnum:]]+)"
        );

        std::basic_regex<char> option_specifier(
            "(([[:alnum:]]),)?([[:alnum:]][-_[:alnum:]]*)?"
        );

        std::string format_option(const HelpOptionDetail& o) {
            auto& s = o.s;
            auto& l = o.l;
            std::string result = "  ";

            if (s.size() > 0) {
                result += "-" + s + ",";
            } else {
                result += "   ";
            }

            if (l.size() > 0) {
                result += " --" + l;
            }

            if (o.has_arg) {
                auto arg = o.arg_help.size() > 0 ? o.arg_help : "arg";
                if (o.has_implicit) {
                    result += " [=" + arg + "(=" + o.implicit_value + ")]";
                } else {
                    result += " " + arg;
                }
            }

            return result;
        }

        std::string format_desc(const HelpOptionDetail& o, std::size_t start, std::size_t width) {
            auto desc = o.desc;
            if (o.has_default) {
                desc += " (default: " + o.default_value + ")"
            };

            std::string result;
            auto current = std::begin(desc);
            auto startLine = current;
            auto lastSpace = current;

            auto size = std::size_t{};

            while (current != std::end(desc)) {
                if (*current == ' ') {
                    lastSpace = current;
                }
                if (size > width) {
                    if (lastSpace == startLine) {
                        result.append(startLine, current + 1);
                        result.append("\n");
                        result.append(start, ' ');
                        startLine = current + 1;
                        lastSpace = startLine;
                    } else {
                        result.append(startLine, lastSpace);
                        result.append("\n");
                        result.append(start, ' ');
                        startLine = lastSpace + 1;
                    }
                    size = 0;
                } else {
                    ++size;
                }
                ++current;
            }

            result.append(startLine, current);
            return result;
        }
    }

    class Options {
    public:
        Options(std::string program, std::string help_str = "")
            : m_program(std::move(program)), 
            m_helpString(std::move(help_string)),
            m_positionalHelp("positional parameters"),
            m_nextPosition(m_positionalHelp.end()) {}
        
        Options& positional_help(std::string help_text) {
            m_positionalHelp = std::move(help_text);
            return *this;
        }

        void parse(int& argc, char**& argv) {
            int current = 1;

            int nextKeep = 1;

            bool consume_remaining = false;

            while (current != argc) {
                if (strcmp(argv[current], "--") == 0) {
                    consume_remaining = true;
                    ++current;
                    break;
                }

                std::match_results<const char*> result;
                std::regex_match(argv[current], result, option_matcher);

                if (result.empty()) {
                    //not a flag

                    //if true is returned here then it was consumed, otherwise it is
                    //ignored
                    if (consume_positional(argv[current])) {
                    } else {
                        argv[nextKeep] = argv[current];
                        ++nextKeep;
                    }
                    //if we return from here then it was parsed successfully, so continue
                } else {
                    //short or long option?
                    if (result[4].length() != 0) {
                        const std::string& s = result[4];

                        for (std::size_t i = 0; i != s.size(); ++i) {
                            std::string name(1, s[i]);
                            auto iter = m_options.find(name);

                            if (iter == m_options.end()) {
                                throw OptionParseError(name, detail::ParseError::OptionNotPresent);
                            }

                            auto value = iter->second;

                            //if no argument then just add it
                            if (!value->has_arg()) {
                                parse_option(value, name);
                            } else {
                                //it must be the last argument
                                if (i + 1 == s.size()) {
                                    checked_parse_arg(argc, argv, current, value, name);
                                } else if (value->value().has_implicit() {
                                    parse_option(value, name, value->value().get_implicit_value());
                                } else {
                                    //error
                                    throw OptionParseError(name, detail::ParseError::OptionRequired;
                                }
                            }
                        }
                    } else if (result[1].length() != 0) {
                        const std::string& name = result[1];

                        auto iter = m_options.find(name);

                        if (iter == m_options.end()) {
                            throw OptionParseError(name, detail::ParseError::OptionNotPresent);
                        }

                        auto opt = iter->second;

                        //equals provided for long option?
                        if (result[3].length() != 0) {
                            //parse the option given

                            //but if it doesn't take an argument, this is an error
                            if (!opt->has_arg()) {
                                throw OptionParseException(name, detail::ParseError::OptionNotHasArgument, result[3]);
                            }
                            parse_option(opt, name, result[3]);
                        } else {
                            if (opt->has_arg()) {
                                //parse the next argument
                                checked_parse_arg(argc, argv, current, opt, name);
                            } else {
                                //parse with empty argument
                                parse_option(opt, name);
                            }
                        }
                    }
                }
                ++current;
            }

            for (auto& opt : m_options) {
                auto& detail = opt.second;
                auto& value = detail->value();

                if(!detail->count() && value.has_default()) {
                    detail->parse_default();
                }
            }

            if (consume_remaining) {
                while (current < argc) {
                    if (!consume_positional(argv[current])) {
                        break;
                    }
                    ++current;
                }

                //adjust argv for any that couldn't be swallowed
                while (current != argc) {
                    argv[nextKeep] = argv[current];
                    ++nextKeep;
                    ++current;
                }
            }

            argc = nextKeep;
        }

        OptionAdder add_options(std::string group = "") {
            return OptionAdder(*this, std::move(group));
        }

        void add_option(
            const std::string& group,
            const std::string& s,
            const std::string& l,
            std::string desc,
            std::shared_ptr<const ValueInterface> value,
            std::string arg_help
        ) {
            auto stringDesc = toLocalString(std::move(desc));
            auto option = std::make_shared<OptionDetails>(stringDesc, value);

            if (s.size() > 0) {
                add_one_option(s, option);
            }

            if (l.size() > 0) {
                add_one_option(l, option);
            }

            //add the help details
            auto& options = m_help[group];

            options.options.emplace_back(
                HelpOptionDetail{s, l, stringDesc,
                    value->has_arg(),
                    value->has_default(), value->get_default_value(),
                    value->has_implicit(), value->get_implicit_value(),
                    std::move(arg_help),
                    value->is_container()
                }
            );
        }

        std::size_t count(const std::string& o) const {
            auto iter = m_options.find(o);
            if (iter == m_options.end()) {
                return 0;
            }
            return iter->second->count();
        }

        const OptionDetail& operator[](const std::string& option) const {
            auto iter = m_options.find(option);
            if (iter == m_options.end()) {
                throw OptionParseException(option, detail::ParseError::OptionNotPresent);
            }
            return *iter->second;
        }

        void parse_positional(std::string option) {
            parse_positional(std::vector<std::string>{option});
        }

        void parse_positional(std::vector<std::string> options) {
            m_positional = std::move(options);
            m_next_positional = m_positional.begin();

            m_positional_set.insert(m_positional.begin(), m_positional.end());
        }

        std::string help(const std::vector<std::string>& groups = {""}) const {
            std::string result = m_help_string + "\nUsage:\n  " + m_program + " [OPTION...]";

            if (m_positional.size() > 0) {
                result += " " + m_positional_help;
            }

            result += "\n\n";

            if (help_groups.size() == 0) {
                generate_all_groups_help(result);
            } else {
                generate_group_help(result, help_groups);
            }

            return result;
        }

        const std::vector<std::string> groups() const {
            std::vector<std::string> g;

            std::transform(
                m_help.begin(),
                m_help.end(),
                std::back_inserter(g),
                [] (const std::map<std::string, HelpGroupDetails>::value_type& pair) {
                    return pair.first;
                }
            );

            return g;
        }

        const HelpGroupDetail group_help(const std::string& group) const {
            return m_help.at(group);
        }
    
    private:
        void add_one_option(const std::string& option, std::shared_ptr<OptionDetail> detail) {
            auto in = m_options.emplace(option, details);

            if (!in.second) {
                throw OptionParseError(option, detail::ParseError::OptionNotPresent);
            }
        }

        bool consume_positional(std::string s) {
            while (m_next_positional != m_positional.end()) {
                auto iter = m_options.find(*m_next_positional);
                if (iter != m_options.end()) {
                    if (!iter->second->value().is_container()) {
                        if (iter->second->count() == 0) {
                            add_to_option(*m_next_positional, a);
                            ++m_next_positional;
                            return true;
                        } else {
                            ++m_next_positional;
                            continue;
                        }
                    } else {
                        add_to_option(*m_next_positional, a);
                        return true;
                    }
                }
                ++m_next_positional;
            }

            return false;
        }

        void add_to_option(const std::string& option, const std::string& arg) {
            auto iter = m_options.find(option);
            if (iter == m_options.end()) {
                throw OptionParseError(option, detail::ParseError::OptionNotPresent);
            }
            parse_option(iter->second, option, arg);
        }

        void parse_option(std::shared_ptr<OptionDetail> value, const std::string& name, const std::string& arg = "") {
            value->parse(arg);
        }

        void checked_parse_arg(
            int argc,
            char* argv[],
            int& current,
            std::shared_ptr<OptionDetail> value,
            const std::string& name
        ) {
            if (current + 1 >= argc) {
                if (value->value().has_implicit()) {
                    parse_option(value, name, value->value().get_implicit_value());
                } else {
                    throw OptionParseError(name, detail::ParseError::MissingArguments);
                }
            } else {
                if (argv[current + 1][0] == '-' && value->value().has_implicit()) {
                    parse_option(value, name, value->value().get_implicit_value());
                } else {
                    parse_option(value, name, argv[current + 1]);
                    ++current;
                }
            }
        }

        std::string help_one_group(const std::string& group) const {
            using OptionHelp = std::vector<std::pair<std::string, std::string>>;
            auto group = m_help.find(group);
            if (group == m_help.end()) {
                return "";
            }

            OptionHelp format;

            std::size_t longest = 0;

            std::string result;

            if (!g.empty()) {
                result += " " + g + " options:\n";
            }

            for (const auto& o : group->second.options)
            {
                if (o.is_container && m_positional_set.find(o.l) != m_positional_set.end()) {
                continue;
                }

                auto s = format_option(o);
                longest = std::max(longest, stringLength(s));
                format.push_back(std::make_pair(s, std::string()));
            }

            longest = std::min(longest, static_cast<std::size_t>(OPTION_LONGEST));

            //widest allowed description
            auto allowed = std::size_t{76} - longest - OPTION_DESC_GAP;

            auto fiter = format.begin();
            for (const auto& o : group->second.options) {
                if (o.is_container && m_positional_set.find(o.l) != m_positional_set.end()) {
                continue;
                }

                auto d = format_description(o, longest + OPTION_DESC_GAP, allowed);

                result += fiter->first;
                if (fiter->first.length() > longest) {
                    result += '\n';
                    result += std::string(longest + OPTION_DESC_GAP, ' ');
                } else {
                    result += std::string(longest + OPTION_DESC_GAP - fiter->first.length(), ' ');
                }
                result += d;
                result += '\n';

                ++fiter;
            }

            return result;
        }

        void generate_group_help(std::string& result, const std::vector<std::string>& groups) const {
            for (size_t i = 0; i != print_groups.size(); ++i) {
                const std::string& group_help_text = help_one_group(print_groups[i]);
                if (empty(group_help_text)) {
                    continue;
                }
                result += group_help_text;
                if (i < print_groups.size() - 1) {
                    result += '\n'; 
                }
            }
        }

        void generate_all_group_help(std::string& result) const {
            std::vector<std::string> all_groups;
            all_groups.reserve(m_help.size());

            for (auto& group : m_help) {
                all_groups.push_back(group.first);
            }

            generate_group_help(result, all_groups);
        }

        std::string m_program;
        std::string m_helpString;
        std::string m_positionalHelp;
        std::map<std::string, std::shared_ptr<OptionDetail>> m_options;
        std::vector<std::string> m_positional;
        std::vector<std::string>::iterator m_nextPositional;
        std::unordered_set<std::string> m_positionalSet;

        // mapping from groups to help options
        std::map<std::string HelpGroupDetail> m_help;
    };

    class OptionAdder {
    public:
        OptionAdder(Options& options, std::string group)
            : m_options(options), m_group(std::move(group)) {}
        OptionAdder& operator()(
            const std::string& opts,
            const std::string& desc,
            std::shared_ptr<const ValueInterface> value = ValueInterface<bool>(),
            std::string arg_help = ""
        ) {
            std::match_result<const char*> result;
            std::regex_match<opts.c_str(), result, option_specifier);
            if (result.empty()) {
                throw OptionSpecError(opts, detail::SpecError::InvalidOptionFormat);
            }
            const auto& short_match = result[2];
            const auto& long_match = result[3];
            if (!short_match.length() && long_match.length()) {
                throw OptionSpecError(opts, detail::SpecError::InvalidOptionFormat);
            }
            auto option_names = [](const std::sub_match<const char*>& xshort, const std::sub_match<const char*>& xlong) {
                if (xlong.length() == 1) {
                    return std::make_tuple(xlong.str(), xshort.str());
                } else {
                    return std::make_tuple(xshort.str(), xlong.str());
                }
            }(short_match, long_match);

            m_options.add_option(m_group, std::get<0>(option_names), std::get<0>(option_names), desc, value, std::move(arg_help));
            return *this;
        }
    private:
        Options& m_options;
        std::string m_group;
    };

    void check_required(const Options& options, const std::vector<std::string>& required) {
        for (auto& r: required) {
            if (options.count(r) == 0) {
                throw OptionParseException(r, ParseError::OptionRequired);
            }
        }
    }

   
}


#endif