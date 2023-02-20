/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "scan.h"
#include <stdbool.h>
#include "util.h"
#include <string.h>

/* states in scanner DFA */
typedef enum {
    START, INASSIGN,PREINCOMMENT, INCOMMENT, INNUM, INID, DONE
}
StateType;



/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];


/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static bool EOF_flag = false; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
   /**
    * @brief 不断获取字符
    * 先获取一整行，然后一个一个读
    *
    *
    * @return 返回读取到的字符
    **/
static int getNextChar(void) {
    //本行已读取完 
    if (!(linepos < bufsize)) {
        //行号增加 
        lineno++;
        //从源代码文件中读取一行数据，字符数目不超过BUFLEN 
        if (fgets(lineBuf, BUFLEN - 1, source)) {

            if (EchoSource) {
                fprintf(listing, "%4d: %s", lineno, lineBuf);
            }
            //当前行的字符数 
            bufsize = strlen(lineBuf);
            //当前行的扫描指针的位置 
            linepos = 0;
            return lineBuf[linepos++];
        }
        else {
            //文件读取完了 
            EOF_flag = true;
            return EOF;
        }
    }
    else {
        return lineBuf[linepos++];
    }
}

/* ungetNextChar backtracks one character
   in lineBuf */
   /**
    * @brief 回溯上一个字符
    *
    *
    * @return
    **/
static void ungetNextChar(void) {
    if (!EOF_flag) {
        linepos--;
    }
}

/* lookup table of reserved words */
/**
 * 预置的关键字
 **/
static struct {
    char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
        {"if",KEYWORD_IF},
        {"void",KEYWORD_VOID},
        {"else",KEYWORD_ELSE},
        {"int",KEYWORD_INT},
        {"return",KEYWORD_RETURN},
        {"while",KEYWORD_WHILE},
        {"input",KEYWORD_INPUT},
        {"output",KEYWORD_OUTPUT}
};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
/**
 * @brief 看预置关键字中是否存在该字符串
 *
 * @param s 需要判断的字符串
 *
 * @return
 **/
static TokenType reservedLookup(char* s) {
    int i;
    for (i = 0; i < MAXRESERVED; i++) {
        if (!strcmp(s, reservedWords[i].str)) {
            return reservedWords[i].tok;
        }
    }
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void){  
    /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;
    while (state != DONE){
        int c = getNextChar();
        save = true;
        int next;
        switch (state)
        {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == '=')
                state = INASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = false;
            else if (c == '/'){
                next = getNextChar();
                if (isdigit(next)|| isalpha(next)|| (next == ' ') || (next == '\t') || (next == '\n')) {
                    state = DONE;
                    currentToken = DIVIDE;
                    ungetNextChar();
                    break;
                }
                else if(next == '*') {
                    save = false;
                    state = INCOMMENT;
                    break;
                }
            }
            else {
                state = DONE;
                switch (c) {
                case EOF:
                    save = false;
                    currentToken = ENDFILE;
                    break;
                case '<':
                    next = getNextChar();
                    if (next != '=') {
                        ungetNextChar();
                        currentToken = LESS_THAN;
                    }
                    else {
                        currentToken = LESS_EQUAL_THAN;
                    }
                    break;
                case '>':
                    next = getNextChar();
                    if (next != '=') {
                        ungetNextChar();
                        currentToken = GREAT_THAN;
                    }
                    else {
                        currentToken = GREAT_EQUAL_THAN;
                    }
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = MUTIPLY;
                    break;
                case '/':
                    currentToken = DIVIDE;
                    break;
                case '(':
                    currentToken = L_PARENTHESIS;
                    break;
                case ')':
                    currentToken = R_PARENTHESIS;
                    break;
                case '{':
                    currentToken = L_BRACE;
                    break;
                case '}':
                    currentToken = R_BRACE;
                    break;
                case ';':
                    currentToken = SEMICOLON;
                    break;
                case ',':
                    currentToken = COMMA;
                    break;
                case '!':
                    next = getNextChar();
                    if (next != '=') {
                        ungetNextChar();
                        currentToken = ERROR;
                    }
                    else {
                        currentToken = NOT_EQUAL;
                    }
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = false;
            if (c == EOF){
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '*') {
                int next = getNextChar();
                if (next == '/') {
                    state = START;
                }
            }
            break;
        case INASSIGN:
            if (c == '=') {
                currentToken = DOUBLE_EQUAL;
                state = DONE;
            }
            else if (isdigit(c)||isalpha(c) || (c == ' ') || (c == '\t') || (c == '\n')) {
                currentToken = ASSIGN;
                ungetNextChar();
                c = '=';
                state = DONE;
            } else{
                /* backup in the input */
                ungetNextChar();
                save = false;
                currentToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c))
            { /* backup in the input */
                ungetNextChar();
                save = false;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            { /* backup in the input */
                ungetNextChar();
                save = false;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default: /* should never happen */
            fprintf(listing, "Scanner Bug: state= %d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE){
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    return currentToken;
} /* end getToken */

void createTokenList() {
    head = (tokenInfo*)malloc(sizeof(tokenInfo));
    point = head;
    TokenType token = getToken();
    while (token != ENDFILE) {
        token = getToken();
    }
    point = head;
}

tokenInfo getNextToken() {
    if (point->next != NULL) {
        point = point->next;
        lineno = point->lineno;
        return *point;
    }
    tokenInfo info = { ENDFILE ,NULL,NULL,NULL };
    return info;
}

tokenInfo getLastToken() {
    if (point != NULL) {
        point = point->last;
        lineno = point->lineno;
        return (*point);
    }
    tokenInfo info = { ENDFILE ,NULL,NULL,NULL };
    return info;
}