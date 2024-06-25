#ifndef TOKEN_H
#define TOKEN_H

#define INIT_TOK_SIZE 8

#include "../../lib/vector.h"

typedef struct Token {
    Vector* value;
    int len;
    int line;
    int tok_beg;
}Token;

void init_token(Token* tok);


#endif
