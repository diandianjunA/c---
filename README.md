# c---
一个基于TM虚拟机的c--编译器
## C--语言描述
C--语言是 C 语言的一个严格子集，其语法复杂度介于 TINY C 和 C-语言（参见实
验参考书附录 A）之间。以下是关键描述：
1. C--语言是大小写敏感语言，只支持整数 int 类型和用于函数返回和参数类型的
   void，/*和*/间内容为注释，注释不可嵌套，空格包括空白、换行符和制表符；
2. C--语言的表达式包括整数上的算术表达式(+, -,*, /)和整数间比较的布尔表达
   式(<, >, !=, <=, >=, ==)；
3. C--语言的语句由分号;分割。可以包含多条语句的语句序列位于{和}内；
4. C--程序由函数声明和变量声明组成，必须先声明后使用，程序最后的一个声
   明必须是主入口函数声明 void main(void)；
5. 输入和输出通过在全局环境中两个预定义函数完成：输入函数 int input(void)
   和输出函数 void output(int x)，分别实现从标准输入设备（键盘）读入一个整数
   值和将一个整数值和换行符打印到标准输出设备（屏幕）；
6. C--语言支持 if-else、while 和 return 控制语句；
7. C--语言不支持数组和递归函数。

本文将选用三个例子作为该编译器的演示结果

```C
/*斐波那契数*/
void main(void){
	int n;
	int i;
	int a;
	int b;
	int sum;
	i=3;
	a=1;
	b=1;
	n=input();
	if(n<=2){
		output(1);
		return;
	}
	while(i<=n){
		sum=a+b;
		b=a;
		a=sum;
		i=i+1;
	}
	output(sum);
}
```

```C
/*求最大数*/
/*the maximum of three numbers*/
int max(int x, int y, int z){
	int biggest;
	biggest = x;
	if(y>biggest) /*comment example*/
		biggest = y;
	if(z>biggest) 
		biggest = z; 
	return biggest;
}
void main(void){
	int x;
	int y;
	int z;
	int biggest; 
	x = input();
	y = input();     
	z = input();
	biggest = max(x, y, z);
	output(biggest);
}
```
```C
/*求1到n的和*/
int sum(int n){
	int result;
	int i;
	i = 1;
	result = 0; 
	while(i<=n){ 
		result = result + i;
		i = i + 1;
	}
	return result;
}
void main(void){
	int n;
	int s; 
	n = input(); 
	s = sum(n); 
	output(s);
}
```

## 词法分析

### 设计思路
从源代码文件中读取一行数据，然后通过指针移动来一个一个进行分析，需要用到数组这一数据结构。如果需要回退也只需要指针前移即可。

设定一系列的关键字，用于简化代码中的字符，便于语法分析时进行判断词语种类；由枚举类来实现。

在进行词语扫描的时候，运用有限自动机的原理，对每一种状态进行定义，在每种状态下，下一个字符的类型会决定自动机接下来的走势。例如在遇到’/’时，将进入一种即将进入注释阶段的状态，下一个字符如果是’*’，则会进入注释状态，接下来所读的字符都不进行记录（再次遇到’*’的情况除外）；下一个字符如果是数字，那么说明上一个’/’是除号，将退回到正常状态并正常记录；下一个字符如果是其他字符，则程序有误，将进入报错的状态。同理离开注释阶段，以及>=，<=，==等多个字符组成的符号，均可采用有限自动机的原理进行解决。

当一个token读取成功时，将进行输出，并将其储存到一个链表中，供下一步的语义分析进行使用

使用到的数据结构有

```C
/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];//存储字符用的数组
/*存储token流的链表*/
tokenInfo* head;
tokenInfo* point;
/*tokenInfo类型的定义*/
typedef struct tokenInfo {
    TokenType token;
    char tokenString[40+1];
    int lineno;
    struct tokenInfo* last;
    struct tokenInfo* next;
} tokenInfo;
//token枚举类
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
```

斐波那契数词法分析程序运行结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture1.png)

三个数的最大值的词法分析程序结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture2.png)
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture3.png)

求1-n的和的词法分析程序结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture4.png)
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture5.png)

### lex输入文件

