#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED, // symbol
  TK_NUM,      // integer
  TK_EOF,      // end of input
} TokenKind;

typedef struct Token Token;
// Token type
struct Token {
  TokenKind kind; // type of token
  Token *next;    // next coming token
  int val;        // when kind isTK_NUM, use this
  char *str;      // token string
  int len;        // length of token
};

// Kind of nodes of abstract syntax tree
typedef enum {
  ND_ADD, //+
  ND_SUB, //-
  ND_MUL, //*
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // integer
} NodeKind;

// Node of absctact syntac tree
typedef struct Node Node;
struct Node {
  NodeKind kind; // type of node
  Node *lhs;     // left-hand side
  Node *rhs;     // right-hand side
  int val;       // when kind is ND_NUM
};

// Input program
char *user_input;
// current token
Token *token;

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
bool consume(char *op);
void expect(char *op);
int expect_number();

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// expr = equality
Node *expr() { return equality(); }

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  /*
    Generate rule:
      mul = unary ("*" unary | "/" unary)*
  */
  Node *node = unary();
  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *primary() {
  /*
    Generate rule:
      primary = "(" expr ")" | num
  */
  // if the next token is '(',
  // '(' expr ') should follow.
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  // otherwise it should be a number
  return new_node_num(expect_number());
}

Node *unary() {
  // unary   = ("+" | "-")? primary
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_node_num(0), unary());
  return primary();
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

// Reports and error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
void error_at(char *err_loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = err_loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// if the next token is the operand which you expect,
// increment token and return true. Otherwise return false.
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// create new token and connect to current one
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  // Allocate and zero-initialize array
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

// input cstring'p' to token
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // ignore space
    if (isspace(*p)) {
      ++p;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    // ()*()
    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Numver of args is wrong.");
    return 1;
  }

  // Tokenize and parse codes
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // First parts of assembly
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Trace abstract tree recursively and generate code
  gen(node);

  // return the last result
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
