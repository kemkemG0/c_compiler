#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
  TK_RESERVED,// symbol
  TK_NUM, //integer
  TK_EOF,// end of input
} TokenKind;

typedef struct Token Token;
// Token type
struct Token{
  TokenKind kind; //type of token
  Token *next; // next coming token
  int val; //when kind isTK_NUM, use this
  char *str; //token string
};

// Input program
char *user_input;
// current token
Token *token;


// Reports and error and exit.
void error(char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
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
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

int expect_number(){
  if(token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

// create new token and connect to current one
Token *new_token(TokenKind kind, Token *cur, char *str){
  // Allocate and zero-initialize array
  Token *tok = calloc(1,sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}


// input cstring'p' to token
Token *tokenize(){
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    //ignore space
    if(isspace(*p)){
      ++p;
      continue;
    }
    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED,cur,p++);
      continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM,cur,p);
      cur->val = strtol(p,&p,10);// 123abc to 123 and p points at 'a'
      continue;
    }
    error_at(p, "expected a number");
  }

  new_token(TK_EOF,cur,p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Numver of args is wrong.");
    return 1;
  }
  

  user_input = argv[1];
  token = tokenize(user_input);

  //First parts of assembly
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // The first of statements has to be a number.
  // so check it and print it.
  printf("  mov rax, %d\n", expect_number());

  // "Consuming + <number>" or "- <number>", generate Assembly
  while(!at_eof()){
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
