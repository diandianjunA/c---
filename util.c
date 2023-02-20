#include "util.h"
#include "scan.h"
/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
/*
* 输出词法分析结果
*/
void printToken(TokenType token, const char* tokenString){
    switch (token){
        case KEYWORD_IF:fprintf(listing, "%s: %s\n", "KEYWORD_IF", tokenString); break;
        case KEYWORD_ELSE:fprintf(listing, "%s: %s\n", "KEYWORD_ELSE", tokenString); break;
        case KEYWORD_INPUT:fprintf(listing, "%s: %s\n", "KEYWORD_INPUT", tokenString); break;
        case KEYWORD_INT:fprintf(listing, "%s: %s\n", "KEYWORD_INT", tokenString); break;
        case KEYWORD_OUTPUT:fprintf(listing, "%s: %s\n", "KEYWORD_OUTPUT", tokenString); break;
        case KEYWORD_RETURN:fprintf(listing, "%s: %s\n", "KEYWORD_RETURN", tokenString); break;
        case KEYWORD_VOID:fprintf(listing, "%s: %s\n", "KEYWORD_VOID", tokenString); break;
        case KEYWORD_WHILE:fprintf(listing, "%s: %s\n", "KEYWORD_WHILE", tokenString); break;
        case ASSIGN: fprintf(listing, "ASSIGN\n"); break;
        case LESS_THAN: fprintf(listing, "LESS_THAN\n"); break;
        case GREAT_THAN: fprintf(listing, "GREAT_THAN\n"); break;
        case LESS_EQUAL_THAN: fprintf(listing, "LESS_EQUAL_THAN\n"); break;
        case GREAT_EQUAL_THAN: fprintf(listing, "GREAT_EQUAL_THAN\n"); break;
        case DOUBLE_EQUAL: fprintf(listing, "DOUBLE_EQUAL\n"); break;
        case NOT_EQUAL: fprintf(listing, "NOT_EQUAL\n"); break;
        case L_PARENTHESIS: fprintf(listing, "L_PARENTHESIS\n"); break;
        case R_PARENTHESIS: fprintf(listing, "R_PARENTHESIS\n"); break;
        case SEMICOLON: fprintf(listing, "SEMICOLON\n"); break;
        case PLUS: fprintf(listing, "PLUS\n"); break;
        case MINUS: fprintf(listing, "MINUS\n"); break;
        case MUTIPLY: fprintf(listing, "MUTIPLY\n"); break;
        case DIVIDE: fprintf(listing, "DIVIDE\n"); break;
        case L_BRACE: fprintf(listing, "L_BRACE\n"); break;
        case R_BRACE: fprintf(listing, "R_BRACE\n"); break;
        case ENDFILE: fprintf(listing, "EOF\n"); break;
        case COMMA: fprintf(listing, "COMMA\n"); break;
        case NUM:
            fprintf(listing,
                "NUM, val= %s\n", tokenString);
            break;
        case ID:
            fprintf(listing,
                "ID, name= %s\n", tokenString);
            break;
        case ERROR:
            fprintf(listing,
                "ERROR: %s\n", tokenString);
            break;
        default: /* should never happen */
            fprintf(listing, "Unknown token: %d\n", token);
    }
    tokenInfo* info = (tokenInfo*)malloc(sizeof(tokenInfo));
    info->token = token;
    int i = 0;
    for (i = 0; tokenString[i] != '\0'; i++) {
        info->tokenString[i] = tokenString[i];
    }
    info->tokenString[i] = '\0';
    info->lineno = lineno;
    point->next = info;
    info->last = point;
    point = info;
    point->next = NULL;
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind) {
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
        t->attr.name = NULL;
    }
    return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind)
{
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
    }
    return t;
}

TreeNode* newParamsNode(ParamsKind kind) {
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ParamsK;
        t->kind.params = kind;
        t->lineno = lineno;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s)
{
    int n;
    char* t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = (char*)malloc(n);
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else strcpy(t, s);
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
    int i;
    for (i = 0; i < indentno; i++)
        fprintf(listing, " ");
}

char* getTokenName(TokenType token) {
    switch (token) {
    case LESS_EQUAL_THAN: return "<="; break;
    case LESS_THAN: return "<"; break;
    case GREAT_THAN: return ">"; break;
    case GREAT_EQUAL_THAN: return ">="; break;
    case DOUBLE_EQUAL: return "=="; break;
    case NOT_EQUAL: return "!="; break;
    case KEYWORD_ELSE: return "else"; break;
    case KEYWORD_IF: return "if"; break;
    case KEYWORD_INT: return "int"; break;
    case KEYWORD_RETURN: return "return"; break;
    case KEYWORD_VOID: return "void"; break;
    case KEYWORD_WHILE: return "while"; break;
    case KEYWORD_INPUT: return "input"; break;
    case KEYWORD_OUTPUT: return "output"; break;
    case ERROR: return "error"; break;
    case ASSIGN: return "="; break;
    case PLUS: return "+"; break;
    case MINUS: return "-"; break;
    case MUTIPLY: return "*"; break;
    case DIVIDE: return "/"; break;
    case SEMICOLON: return ";"; break;
    case COMMA: return ","; break;
    default:
        return NULL;
        break;
    }
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree)
{
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        if (tree->nodekind == StmtK) {
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(listing, "If\n");
                break;
            case RepeatK:
                fprintf(listing, "While\n");
                break;
            case AssignK:
                fprintf(listing, "Assign to: %s\n", tree->attr.name);
                break;
            case ReadK:
                fprintf(listing, "Input\n");
                break;
            case WriteK:
                fprintf(listing, "Output: \n");
                break;
            case DeclarationK:
                fprintf(listing, "Declaration:%s %s\n",getTokenName(tree->attr.op), tree->attr.name);
                break;
            case ReturnK:
                fprintf(listing, "Return\n");
                break;
            case StatementK:
                fprintf(listing, "Statement\n");
                break;
            case Callk:
                fprintf(listing, "Call: \n");
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
        }
        else if (tree->nodekind == ExpK)
        {
            switch (tree->kind.exp) {
            case OpK:
                fprintf(listing, "Op: ");
                printToken(tree->attr.op, "\0");
                break;
            case ConstK:
                fprintf(listing, "Const: %d\n", tree->attr.val);
                break;
            case IdK:
                fprintf(listing, "Id: %s\n", tree->attr.name);
                break;
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
        }
        else if (tree->nodekind == ParamsK) {
            switch (tree->kind.params) {
            case ParamListK:
                if (tree->attr.op == KEYWORD_VOID) {
                    fprintf(listing, "ParamList: void\n");
                }
                else {
                    fprintf(listing, "ParamList: \n");
                }
                break;
            case ParamsK:
                fprintf(listing, "Param: ");
                fprintf(listing, "%s %s\n", getTokenName(tree->attr.op), tree->attr.name);
                break;
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
        } else fprintf(listing, "Unknown node kind\n");
        for (i = 0; i < MAXCHILDREN; i++) {
            if (tree->child[i] != NULL) {
                printTree(tree->child[i]);
            }
        }
            
        tree = tree->sibling;
    }
    UNINDENT;
}
