#include "../include/error.hpp"
#include <string>

Error::Error() {}

Error::Error(std::string _file, std::string _msg, int _err_line, int _err_beg) {
    file = _file;
    msg = _msg;
    err_line = _err_line;
    err_beg = _err_beg;
}

Error::Error(std::string _file, std::string _msg, int _err_line) {
    file = _file;
    msg = _msg;
    err_line = _err_line;
}

void Error::set_file(std::string _file) {
    file = _file;
}

void Error::set_msg(std::string _msg) {
    msg = _msg;
}

void Error::set_err_line(int _err_line) {
    err_line = _err_line;
}

void Error::set_err_beg(int _err_beg) {
    err_beg = _err_beg;
}

std::string Error::to_string() {
    return file + ": " + msg + " at " + std::to_string(err_line) + ":" + std::to_string(err_beg);
}


