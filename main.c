#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include"globals.h"
#include"scan.h"
#include"parse.h"
#include"util.h"
#include"cgen.h"
/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE
/* allocate global variables */
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

/* allocate and set tracing flags */
bool EchoSource = false;
bool TraceScan = true;
bool TraceParse = true;
bool TraceAnalyze = false;
bool TraceCode = false;

bool Error = false;

int main(int argc, char* argv[]) {
    TreeNode* syntaxTree;
    char pgm[120]; //Դ�����ļ���
    //�����в���������2�������������
    if (argc != 2) {
        //stderr��׼���������
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    //��������������ļ������Ƶ�������
    strcpy(pgm, argv[1]);
    //�ļ�����û�г���.����ô�����Ӻ�׺
    if (strchr(pgm, '.') == NULL) {
        strcat(pgm, ".tny");
    }
    //��ֻ����ʽ���ļ�
    source = fopen(pgm, "r");
    //�ļ�δ�ҵ�
    if (source == NULL) {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }
    listing = stdout; //��ȡ��׼���
    fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
    while (getToken() != ENDFILE);
#else
    syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }
#if !NO_ANALYZE
    if (!Error)
    {
        if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
        //buildSymtab(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
        //typeCheck(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
    }
#if !NO_CODE
    if (!Error)
    {
        char* codefile;
        int fnlen = strcspn(pgm, ".");
        codefile = (char*)malloc((fnlen + 4)*sizeof(char));
        strncpy(codefile, pgm, fnlen);
        *(codefile + fnlen) = '\0';
        strcat(codefile, ".tm\0");
        code = fopen(codefile, "w");
        if (code == NULL)
        {
            printf("Unable to open %s\n", codefile);
            exit(1);
        }
        codeGen(syntaxTree, codefile);
        fclose(code);
    }
#endif
#endif
#endif
    fclose(source);
    return 0;
}
