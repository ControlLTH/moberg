#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <moberg_config_parser.h>
#include <moberg_driver.h>

#define token moberg_config_parser_token
#define token_kind moberg_config_parser_token_kind 
#define acceptsym moberg_config_parser_acceptsym
#define peeksym moberg_config_parser_peeksym

typedef struct moberg_config_parser_token token_t;
typedef struct moberg_config_parser_ident ident_t;

typedef struct moberg_config_parser_context {
  char *buf;      /* Pointer to data to be parsed */
  const char *p;  /* current parse location */
  token_t token;
} context_t;

static const void nextsym_ident_or_keyword(context_t *c)
{
  c->token.u.ident.length = 1;
  c->token.u.ident.value = c->p;
  c->p++;
  while (*c->p) {
    switch (*c->p) {
      case 'a'...'z':
      case 'A'...'Z':
      case '0'...'9':
      case '_':
        c->p++;
        c->token.u.ident.length++;
        break;
      default:
        goto out;
        break;
    }
  }
out: ;
  const char *v = c->token.u.ident.value;
  int l = c->token.u.ident.length;
  if (strncmp("config", v, l) == 0) {
    c->token.kind = tok_CONFIG;
  } else if (strncmp("map", v, l) == 0) {
    c->token.kind = tok_MAP;
  } else if (strncmp("analog_in", v, l) == 0) {
    c->token.kind = tok_ANALOGIN;
  } else if (strncmp("analog_out", v, l) == 0) {
    c->token.kind = tok_ANALOGOUT;
  } else if (strncmp("digital_in", v, l) == 0) {
    c->token.kind = tok_DIGITALIN;
  } else if (strncmp("digital_out", v, l) == 0) {
    c->token.kind = tok_DIGITALOUT;
  } else if (strncmp("encoder_in", v, l) == 0) {
    c->token.kind = tok_ENCODERIN;
  } else {
    c->token.kind = tok_IDENT;
  }
  printf("IDENT: %.*s %d\n", l, v, c->token.kind);
}

static const void nextsym_integer(context_t *c)
{
  c->token.kind = tok_INTEGER;
  c->token.u.integer.value = 0;
  while (*c->p && '0' <= *c->p && *c->p <= '9') {
    c->token.u.integer.value *= 10;
    c->token.u.integer.value += *c->p - '0';
    c->p++;
  }
  printf("INTEGER: %d\n", c->token.u.integer.value);
}

static const void nextsym_string(context_t *c)
{
  printf("STTRING");
  c->token.kind = tok_STRING;
  c->p++;
  c->token.u.string.value = c->p;
  c->token.u.string.length = 0;
  while (*c->p && *c->p != '"') {
    if (*c->p == '\\') {
      c->token.u.string.length++;
      c->p++;
    }
    if (*c->p) {
      c->token.u.string.length++;
      c->p++;
    }
  }
  c->p++;
  printf("STRING: %.*s\n", c->token.u.string.length, c->token.u.string.value);
}

static int nextsym(context_t *c)
{
  c->token.kind = tok_none;
  while (c->p && *c->p && c->token.kind == tok_none) {
    if (c->p[0] == '/' && c->p[1] == '*') {
      printf("Skipping COMMENT\n");
      /* Skip comment */
      c->p += 2;
      while (*c->p && (c->p[0] != '*' || c->p[1] != '/')) {
        c->p++;
      }
      c->p += 2;
      continue;
    }
    switch (*c->p) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        /* Skip whitespace */
        c->p++;
        break;
      case '{':
        c->token.kind = tok_LBRACE;
        c->p++;
        break;
      case '}':
        c->token.kind = tok_RBRACE;
        c->p++;
        break;
      case '[':
        c->token.kind = tok_LBRACKET;
        c->p++;
        break;
      case ']':
        c->token.kind = tok_RBRACKET;
        c->p++;
        break;
      case '=':
        c->token.kind = tok_EQUAL;
        c->p++;
        break;
      case ':':
        c->token.kind = tok_COLON;
        c->p++;
        break;
      case ';':
        c->token.kind = tok_SEMICOLON;
        c->p++;
        break;
      case '"':
        nextsym_string(c);
        break;
      case 'a'...'z':
      case 'A'...'Z':
      case '_':
        nextsym_ident_or_keyword(c);
        break;
      case '0'...'9':
        nextsym_integer(c);
        break;
      default:
        printf("UNEXPECTED %c\n\n", *c->p);
        c->p++;
        break;
    }
  }
  printf("TOKEN %d %c\n\n", c->token.kind, c->token.kind<255?c->token.kind:' ');
  if (c->token.kind != tok_none) {
    return 1;
  } else {
    return 0;
  }
}

