/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include "util.h"

#define ofpFO 0

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 3;

static int tmpOffsetForMp = 0;

static int tmpVar = 0;

char* tmpName[10];


/* prototype for internal recursive code generator */
static void cGen(TreeNode* tree);

int mainLoc;

static void genParam(TreeNode* tree) {
    if (tree->attr.op == KEYWORD_VOID) {
        return;
    }
    TreeNode* p = tree->child[0];
    int i = 0;
    while (p != NULL) {
        st_insert(p->attr.name, p->lineno, i++);
        tmpName[tmpVar++] = copyString(p->attr.name);
        p = p->sibling;
    }
}

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen(TreeNode* tree)
{
    if (tree != NULL)
    {
        switch (tree->nodekind) {
        case StmtK:
            genStmt(tree);
            break;
        case ExpK:
            genExp(tree);
            break;
        case ParamsK:
            genParam(tree);
            break;
        default:
            break;
        }
        cGen(tree->sibling);
    }
}

/* Procedure genStmt generates code at a statement node */
static void genStmt(TreeNode* tree) {
    TreeNode* p1, * p2, * p3;
    int savedLoc1, savedLoc2, currentLoc;
    int loc;
    switch (tree->kind.stmt) {
    case StatementK:
        cGen(tree->child[0]);
        break;
    case IfK:
        if (TraceCode) emitComment("-> if");
        p1 = tree->child[0];
        p2 = tree->child[1];
        p3 = tree->child[2];
        /* generate code for test expression */
        cGen(p1);
        savedLoc1 = emitSkip(1);
        emitComment("if: jump to else belongs here");
        /* recurse on then part */
        cGen(p2);
        savedLoc2 = emitSkip(1);
        emitComment("if: jump to end belongs here");
        currentLoc = emitSkip(0);
        emitBackup(savedLoc1);
        emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
        emitRestore();
        /* recurse on else part */
        cGen(p3);
        currentLoc = emitSkip(0);
        emitBackup(savedLoc2);
        emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
        emitRestore();
        if (TraceCode)  emitComment("<- if");
        break; /* if_k */
    case RepeatK:
        if (TraceCode) emitComment("-> repeat");
        p1 = tree->child[0];
        p2 = tree->child[1];
        savedLoc1 = emitSkip(0);
        /* generate code for test */
        cGen(p1);
        savedLoc2 = emitSkip(1);
        /* generate code for body */
        cGen(p2);
        emitRM_Abs("LDA", pc, savedLoc1, "jmp to end");
        currentLoc = emitSkip(0);
        emitBackup(savedLoc2);
        emitRM_Abs("JEQ", ac, currentLoc, "if: jmp out");
        emitRestore();
        if (TraceCode)  emitComment("<- repeat");
        break; /* repeat */
    case AssignK:
        if (TraceCode) emitComment("-> assign");
        /* generate code for rhs */
        cGen(tree->child[0]);
        /* now store value */
        loc = st_lookup(tree->attr.name);
        emitRM("ST", ac, loc, gp, "assign: store value");
        if (TraceCode)  emitComment("<- assign");
        break; /* assign_k */
    case ReadK:
        emitRO("IN", ac, 0, 0, "read integer value");
        break;
    case WriteK:
        /* generate code for expression to write */
        cGen(tree->child[0]);
        /* now output it */
        emitRO("OUT", ac, 0, 0, "write ac");
        break;
    case ReturnK:
        if (tree->child[0]!=NULL) {
            cGen(tree->child[0]);
            emitRM("ST", ac, 1, gp, "返回值");
            emitRM("LDA", pc, 0, re, "返回主函数");
        }
        else {
            emitRM("JNE", re, 1, pc, "有回调函数");
            emitRO("HALT", 0, 0, 0, "");
            emitRM("LDA", pc, 0, re, "返回主函数");
        }
        break;
    case DeclarationK:
        if (tree->child[0] == NULL) {
            //变量声明
            st_insert(tree->attr.name, tree->lineno, tmpOffset++);
            tmpName[tmpVar++] = copyString(tree->attr.name);
        }
        else {
            //函数声明及定义
            currentLoc = emitSkip(0);
            if (strcmp(tree->attr.name, "main") == 0) {
                cGen(tree->child[1]);
                mainLoc = currentLoc;
            }
            else {
                tmpVar = 0;
                cGen(tree->child[0]);
                cGen(tree->child[1]);
                st_insert(tree->attr.name, tree->lineno, currentLoc);
                st_del(tmpName, tmpVar);
            }
        }
        break;
    case Callk:
        loc = st_lookup(tree->attr.name);
        if (tree->child[0] != NULL) {
            TreeNode* p = tree->child[0];
            int i = 0;
            while (p != NULL) {
                genExp(p);
                emitRM("ST", ac, i++, gp, "load param");
                p = p->sibling;
            }
        }
        emitRM("LDC", re, emitSkip(0) + 2, ze, "jmp to end");
        emitRM("LDA", pc, loc, ze, "jmp to method");
        emitRM("LD", ac, 1, gp, "load id value");
        emitRM("LDC", re, 0, 0, "返回地址设为0");
    default:
        break;
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp(TreeNode* tree)
{
    int loc,loc1;
    TreeNode* p1, * p2;
    switch (tree->kind.exp) {

    case ConstK:
        if (TraceCode) emitComment("-> Const");
        /* gen code to load integer constant using LDC */
        emitRM("LDC", ac, tree->attr.val, 0, "load const");
        if (TraceCode)  emitComment("<- Const");
        break; /* ConstK */

    case IdK:
        if (TraceCode) emitComment("-> Id");
        if (tree->child[0] == NULL) {
            loc = st_lookup(tree->attr.name);
            emitRM("LD", ac, loc, gp, "load id value");
            if (TraceCode)  emitComment("<- Id");
        }
        else {
            loc = st_lookup(tree->attr.name);
            if (tree->child[0] != NULL) {
                TreeNode* p = tree->child[0];
                int i = 0;
                while (p != NULL) {
                    genExp(p);
                    emitRM("ST", ac, i++, gp, "load param");
                    p = p->sibling;
                }
            }
            emitRM("LDC", re, emitSkip(0)+2, ze, "jmp to end");
            emitRM("LDA", pc, loc, ze, "jmp to method");
            emitRM("LD", ac, 1, gp, "load id value");
            emitRM("LDC", re, 0, 0, "返回地址设为0");
        }
        break; /* IdK */

    case OpK:
        if (TraceCode) emitComment("-> Op");
        p1 = tree->child[0];
        p2 = tree->child[1];
        /* gen code for ac = left arg */
        cGen(p2);
        /* gen code to push left operand */
        emitRO("ADD", ac1, ac, ze, "op: push right");
        /* gen code for ac = right operand */
        cGen(p1);
        switch (tree->attr.op) {
        case PLUS:
            emitRO("ADD", ac, ac, ac1, "op +");
            break;
        case MINUS:
            emitRO("SUB", ac, ac, ac1, "op -");
            break;
        case MUTIPLY:
            emitRO("MUL", ac, ac, ac1, "op *");
            break;
        case DIVIDE:
            emitRO("DIV", ac, ac, ac1, "op /");
            break;
        case LESS_THAN:
            emitRO("SUB", ac, ac1, ac, "op <");
            emitRM("JGT", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case LESS_EQUAL_THAN:
            emitRO("SUB", ac, ac1, ac, "op <");
            emitRM("JGE", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case GREAT_EQUAL_THAN:
            emitRO("SUB", ac, ac1, ac, "op <");
            emitRM("JLE", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case GREAT_THAN:
            emitRO("SUB", ac, ac1, ac, "op <");
            emitRM("JLT", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case DOUBLE_EQUAL:
            emitRO("SUB", ac, ac1, ac, "op ==");
            emitRM("JEQ", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case NOT_EQUAL:
            emitRO("SUB", ac, ac1, ac, "op !=");
            emitRM("JNE", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        default:
            emitComment("BUG: Unknown operator");
            break;
        } /* case op */
        if (TraceCode)  emitComment("<- Op");
        break; /* OpK */

    default:
        break;
    }
} /* genExp */



/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode* syntaxTree, char* codefile)
{
    //char* s = (char*)malloc(strlen(codefile) + 10);
    //strcpy(s, "File: \0");
    //strcat(s, codefile);
    emitComment("TINY Compilation to TM Code");
    //emitComment(s);
    /* generate standard prelude */
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
    emitRM("ST", ac, 0, ac, "clear location 0");
    emitRM("LDC", ze, 0, 0, "load maxaddress from location 0");
    emitRM("LDC", re, 0, 0, "返回地址设为0");
    emitComment("End of standard prelude.");
    /* generate code for TINY program */
    int curloc = emitSkip(0);
    emitSkip(1);
    cGen(syntaxTree);
    emitBackup(curloc);
    emitRM("LDA", pc, mainLoc,ze, "jmp to main");
    emitRestore();
    /* finish */
    emitComment("End of execution.");
    emitRO("HALT", 0, 0, 0, "");
}