/**
 * Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
 * License: your favourite BSD-style license
 *
 * git clone http://github.com/weisslj/cpp-optparse.git
 *
 * This is yet another option parser for C++. It is modelled after the
 * excellent Python optparse API. Although incomplete, anyone familiar to
 * optparse should feel at home:
 * http://docs.python.org/library/optparse.html
 *
 * Design decisions:
 * - elegant and easy usage more important than speed / flexibility
 * - shortness more important than feature completeness
 *   * no unicode
 *   * no checking for user programming errors
 *
 * Why not use getopt/getopt_long?
 * - not C++ / not completely POSIX
 * - too cumbersome to use, would need lot of additional code
 *
 * Why not use Boost.Program_options?
 * - boost not installed on all target platforms (esp. cluster, HPC, ...)
 * - too big to include just for option handling:
 *   322 *.h (44750 lines) + 7 *.cpp (2078 lines)
 *
 * Why not use tclap/Opag/Options/CmdLine/Anyoption/Argument_helper/...?
 * - no reason, writing one is faster than code inspection :-)
 * - similarity to Python desired for faster learning curve
 *
 * Future work:
 * - option groups
 * - callbacks?
 * - code simplification / cleanup
 * - comments?
 *
 *
 * Example:
 *
 * using optparse::OptionParser;
 *
 * OptionParser parser = OptionParser() .description("just an example");
 *
 * parser.add_option("-f", "--file") .dest("filename")
 *                   .help("write report to FILE") .metavar("FILE");
 * parser.add_option("-q", "--quiet")
 *                   .action("store_false") .dest("verbose") .set_default("1")
 *                   .help("don't print status messages to stdout");
 * 
 * optparse::Values options = parser.parse_args(argc, argv);
 * vector<string> args = parser.args();
 *
 * if (options.get("verbose"))
 *     cout << options["filename"] << endl;
 *
 */

#ifndef OPTIONPARSER_H_
#define OPTIONPARSER_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <sstream>

namespace optparse {

//! Class for automatic conversion from string -> anytype
class Value {
  public:
    Value(const std::string& v) : s(v) {}
    operator const char*() { return s.c_str(); }
    operator bool() { bool t; std::istringstream(s) >> t; return t; }
    operator short() { short t; std::istringstream(s) >> t; return t; }
    operator unsigned short() { unsigned short t; std::istringstream(s) >> t; return t; }
    operator int() { int t; std::istringstream(s) >> t; return t; }
    operator unsigned int() { unsigned int t; std::istringstream(s) >> t; return t; }
    operator long() { long t; std::istringstream(s) >> t; return t; }
    operator unsigned long() { unsigned long t; std::istringstream(s) >> t; return t; }
    operator float() { float t; std::istringstream(s) >> t; return t; }
    operator double() { double t; std::istringstream(s) >> t; return t; }
    operator long double() { long double t; std::istringstream(s) >> t; return t; }
 private:
    const std::string s;
};

typedef std::map<std::string,std::string> strMap;

class Values {
  public:
    const std::string& operator[] (const std::string& d) const;
    std::string& operator[] (const std::string& d);
    bool is_set(const std::string& d) const;
    Value get(const std::string& d) const;
  private:
    strMap _map;
};

class Option {
  public:
    Option() : _action("store"), _type("string"), _nargs(1) {}

    Option& action(const std::string& a);
    Option& type(const std::string& t) { _type = t; return *this; }
    Option& dest(const std::string& d) { _dest = d; return *this; }
    Option& set_default(const std::string& d) { _default = d; return *this; }
    Option& nargs(size_t n) { _nargs = n; return *this; }
    Option& set_const(const std::string& c) { _const = c; return *this; }
    template<typename InputIterator>
    Option& choices(InputIterator begin, InputIterator end) {
      _choices.assign(begin, end); type("choice"); return *this;
    }
    Option& help(const std::string& h) { _help = h; return *this; }
    Option& metavar(const std::string& m) { _metavar = m; return *this; }

