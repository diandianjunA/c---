#pragma once
/* MAXTOKENLEN is the maximum size of a token */
#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];
tokenInfo* head;
tokenInfo* point;
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void);

tokenInfo getNextToken();

tokenInfo getLastToken();

void createTokenList();