```C
%{

%}
%option yylineno
delim      [ \t]
letter     [a-zA-Z]
digit      [0-9]
id         {letter}{letter}*|{letter}{digit}*
NUM        {digit}{digit}*

%%
{delim}+                                        {  }
"\n"|"\r\n"                                     {  }
"{"                                             { printf("L_BRACE : %s\n", yytext);}
"}"                                             { printf("R_BRACE : %s\n", yytext);}
"("                                             { printf("L_PARENTHESIS : %s\n", yytext);}
")"                                             { printf("R_PARENTHESIS : %s\n", yytext);}
"/"|"*"|"-"|"+"|"="                             { printf("OPERATOR : %s\n", yytext);}
","                                             { printf("COMMA : %s\n", yytext);}
";"                                             { printf("SEMICOLON : %s\n", yytext);}
"else"                                          { printf("KEYWORD_ELSE : %s\n", yytext);}
"if"                                            { printf("KEYWORD_IF : %s\n", yytext);}
"int"                                           { printf("KEYWORD_INT : %s\n", yytext);}
"return"                                        { printf("KEYWORD_RETURN : %s\n", yytext);}
"void"                                          { printf("KEYWORD_VOID : %s\n", yytext);}
"while"                                         { printf("KEYWORD_WHILE : %s\n", yytext);}
"<"                                             { printf("LESS_THAN : %s\n", yytext);}
"<="                                            { printf("LESS_EQUAL_THAN : %s\n", yytext);}
">"                                             { printf("GREAT_THAN : %s\n", yytext);}
">="                                            { printf("GREAT_EQUAL_THAN : %s\n", yytext);}
"=="                                            { printf("DOUBLE_EQUAL : %s\n", yytext);}
"!="                                            { printf("NOT_EQUAL : %s\n", yytext);}
{id}                                            { printf("ID : %s\n", yytext);}
{NUM}                                           { printf("NUM : %s\n", yytext);}
"/*"[^*]*"*/"|"/*"[^*]*"*/"\n                   { printf("MULTI_LINE_ANNOTATION : %s\n", yytext);}
.                                               { printf("DOT : %s\n", yytext);}

%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}
```

## 语法分析

### C--的BNF语法范式

```C
    0 $accept: program $end

    1 program: declaration_list

    2 declaration_list: declaration_list declaration
    3                 | declaration
    4                 | comment

    5 declaration: var_declaration
    6            | fun_declaration

    7 var_declaration: type_specifier ID ';'
    8                | type_specifier ID '[' NUM ']' ';'

    9 type_specifier: KEYWORD_INT
   10               | KEYWORD_VOID

   11 fun_declaration: type_specifier ID '(' params ')' compound_stmt
   12                | type_specifier ID '(' params ')' ';'

   13 comment: comment MULTI_LINE_ANNOTATION
   14        | /* empty */

   15 params: param_list
   16       | KEYWORD_VOID

   17 param_list: param_list ',' param
   18           | param

   19 param: type_specifier ID
   20      | type_specifier ID '[' ']'

   21 compound_stmt: '{' local_declarations statement_list '}'

   22 local_declarations: local_declarations var_declaration
   23                   | /* empty */

   24 statement_list: statement_list statement comment
   25               | /* empty */

   26 statement: expression_stmt
   27          | compound_stmt
   28          | selection_stmt
   29          | iteration_stmt
   30          | return_stmt

   31 expression_stmt: expression ';'
   32                | ';'

   33 selection_stmt: KEYWORD_IF '(' expression ')' statement
   34               | KEYWORD_IF '(' expression ')' statement KEYWORD_ELSE statement

   35 $@1: /* empty */

   36 iteration_stmt: KEYWORD_WHILE '(' expression $@1 ')' statement

   37 return_stmt: KEYWORD_RETURN ';'
   38            | KEYWORD_RETURN expression ';'

   39 expression: var '=' expression
   40           | simple_expression

   41 var: ID
   42    | ID '[' expression ']'

   43 simple_expression: additive_expression LESS_EQUAL_THAN additive_expression
   44                  | additive_expression LESS_THAN additive_expression
   45                  | additive_expression GREAT_THAN additive_expression
   46                  | additive_expression GREAT_EQUAL_THAN additive_expression
   47                  | additive_expression DOUBLE_EQUAL additive_expression
   48                  | additive_expression NOT_EQUAL additive_expression
   49                  | additive_expression

   50 additive_expression: additive_expression '+' term
   51                    | additive_expression '-' term
   52                    | term

   53 term: term '*' factor
   54     | term '/' factor
   55     | factor

   56 factor: '(' expression ')'
   57       | var
   58       | call
   59       | NUM

   60 call: ID '(' args ')'

   61 args: arg_list
   62     | /* empty */

   63 arg_list: arg_list ',' expression
   64         | expression
```