int moberg_config_parser_acceptsym(context_t *c,
                                   enum token_kind kind,
                                   token_t *token)
{
  if (c->token.kind == kind) {
    printf("ACCEPT %d", c->token.kind);
    if (token) {
      *token = c->token;
    }
    nextsym(c);
    return 1;
  }
  printf("REJECT %d (%d)", kind, c->token.kind);
  return 0;
}

int moberg_config_parser_peeksym(context_t *c,
				 token_t *token)
{
  if (token) {
    *token = c->token;
  }
  return *c->p != 0;
}

static int parse_map_range(context_t *c)
{
  token_t low, high;
  if (! acceptsym(c, tok_LBRACKET, NULL)) { goto err; }
  if (! acceptsym(c, tok_INTEGER, &low)) { goto err; }
  if (acceptsym(c, tok_COLON, NULL)) {
    if (! acceptsym(c, tok_INTEGER, &high)) { goto err; }
  } else {
    high = low;
  }
  if (! acceptsym(c, tok_RBRACKET, NULL)) { goto err; }
  return 1;
err:
  printf("OOPS");
  return 0;
}

static int parse_map(context_t *c,
                     struct moberg_driver *driver)
{
  printf("parsemap");
  struct token t;
  if (acceptsym(c, tok_ANALOGIN, &t) ||
      acceptsym(c, tok_ANALOGOUT, &t) ||
      acceptsym(c, tok_DIGITALIN, &t) ||
      acceptsym(c, tok_DIGITALOUT, &t) ||
      acceptsym(c, tok_ENCODERIN, &t)) {
    if (! parse_map_range(c)) { goto err; }
    if (! acceptsym(c, tok_EQUAL, NULL)) { goto err; }
    driver->module.parse_map(c, t.kind);
    if (! parse_map_range(c)) { goto err; }
    if (! acceptsym(c, tok_SEMICOLON, NULL)) { goto err; }
    switch (t.kind) {
      case tok_ANALOGIN:
        
      case tok_ANALOGOUT:
        
      case tok_DIGITALIN:

      case tok_DIGITALOUT:
        
      case tok_ENCODERIN:
        break;
      default:
        goto err;
    }
  } else {
    goto err;
  }
  return 1;
err:    
  return 0;
}

static int parse_device(context_t *c,
                        struct moberg_driver *driver)
{
  if (! acceptsym(c, tok_LBRACE, NULL)) { goto err; }
  for (;;) {
    struct token t;
    peeksym(c, &t);
    printf("PEEK %d", t.kind);
    if (acceptsym(c, tok_CONFIG, NULL)) {
      driver->module.parse_config(c);
    } else if (acceptsym(c, tok_MAP, NULL)) {
      parse_map(c, driver);
    } else if (acceptsym(c, tok_RBRACE, NULL)) {
      break;
    } else {
      goto err;
    }
  }
  return 1;
err:
  return 0;
}

static int parse_driver(context_t *c,
                        ident_t name)
{
  struct moberg_driver *driver = moberg_open_driver(name);
  if (! driver) {
    printf("Driver not found\n");
    goto err;
  } else {
    int OK = parse_device(c, driver);
    moberg_close_driver(driver);
    if (! OK) { goto err; } 
  }
  return 1;
err:
  return 0;
}

static int parse_config(context_t *c)
{
  for (;;) {
    struct token t;
    peeksym(c, &t);
    printf("PEEK %d", t.kind);
    if (acceptsym(c, tok_IDENT, &t)) {
      printf("DRIVER=%.*s\n", t.u.ident.length, t.u.ident.value);
      if (! parse_driver(c, t.u.ident)) { goto err; }
    }
  }
  return 1;
err:
  printf("Failed!!");
  exit (1);
  return 0;
}

static char *read_fd(int fd)
{
  char *result = malloc(10000); /* HACK */
  int pos = read(fd, result, 10000);
  result[pos] = 0;
  return result;
}

int main(int argc, char *argv[])
{
  context_t context;
  int fd = open(argv[1], O_RDONLY);
  context.buf = read_fd(fd);
  context.p = context.buf;
  close(fd);
  printf("%s\n", context.buf);
  nextsym(&context);
  parse_config(&context);
  free(context.buf);
}
