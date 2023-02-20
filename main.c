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
    char pgm[120]; //源代码文件名
    //命令行参数不等于2，参数输入错误
    if (argc != 2) {
        //stderr标准错误输出流
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    //把命令行输入的文件名复制到数组中
    strcpy(pgm, argv[1]);
    //文件名中没有出现.，那么给它加后缀
    if (strchr(pgm, '.') == NULL) {
        strcat(pgm, ".tny");
    }
    //以只读方式打开文件
    source = fopen(pgm, "r");
    //文件未找到
    if (source == NULL) {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }
    listing = stdout; //获取标准输出
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