### 设计思路

获取从词法分析函数解析得到的token流（以链表的形式储存），然后根据BNF语法范式，用有限自动机的方式循环获取其中所有的结构，并以二叉树的形式进行储存，二叉树的每一个节点都储存如下信息：子节点，兄弟节点，此节点的类型，行号，储存的token类型，值以及名称。共可完成if语句，while语句，赋值语句，输入输出语句，声明语句，返回语句的分析。

在输出抽象语法树时，采用前序遍历的方式，先输出子节点，再输出兄弟节点，通过一些缩进来使语法树的结构更加清晰

主要用到的一些数据结构

```C
//段落语句类型
typedef enum { StmtK, ExpK,ParamsK } NodeKind;
//语句类型
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK, DeclarationK, ReturnK, StatementK,Callk } StmtKind;
//表达式类型
typedef enum { OpK, ConstK, IdK } ExpKind;
//参数列表类型
typedef enum { ParamListK,Param } ParamsKind;
/* ExpType is used for type checking */
typedef enum { Void, Integer, Boolean } ExpType;
typedef int TokenType;
//抽象语法树的节点结构体
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
```

斐波那契数的语法分析树
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture6.png)

求最大值的语法分析树
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture7.png)

求1-n的和的语法分析树
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture8.png)

## 转换成TM虚拟机的指令

具体指令参考书籍编译原理与实践的第8章代码生成

![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture9.png)

### 设计思路
根据语法树设计环节区分的语句种类，分不同的语句实现其TM汇编指令

借助0-7不同的寄存器以及两个栈，分别实现if，while，return等语句

其中0,1为两个存储临时变量的寄存器

2,3,4为自由使用的寄存器

5,6分别为指向栈顶和栈底的寄存器

7为指向程序目前执行位置的寄存器

具体语句转换成TM指令的程序实现在代码cgen.c文件中可供查看

斐波那契数程序转换为的指令文件

```html
  0:     LD  6,0(0) 
  1:     ST  0,0(0) 
  2:    LDC  3,0(0) 
  3:    LDC  2,0(0) 
  5:    LDC  0,3(0) 
  6:     ST  0,4(5) 
  7:    LDC  0,1(0) 
  8:     ST  0,5(5) 
  9:    LDC  0,1(0) 
 10:     ST  0,6(5) 
 11:     IN  0,0,0 
 12:     ST  0,3(5) 
 13:    LDC  0,2(0) 
 14:    ADD  1,0,3 
 15:     LD  0,3(5) 
 16:    SUB  0,1,0 
 17:    JGE  0,2(7) 
 18:    LDC  0,0(0) 
 19:    LDA  7,1(7) 
 20:    LDC  0,1(0) 
 22:    LDC  0,1(0) 
 23:    OUT  0,0,0 
 24:    JNE  2,1(7) 
 25:   HALT  0,0,0 
 26:    LDA  7,0(2) 
 21:    JEQ  0,6(7) 
 27:    LDA  7,0(7) 
 28:     LD  0,3(5) 
 29:    ADD  1,0,3 
 30:     LD  0,4(5) 
 31:    SUB  0,1,0 
 32:    JGE  0,2(7) 
 33:    LDC  0,0(0) 
 34:    LDA  7,1(7) 
 35:    LDC  0,1(0) 
 37:     LD  0,6(5) 
 38:    ADD  1,0,3 
 39:     LD  0,5(5) 
 40:    ADD  0,0,1 
 41:     ST  0,7(5) 
 42:     LD  0,5(5) 
 43:     ST  0,6(5) 
 44:     LD  0,7(5) 
 45:     ST  0,5(5) 
 46:    LDC  0,1(0) 
 47:    ADD  1,0,3 
 48:     LD  0,4(5) 
 49:    ADD  0,0,1 
 50:     ST  0,4(5) 
 51:    LDA  7,-24(7) 
 36:    JEQ  0,15(7) 
 52:     LD  0,7(5) 
 53:    OUT  0,0,0 
  4:    LDA  7,5(3) 
 54:   HALT  0,0,0 
```

求三个数的最大值的程序代码转换为的指令文件

