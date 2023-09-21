#ifndef PARSE_H_
#define PARSE_H_

int parse_simple_int(const char *begin, const char *end, char **restrict next_o);

float parse_simple_float(const char *begin, const char *end, char **restrict next_o);


static inline int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

static inline int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

#endif // PARSE_H_
