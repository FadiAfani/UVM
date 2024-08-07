#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error {
    std::string file;
    std::string msg;
    int err_line;
    int err_beg;

    public:
        std::string to_string();
        Error();
        Error(const char* _file, const char* _msg, int _err_line);
        Error(const char* _file, const char* _msg, int _err_line, int _err_beg);
        void set_file(const char* _file);
        void set_msg(const char* _msg);
        void set_err_line(int _err_line);
        void set_err_beg(int _err_beg);

};


#endif
