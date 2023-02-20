#pragma once
#include <stdio.h>
#include <stdbool.h>
#define MAXCHILDREN 3
/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 8
#define ENDFILE 0
/* Tokens.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
enum yytokentype {
    ID = 258,
    NUM = 259,
    LESS_EQUAL_THAN = 260,
    LESS_THAN = 261,
    GREAT_THAN = 262,
    GREAT_EQUAL_THAN = 263,
    DOUBLE_EQUAL = 264,
    NOT_EQUAL = 265,
    KEYWORD_ELSE = 266,
    KEYWORD_IF = 267,
    KEYWORD_INT = 268,
    KEYWORD_RETURN = 269,
    KEYWORD_VOID = 270,
    KEYWORD_WHILE = 271,
    MULTI_LINE_ANNOTATION = 272,
    KEYWORD_INPUT=273,
    KEYWORD_OUTPUT=274,
    ERROR=275,
    ASSIGN=276,
    PLUS=277,
    MINUS=278,
    MUTIPLY=279,
    DIVIDE=280,
    L_BRACE=281,
    R_BRACE=282,
    L_PARENTHESIS=283,
    R_PARENTHESIS=284,
    SEMICOLON=285,
    COMMA=286
};
#endif
/* Tokens.  */
#define ID 258
#define NUM 259
#define LESS_EQUAL_THAN 260
#define LESS_THAN 261
#define GREAT_THAN 262
#define GREAT_EQUAL_THAN 263
#define DOUBLE_EQUAL 264
#define NOT_EQUAL 265
#define KEYWORD_ELSE 266
#define KEYWORD_IF 267
#define KEYWORD_INT 268
#define KEYWORD_RETURN 269
#define KEYWORD_VOID 270
#define KEYWORD_WHILE 271
#define MULTI_LINE_ANNOTATION 272
#define KEYWORD_INPUT 273
#define KEYWORD_OUTPUT 274
#define ERROR 275
#define ASSIGN 276
#define PLUS 277
#define MINUS 278
#define MUTIPLY 279
#define DIVIDE 280
#define L_BRACE 281
#define R_BRACE 282
#define L_PARENTHESIS 283
#define R_PARENTHESIS 284
#define SEMICOLON 285
#define COMMA 286

typedef enum { StmtK, ExpK,ParamsK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK, DeclarationK, ReturnK, StatementK,Callk } StmtKind;
typedef enum { OpK, ConstK, IdK } ExpKind;
typedef enum { ParamListK,Param } ParamsKind;
/* ExpType is used for type checking */
typedef enum { Void, Integer, Boolean } ExpType;
typedef int TokenType;
typedef struct treeNode {
    struct treeNode* child[MAXCHILDREN];
    struct treeNode* sibling;
    int lineno;
    NodeKind nodekind;
    struct { StmtKind stmt; ExpKind exp; ParamsKind params; } kind;
    struct {
        TokenType op;
        int val;
        char* name;
    } attr;
    ExpType type; /* for type checking of exps */
} TreeNode;
typedef struct tokenInfo {
    TokenType token;
    char tokenString[40+1];
    int lineno;
    struct tokenInfo* last;
    struct tokenInfo* next;
} tokenInfo;

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */
/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern bool EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern bool TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern bool TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern bool TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern bool TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern bool Error;