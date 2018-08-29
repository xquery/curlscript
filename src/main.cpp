/******************************************************************************
 * curlscript - https://github.com/xquery/curlscript
 ******************************************************************************
 * Copyright (c) 2017-2018 James Fuller <jim.fuller@webcomposite.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
******************************************************************************/

#include <cxxopts.hpp>
#include <vector>
#include <numeric>
#include <functional>
#include <iostream>

#include <curlscript/curlscript.cpp>

using namespace std;

int banner(){
    cout << "curlscript " << get_version() << " | ⓒ 2017-2018 James Fuller <jim.fuller@webcomposite.com> | https://github.com/xquery/curlscript" << endl;
    return CS_OK;
}

int usage(){
    banner();
    cout << "\n> curlscript mycurlscript.cs \n\n"
        << "    -h | --help  : help\n"
        << "    -d | --debug : emit debug info ()\n"
        << "    -i | --info  : emit info\n"
        << "    -q | --quiet : suppress output to console\n"
        << "    -f | --file  : alternate curlscript file\n" << endl;
    return CS_OK;
}

cxxopts::Options setopts(){
    cxxopts::Options opts("curlscript", "http scripting with curl.");
    opts.add_options()
            ("positional",
             "Positional arguments: these are the arguments that are entered "
             "without an option", cxxopts::value<std::vector<std::string>>())
            ("d,debug", "Emit debug level logging")
            ("l,log", "Enable logging to file", cxxopts::value<string>())
            ("s,serialiser", "Switch serialiser",cxxopts::value<string>())
            ("q,quiet", "Disable output")
            ("h,help", "help")
            ("i,info", "Emit info level logging")
            ("f,file", "File name", cxxopts::value<string>());
    opts.parse_positional({"input", "output", "positional"});
    return opts;
}

int main(int argc, char** argv ){

#ifndef NDEBUG
    DLOG_S(INFO) << "debug mode";
#endif

    cxxopts::Options opts = setopts();
    auto result = opts.parse(argc,argv);

    if(result["help"].count() == 1){
        usage();
        return EXIT_SUCCESS;}

    if(result["log"].count() == 1){
        set_log_file(result["log"].as<string>());
        DLOG_S(INFO) << "set logging to " << result["log"].as<string>(); }

    DLOG_S(INFO) << "start processing...";
    if(result["info"].count() == 1){
        banner();
        set_log_verbosity_info();
    }else{
        set_log_verbosity_error();}

    if(result["debug"].count() == 1){
        set_log_verbosity_max();}

    if(result["positional"].count() == 1){
        DLOG_S(INFO) << "set positional opt";
        auto& v = result["positional"].as<std::vector<std::string>>();
        for (const auto& s : v) {
            curlscript::exec(s, result["quiet"].count());}
    } else{
        if(result["file"].count() == 1){
            DLOG_S(INFO) << "set file opt";
            curlscript::exec(result["file"].as<string>(), result["quiet"].count());
        }else{
            LOG_S(ERROR) << "must supply curlscript file.";
            return EXIT_FAILURE; }}

    DLOG_S(INFO) << "finished processing, dumping output to stdout.";
    return EXIT_SUCCESS;
}
