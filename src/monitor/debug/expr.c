#include "expr.h"
#include <isa.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NUM

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus
    {"==", TK_EQ},     // equal
    {"\\-", '-'},      {"\\*", '*'}, {"\\/", '/'},
    {"\\(", '('},      {"\\)", ')'}, {"[0-9]+", TK_NUM},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e, int *token_len) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        tokens[*token_len].type = rules[i].token_type;
        switch (rules[i].token_type) {
        default:
          tokens[(*token_len)++].str[0] = rules[i].token_type;
          break; // by defalut, the tokens should be character instead of num
        case TK_NUM:
          for (int j = 0; j <= substr_len; j++) {
            tokens[(*token_len)++].str[j] = e[position + j];
          }
          break;
        }
        position += substr_len;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

word_t expr(char *e, bool *success) {
  int token_len = 0;
  if (!make_token(e, token_len)) {
    *success = false;
    return 0;
  } else {
    *success = true;
    return eval(0, token_len);
  }
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  return 0;
}

bool check_parentheses(int p, int q) {
  if (strcmp(tokens[p].str, "(") && strcmp(tokens[q].str, ")")) {
    int stack_match = 0;
    for (int pos = 0; pos < q - p; pos++) {
      switch (tokens[p + pos].str[0]) {
      case '(':
        stack_match++;
        break;
      case ')':
        stack_match--;
        break;
      }
      return stack_match == 0;
    }
  }
  return false;
}

word_t eval(p, q) {
  if (p > q) {
    assert(0);
  } else if (p == q) {
    return atoi(tokens[q].str);
  } else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  } else {
    /*    op = the position of 主运算符 in the token expression;
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);*/
    int op;
    bool priority = true;
    for (int i = p; i < q; i++) {
      if (tokens[i].type == '+' || tokens[i].type == '-') {
        op = i;
        priority = false;
      } else if (priority && (tokens[i].type == '*' || tokens[i].type == '/')) {
        op = i;
      }
    }
    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);
    switch (tokens[op].type) {
    case '+':
      return val1 + val2;
      break;
    case '-':
      return val1 - val2;
      break;
    case '*':
      return val1 * val2;
      break;
    case '/':
      return val1 / val2;
      break;
    default:
      assert(0);
      break;
    }
  }
}