#ifndef __MOBERG_CONFIG_PARSER_H__
#define __MOBERG_CONFIG_PARSER_H__

#include <stdio.h>

struct moberg_config_parser_context;
struct moberg_config_parser_token;

enum moberg_config_parser_token_kind {
  tok_none,
  tok_LBRACE = '{',
  tok_RBRACE = '}',
  tok_LBRACKET = '[',
  tok_RBRACKET = ']',
  tok_EQUAL = '=',
  tok_COLON = ':',
  tok_SEMICOLON = ';',
  tok_INTEGER = 256,
  tok_IDENT,
  tok_STRING,
};

struct moberg_config_parser_ident {
  int length;
  const char *value;
};

struct moberg_config_parser_token {
  enum moberg_config_parser_token_kind kind;
  union {
    struct moberg_config_parser_ident ident;
    struct moberg_config_parser_ident string;
    struct moberg_config_parser_integer {
      int value;
    } integer;
  } u;
};

int moberg_config_parser_acceptsym(
  struct moberg_config_parser_context *c,
  enum moberg_config_parser_token_kind kind,
  struct moberg_config_parser_token *token);

int moberg_config_parser_acceptkeyword(
  struct moberg_config_parser_context *c,
  const char *keyword);

void moberg_config_parser_failed(
  struct moberg_config_parser_context *c,
  FILE *f);

#endif