#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
TokenType token; /* holds current token */
struct tokenInfo tokenInfos;

/* function prototypes for recursive calls */
TreeNode* stmt_sequence(void);
TreeNode* statement(void);
TreeNode* if_stmt(void);
TreeNode* repeat_stmt(void);
TreeNode* assign_stmt(void);
TreeNode* read_stmt(void);
TreeNode* write_stmt(void);
TreeNode* exps(void);
TreeNode* simple_exp(void);
TreeNode* term(void);
TreeNode* factor(void);
TreeNode* declaration_stmt(void);
TreeNode* return_stmt(void);

/*
* ÓÃÀ´Êä³ö´íÎó
*/
static void syntaxError(const char* message) {
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = true;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
/*
* ·µ»ØÒ»¿Ã³éÏóÓï·¨Ê÷
*/
TreeNode* parse(void) {
    TreeNode* t;
    createTokenList();
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    t = stmt_sequence();
    if (token != ENDFILE) {
        syntaxError("Code ends before file\n");
    }
    return t;
}

/*
* ±È¶Ôtoken
*/
bool match(TokenType expected) {
    if (token == expected) {
        tokenInfos = getNextToken();
        token = tokenInfos.token;
        return true;
    } else {
        return false;
    }
}
/*
* Ñ­»·»ñÈ¡µ½ËùÓÐÓï¾ä
*/
TreeNode* stmt_sequence(void) {
    TreeNode* stmt = newStmtNode(StatementK);
    TreeNode* t = statement();
    TreeNode* p = t;
    stmt->child[0] = p;
    while ((token != ENDFILE)) {
        TreeNode* q;
        match(SEMICOLON);
        q = statement();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return stmt;
}

TreeNode* stmt_list(void) {
    TreeNode* stmt = newStmtNode(StatementK);
    TreeNode* t = statement();
    TreeNode* p = t;
    stmt->child[0] = p;
    while ((token != R_BRACE)) {
        TreeNode* q;
        match(SEMICOLON);
        q = statement();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return stmt;
}

TreeNode* statement(void) {
    TreeNode* t = NULL;
    switch (token) {
        //ifÓï¾ä
        case KEYWORD_IF: t = if_stmt(); break;
        //whileÓï¾ä
        case KEYWORD_WHILE: t = repeat_stmt(); break;
        //¸³ÖµÓï¾ä
        case ID: 
            match(ID);
            if (token == L_PARENTHESIS) {
                tokenInfos = getLastToken();
                token = tokenInfos.token;
                t = newStmtNode(Callk);
                t->attr.name = copyString(tokenInfos.tokenString);
                match(ID);
                if (match(L_PARENTHESIS)) {
                    if (!match(R_PARENTHESIS)) {
                        t->child[0] = factor();
                        TreeNode* p = t->child[0];
                        while (match(COMMA)) {
                            TreeNode* q;
                            q = factor();
                            if (q != NULL) {
                                p->sibling = q;
                                p = q;
                            }
                        }
                        match(R_PARENTHESIS);
                    }
                }
            }
            else {
                tokenInfos = getLastToken();
                token = tokenInfos.token;
                t = assign_stmt();
            }
            break;
        //Êä³öÓï¾ä
        case KEYWORD_INPUT: t = read_stmt(); break;
        //ÊäÈëÓï¾ä
        case KEYWORD_OUTPUT: t = write_stmt(); break;
        //ÉùÃ÷Óï¾ä
        case KEYWORD_INT:
        case KEYWORD_VOID:
            t = declaration_stmt(); break;
        //·µ»ØÓï¾ä
        case KEYWORD_RETURN:
            t = return_stmt(); break;
        //²»ÊÇÒÔÉÏÊöÈÎºÎ¹Ø¼ü×Ö¿ªÍ·£¬ÔòÓï¾ä´íÎó
    } 
    /* end case */
    return t;
}
/*
* ifÓï¾ä
*/
TreeNode* if_stmt(void) {
    TreeNode* t = newStmtNode(IfK);
    if (match(KEYWORD_IF)) {
        if (match(L_PARENTHESIS)) {
            if (t != NULL) t->child[0] = exps();
            if (match(R_PARENTHESIS)) {
                if (match(L_BRACE)) {
                    if (t != NULL) t->child[1] = stmt_list();
                    if (!match(R_BRACE)) {
                        syntaxError("unexpected token -> ");
                        printToken(token, tokenInfos.tokenString);
                        tokenInfos = getNextToken();
                        token = tokenInfos.token;
                        return NULL;
                    }
                }
                else {
                    if (t != NULL) {
                        t->child[1] = statement();
                        match(SEMICOLON);
                    }
                }
                if (match(KEYWORD_ELSE)) {
                    if (match(L_BRACE)) {
                        if (t != NULL) t->child[2] = stmt_list();
                        if (!match(R_BRACE)) {
                            syntaxError("unexpected token -> ");
                            printToken(token, tokenInfos.tokenString);
                            tokenInfos = getNextToken();
                            token = tokenInfos.token;
                            return NULL;
                        }
                    }
                    else if (token==KEYWORD_IF) {
                        if (t != NULL) t->child[2] = if_stmt();
                    }else {
                        if (t != NULL) t->child[2] = stmt_list();
                    }
                }
                return t;
            }
        }
    }
    syntaxError("unexpected token -> ");
    printToken(token, tokenString);
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    return NULL;
}
/*
* whileÓï¾ä
*/
TreeNode* repeat_stmt(void) {
    TreeNode* t = newStmtNode(RepeatK);
    if (match(KEYWORD_WHILE)) {
        if (match(L_PARENTHESIS)) {
            if (t != NULL) t->child[0] = exps();
            if (match(R_PARENTHESIS)) {
                if (match(L_BRACE)) {
                    if (t != NULL) t->child[1] = stmt_list();
                    if (!match(R_BRACE)) {
                        syntaxError("unexpected token -> ");
                        printToken(token, tokenInfos.tokenString);
                        tokenInfos = getNextToken();
                        token = tokenInfos.token;
                        return NULL;
                    }
                }
                else {
                    if (t != NULL) t->child[1] = stmt_list();
                }
                return t;
            }

        }
    }
    syntaxError("unexpected token -> ");
    printToken(token, tokenInfos.tokenString);
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    return NULL;
}
/*
* ¸³ÖµÓï¾ä
*/
TreeNode* assign_stmt(void) {
    TreeNode* t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenInfos.tokenString);
    match(ID);
    if (match(ASSIGN)) {
        if (t != NULL) t->child[0] = exps();
        return t;
    }
    syntaxError("unexpected token -> ");
    printToken(token, tokenInfos.tokenString);
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    return NULL;
}
/*
* ÊäÈëÓï¾ä
*/
TreeNode* read_stmt(void) {
    TreeNode* t = newStmtNode(ReadK);
    match(KEYWORD_INPUT);
    t->attr.op = KEYWORD_INPUT;
    match(L_PARENTHESIS);
    match(R_PARENTHESIS);
    return t;
}

int parseStr(char* str) {
    int res = 0;
    for (int i = 0; *(str + i) != '\0'; i++) {
        res = res * 10 + *(str + i) - '0';
    }
    return res;
}

/*
* Êä³öÓï¾ä
*/
TreeNode* write_stmt(void) {
    TreeNode* t = newStmtNode(WriteK);
    match(KEYWORD_OUTPUT);
    t->attr.op = KEYWORD_OUTPUT;
    if (match(L_PARENTHESIS)) {
        t->child[0] = exps();
        if (match(R_PARENTHESIS)) {
            return t;
        }
    }
    syntaxError("unexpected token -> ");
    printToken(token, tokenInfos.tokenString);
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    return NULL;
}

TreeNode* param(void) {
    TreeNode* t = newParamsNode(ParamsK);
    t->attr.op = token;
    match(token);
    if ((t != NULL) && (token == ID)) {
        t->attr.name = copyString(tokenInfos.tokenString);
    }
    match(ID);
    return t;
}

TreeNode* params(void) {
    TreeNode* t = newParamsNode(ParamListK);
    if (match(KEYWORD_VOID)) {
        t->attr.op = KEYWORD_VOID;
        t->attr.name = "void";
        return t;
    }
    t->child[0] = param();
    TreeNode* p = t->child[0];
    while (match(COMMA)) {
        TreeNode* q;
        q = param();
        if (q != NULL) {
            p->sibling = q;
            p = q;
        }
    }
    return t;
}


/*
* ÉùÃ÷Óï¾ä
*/
TreeNode* declaration_stmt(void) {
    TreeNode* t = newStmtNode(DeclarationK);
    if (match(KEYWORD_INT)) {
        t->attr.op = KEYWORD_INT;
        if (token==ID) {
            t->attr.name = copyString(tokenInfos.tokenString);
            match(ID);
            if (match(L_PARENTHESIS)) {
                if (t != NULL) t->child[0] = params();
                if (match(R_PARENTHESIS)) {
                    match(L_BRACE);
                    t->child[1]= stmt_list();
                    match(R_BRACE);
                    return t;
                } else {
                    syntaxError("unexpected token -> ");
                    printToken(token, tokenInfos.tokenString);
                    tokenInfos = getNextToken();
                    token = tokenInfos.token;
                    return NULL;
                }
            }
            else {
                return t;
            }
        }
    }
    else if (match(KEYWORD_VOID)) {
        t->attr.op = KEYWORD_VOID;
        if (token==ID) {
            t->attr.name = copyString(tokenInfos.tokenString);
            match(ID);
            if (match(L_PARENTHESIS)) {
                if (t != NULL) t->child[0] = params();
                if (match(R_PARENTHESIS)) {
                    match(L_BRACE);
                    t->child[1] = stmt_list();
                    match(R_BRACE);
                    return t;
                }
                else {
                    syntaxError("unexpected token -> ");
                    printToken(token, tokenInfos.tokenString);
                    tokenInfos = getNextToken();
                    token = tokenInfos.token;
                    return NULL;
                }
            }
            else {
                return t;
            }
        }
    }
    syntaxError("unexpected token -> ");
    printToken(token, tokenInfos.tokenString);
    tokenInfos = getNextToken();
    token = tokenInfos.token;
    return NULL;
}

/*
* ·µ»ØÓï¾ä
*/
TreeNode* return_stmt(void) {
    TreeNode* t = newStmtNode(ReturnK);
    match(KEYWORD_RETURN);
    if (token != SEMICOLON) {
        if (t != NULL) t->child[0] = exps();
    }
    return t;
}

TreeNode* factor(void) {
    TreeNode* t = NULL;
    switch (token) {
    case NUM:
        t = newExpNode(ConstK);
        if ((t != NULL) && (token == NUM))
            t->attr.val = atoi(tokenInfos.tokenString);
        match(NUM);
        break;
    case ID:
        t = newExpNode(IdK);
        if ((t != NULL) && (token == ID))
            t->attr.name = copyString(tokenInfos.tokenString);
        match(ID);
        if (match(L_PARENTHESIS)) {
            if (!match(R_PARENTHESIS)) {
                t->child[0] = factor();
                TreeNode* p = t->child[0];
                while (match(COMMA)) {
                    TreeNode* q;
                    q = factor();
                    if (q != NULL) {
                        p->sibling = q;
                        p = q;
                    }
                }
                match(R_PARENTHESIS);
            }
        }
        break;
    case L_PARENTHESIS:
        match(L_PARENTHESIS);
        t = exps();
        match(R_PARENTHESIS);
        break;
    case KEYWORD_INPUT:
        t = read_stmt();
        break;
    default:
        syntaxError("unexpected token -> ");
        tokenInfos = getNextToken();
        token = tokenInfos.token;
        break;
    }
    return t;
}

TreeNode* exps(void) {
    TreeNode* t = simple_exp();
    if ((token == LESS_THAN) || (token == LESS_EQUAL_THAN) || (token == DOUBLE_EQUAL)||(token==GREAT_THAN)||(token==GREAT_EQUAL_THAN)|| (token == NOT_EQUAL)) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

TreeNode* simple_exp(void) {
    TreeNode* t = term();
    while ((token == PLUS) || (token == MINUS)){
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

TreeNode* term(void)
{
    TreeNode* t = factor();
    while ((token == MUTIPLY) || (token == DIVIDE))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}