```html
  0:     LD  6,0(0) 
  1:     ST  0,0(0) 
  2:    LDC  3,0(0) 
  3:    LDC  2,0(0) 
  5:     LD  0,0(5) 
  6:     ST  0,3(5) 
  7:     LD  0,3(5) 
  8:    ADD  1,0,3 
  9:     LD  0,1(5) 
 10:    SUB  0,1,0 
 11:    JLT  0,2(7) 
 12:    LDC  0,0(0) 
 13:    LDA  7,1(7) 
 14:    LDC  0,1(0) 
 16:     LD  0,1(5) 
 17:     ST  0,3(5) 
 15:    JEQ  0,3(7) 
 18:    LDA  7,0(7) 
 19:     LD  0,3(5) 
 20:    ADD  1,0,3 
 21:     LD  0,2(5) 
 22:    SUB  0,1,0 
 23:    JLT  0,2(7) 
 24:    LDC  0,0(0) 
 25:    LDA  7,1(7) 
 26:    LDC  0,1(0) 
 28:     LD  0,2(5) 
 29:     ST  0,3(5) 
 27:    JEQ  0,3(7) 
 30:    LDA  7,0(7) 
 31:     LD  0,3(5) 
 32:     ST  0,1(5) 
 33:    LDA  7,0(2) 
 34:     IN  0,0,0 
 35:     ST  0,4(5) 
 36:     IN  0,0,0 
 37:     ST  0,5(5) 
 38:     IN  0,0,0 
 39:     ST  0,6(5) 
 40:     LD  0,4(5) 
 41:     ST  0,0(5) 
 42:     LD  0,5(5) 
 43:     ST  0,1(5) 
 44:     LD  0,6(5) 
 45:     ST  0,2(5) 
 46:    LDC  2,48(3) 
 47:    LDA  7,5(3) 
 48:     LD  0,1(5) 
 49:    LDC  2,0(0) 
 50:     ST  0,7(5) 
 51:     LD  0,7(5) 
 52:    OUT  0,0,0 
  4:    LDA  7,34(3) 
 53:   HALT  0,0,0 
```

求1-n的和的程序代码转换的指令文件

```html
  0:     LD  6,0(0) 
  1:     ST  0,0(0) 
  2:    LDC  3,0(0) 
  3:    LDC  2,0(0) 
  5:    LDC  0,1(0) 
  6:     ST  0,4(5) 
  7:    LDC  0,0(0) 
  8:     ST  0,3(5) 
  9:     LD  0,0(5) 
 10:    ADD  1,0,3 
 11:     LD  0,4(5) 
 12:    SUB  0,1,0 
 13:    JGE  0,2(7) 
 14:    LDC  0,0(0) 
 15:    LDA  7,1(7) 
 16:    LDC  0,1(0) 
 18:     LD  0,4(5) 
 19:    ADD  1,0,3 
 20:     LD  0,3(5) 
 21:    ADD  0,0,1 
 22:     ST  0,3(5) 
 23:    LDC  0,1(0) 
 24:    ADD  1,0,3 
 25:     LD  0,4(5) 
 26:    ADD  0,0,1 
 27:     ST  0,4(5) 
 28:    LDA  7,-20(7) 
 17:    JEQ  0,11(7) 
 29:     LD  0,3(5) 
 30:     ST  0,1(5) 
 31:    LDA  7,0(2) 
 32:     IN  0,0,0 
 33:     ST  0,5(5) 
 34:     LD  0,5(5) 
 35:     ST  0,0(5) 
 36:    LDC  2,38(3) 
 37:    LDA  7,5(3) 
 38:     LD  0,1(5) 
 39:    LDC  2,0(0) 
 40:     ST  0,6(5) 
 41:     LD  0,6(5) 
 42:    OUT  0,0,0 
  4:    LDA  7,32(3) 
 43:   HALT  0,0,0 
```

## 在tm虚拟机中运行

tm虚拟机可使用列表那个tm.exe文件，也可找寻网上的tm虚拟机源码进行编译生成

还可以根据编译原理及实践一书中的指导进行自行编写

在命令行中运行tm.exe xxx.tm(cgen生成的指令代码文件)

command时输入g，然后输入你要输入的数字，回车即可返回结果

斐波那契数执行结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture10.png)

求最大值执行结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture11.png)

求1-n的和的执行结果
![](http://www.diandianjun.com.cn:8080/resource/blog/text/ProjectPractice/C--/picture12.png)
