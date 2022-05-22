/*
 parser.c : analisador sintatico (exemplo de automato com pilha)
 Autor: Edna A. Hoshino

Gramática da linguagem aceita pelo parser:

 S  -> Function S_ 
 S_ -> Function S_ 
      | epsilon
 Function -> Type Function_
 Type -> void 
       | int 
       | float
 Function_ -> main() { B } 
            | id() { B }
 B -> C B 
      | epsilon
 C -> id = E ; 
      | while (E) C 
      | { B }
 E -> TE'
 E'-> +TE' 
      | epsilon
 T -> FT'
 T'-> *FT' // * é só um operador da linguagem
      | epsilon
 F -> (E) 
      | id 
      | num

 A saida do analisador apresenta o total de linhas processadas e uma mensagem de erro ou sucesso. 
 Atualmente, nao ha controle sobre a coluna e a linha em que o erro foi encontrado.
*/

#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "parser.h"

/* variaveis globais */
int lookahead;
FILE *fin;

/* Exige que o próximo terminal seja t e avança o ponteiro da fita de entrada (i.e., pega o próximo terminal) */
void match(int t)
{
  if(lookahead == t){
    lookahead = lex();
  }
  else{
     printf("\nErro(linha=%d): token %s (cod=%d) esperado.## Encontrado \"%s\" ##\n", lines, terminalName[t], t, lexema);
     exit(1);
  }
}

// S  -> Function S_ 
void S(){
  Function();
  S_();
}

/* S_ -> Function S_ 
      | epsilon
*/
void S_(){
  if(lookahead == INT || lookahead == FLOAT || lookahead == VOID){
    Function();
    S_();
  }
}

// Function -> Type Function_
void Function(){
  Type();
  Function_();
}

/* Type -> void 
       | int 
       | float
*/
void Type(){  
  if(lookahead==INT){
    match(INT);
  }
  else if(lookahead==FLOAT){
    match(FLOAT);
  }
  else{
    match(VOID);
  }
}

/* Function_ -> main() { B } 
            | id() { B }
*/
void Function_(){
  if(lookahead == MAIN){
    match(MAIN);
    match(ABRE_PARENT);
    match(FECHA_PARENT);
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
  else{
    match(ID);
    match(ABRE_PARENT);
    match(FECHA_PARENT);
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
}

/* B -> C B 
      | epsilon
*/
void B(){
  if(lookahead==ID || lookahead==WHILE || lookahead==ABRE_CHAVES){
    C();
    B();
  }
}
/*
 C -> id = E ; 
      | while (E) C 
      | { B }
*/
void C(){
   if(lookahead==ID){ //id = E ;
    match(ID);
    match(OP_ATRIB);
    E();
    match(PONTO_VIRG);
  }
  else if(lookahead==WHILE){
    match(WHILE);
    match(ABRE_PARENT);
    E();
    match(FECHA_PARENT);
    C();
  }
  else if(lookahead == SWITCH){
    match(SWITCH);
    match(ABRE_PARENT);
    C_(); 
    match(FECHA_PARENT);
    
    match(ABRE_CHAVES);
    CASE(); 
    match(FECHA_CHAVES);
  }
  else {
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
}
// E-> T E_
void E(){
  T();
  E_();
}
// T -> FT'
void T(){
  F();
  T_();
}
// E_ -> + T E_ | epsilon
void E_(){
  if(lookahead==OP_ADIT){
    match(OP_ADIT);
    T();
    E_();
  }
}
/* T'-> *FT' 
      | epsilon
*/
void T_(){
  if(lookahead==OP_MULT){
    match(OP_MULT);
    F();
    T_();
  }
}
/*
 F -> (E) 
      | id 
      | num
*/
void F(){
  if(lookahead==ABRE_PARENT){
    match(ABRE_PARENT);
    E();
    match(FECHA_PARENT);
  }
  else{
    if(lookahead==ID){
      match(ID);
    }
    else {
      match(NUM);
    }
  }

}

/* CASE -> CASE_CONDICAO CASE_ 
        | CASE_CONDICAO C   
        | Epsilon                  */
void CASE() {
  if(lookahead == CASE) {
    match(CASE);
    CASE_CONDICAO();
    CASE_();
    
  } 
  else if (lookahead == DEFAULT){
    match(DEFAULT);
    CASE_CONDICAO();
    C();
    if(lookahead == BREAK){
      match(BREAK);
    }
  }
}

/* CASE_ -> CASE
          | C CASE  */
void CASE_(){
  if(lookahead == CASE){
      CASE();

  } else if(lookahead == BREAK) {
    match(BREAK);
    match(PONTO_VIRG);
    CASE();

  } else {
    C();
    if(lookahead == BREAK){
      match(BREAK);
    }
    match(PONTO_VIRG);
    CASE();
  }
}

/* CASE_CONDICAO -> C_    */
void CASE_CONDICAO() {

  if(lookahead == ABRE_PARENT) {
      match(ABRE_PARENT);
      C_();
      match(FECHA_PARENT);
  }
  else {
      C_();
    match(DOT_DOT);
  }
}

/* C_ -> EXP_CASE */
void C_() {
  if(lookahead == ID) {
    match(ID);
    EXP_CASE();
  }
  else {
    match(NUM);
    EXP_CASE();
  }
}

/* EXP_CASE -> C_ 
              | Epsilon      */
void EXP_CASE() {
  if(lookahead == OP_ADIT){
    match(OP_ADIT);
    C_();
  } else if(lookahead == OP_MULT){
    match(OP_MULT);
    C_();
  } else if(lookahead == OP_MINUS){
    match(OP_MINUS);
    C_();
  } else if(lookahead == OP_DIV){
    match(OP_DIV);
    C_();
  }
  // VAZIA
}


/*******************************************************************************************
 parser(): 
 - efetua o processamento do automato com pilha AP
 - devolve uma mensagem para indicar se a "palavra" (programa) estah sintaticamente correta.
********************************************************************************************/
char *parser()
{
   lookahead = lex(); // inicializa lookahead com o primeiro terminal da fita de entrada (arquivo)
   S(); // chama a variável inicial da gramática.
   if(lookahead == FIM)
      return("Programa sintaticamente correto!");
   else
      return("Fim de arquivo esperado");
}

int main(int argc, char**argv)
{
  if(argc < 2){
    printf("\nUse: compile <filename>\n");
    return 1;
  }
  else{
    printf("\nAnalisando lexica e sintaticamente o programa: %s", argv[1]);
    fin=fopen(argv[1], "r");
    if(!fin){
      printf("\nProblema na abertura do programa %s\n", argv[1]);
      return 1;
    }
    // chama o parser para processar o arquivo de entrada
    printf("\nTotal de linhas processadas: %d\nResultado: %s\n", lines, parser());
    fclose(fin);
    return 0;
  }
}