    const std::string& action() const { return _action; }
    const std::string& type() const { return _type; }
    const std::string& dest() const { return _dest; }
    const std::string& get_default() const { return _default; }
    size_t nargs() const { return _nargs; }
    const std::string& get_const() const { return _const; }
    const std::list<std::string>& choices() const { return _choices; }
    const std::string& help() const { return _help; }
    const std::string& metavar() const { return _metavar; }

  private:
    std::string check_type(const std::string& opt, const std::string& val) const;
    std::string format_option_help() const;
    std::string format_help() const;

    std::set<std::string> _short_opts;
    std::set<std::string> _long_opts;

    std::string _action;
    std::string _type;
    std::string _dest;
    std::string _default;
    size_t _nargs;
    std::string _const;
    std::list<std::string> _choices;
    std::string _help;
    std::string _metavar;

    friend class OptionParser;
};

typedef std::map<std::string,Option*> optMap;

class OptionParser {
  public:
    OptionParser();

    OptionParser& usage(const std::string& u) { set_usage(u); return *this; }
    OptionParser& version(const std::string& v) { _version = v; return *this; }
    OptionParser& description(const std::string& d) { _description = d; return *this; }
    OptionParser& add_help_option(bool h) { _add_help_option = h; return *this; }
    OptionParser& add_version_option(bool v) { _add_version_option = v; return *this; }
    OptionParser& prog(const std::string& p) { _prog = p; return *this; }
    OptionParser& epilog(const std::string& e) { _epilog = e; return *this; }
    OptionParser& set_defaults(const std::string& dest, const std::string& val) {
      _defaults[dest] = val; return *this;
    }

    const std::string& usage() const { return _usage; }
    const std::string& version() const { return _version; }
    const std::string& description() const { return _description; }
    bool add_help_option() const { return _add_help_option; }
    bool add_version_option() const { return _add_version_option; }
    const std::string& prog() const { return _prog; }
    const std::string& epilog() const { return _epilog; }

    Option& add_option(const std::string& opt);
    Option& add_option(const std::string& opt1, const std::string& opt2);
    Option& add_option(const std::string& opt1, const std::string& opt2, const std::string& opt3);
    Option& add_option(const std::vector<std::string>& opt);

    Values& parse_args(int argc, char const* const* argv);
    Values& parse_args(const std::vector<std::string>& args);
    template<typename InputIterator>
    Values& parse_args(InputIterator begin, InputIterator end) {
      return parse_args(std::vector<std::string>(begin, end));
    }

    const std::list<std::string>& args() const { return _leftover; }
    std::vector<std::string> args() {
      return std::vector<std::string>(_leftover.begin(), _leftover.end());
    }

    std::string format_help() const;
    std::string format_option_help() const;
    void print_help() const;

    void set_usage(const std::string& u);
    std::string get_usage() const;
    void print_usage(std::ostream& out) const;
    void print_usage() const;

    std::string get_version() const;
    void print_version(std::ostream& out) const;
    void print_version() const;

    void error(const std::string& msg) const;
    void exit() const;

  private:
    const Option& lookup_short_opt(const std::string& opt) const;
    const Option& lookup_long_opt(const std::string& opt) const;

    void handle_short_opt(const std::string& opt, const std::string& arg);
    void handle_long_opt(const std::string& optstr);

    void process_opt(const Option& option, const std::string& opt, const std::string& value);

    std::string format_usage(const std::string& u) const;

    std::string _usage;
    std::string _version;
    std::string _description;
    bool _add_help_option;
    bool _add_version_option;
    std::string _prog;
    std::string _epilog;

    Values _values;

    std::list<Option> _opts;
    optMap _optmap_s;
    optMap _optmap_l;
    strMap _defaults;

    std::list<std::string> _remaining;
    std::list<std::string> _leftover;
};

}

#endif