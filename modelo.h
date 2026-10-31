#ifndef MODELO_H
#define MODELO_H

// O MODELO DE UM RELATORIO
// area 1 cabecalho, area 2 corpo (campos + secoes com linhas) e area 3 rodape.
// campos, secoes e linhas sao listas dinamicas, montadas na hora de gerar o relatorio

#define TAM_VALOR 100
#define TAM_NOME 30
#define MAX_CAMPOS 12

// tipos de campo
#define TEXTO 'T'
#define INTEIRO 'I'
#define DECIMAL 'D'
#define MOEDA 'M'

// um campo (coluna) do relatorio
typedef struct Campo {
  int coluna;          // posicao do valor dentro da Linha
  char nome[TAM_NOME]; // titulo da coluna
  char tipo;           // TEXTO, INTEIRO, DECIMAL ou MOEDA
  int casas;           // formato: casas decimais (decimal e moeda)
  int somar;           // 1 = mostra a soma no fim de cada secao e no rodape
  int largura;         // calculada na hora de gravar
  struct Campo *prox;
} Campo;

// uma linha do corpo: um texto por coluna, ainda sem formatar ("39.90", "Ana")
typedef struct Linha {
  char valor[MAX_CAMPOS][TAM_VALOR];
  struct Linha *prox;
} Linha;

// o corpo pode ser dividido em secoes (ex.: uma secao por usuario)
typedef struct Secao {
  char titulo[TAM_VALOR]; // "" quando o corpo nao e dividido
  int qtd;                // quantas linhas tem
  Linha *linhas, *ultima;
  struct Secao *prox;
} Secao;

typedef struct Modelo {
  // area 1: cabecalho
  char titulo[TAM_VALOR];
  int mostrar_data;
  // area 2: corpo
  Campo *campos;             // campos que aparecem, na ordem
  int qtd_campos;            // quantos campos o relatorio oferece
  int coluna_secao;          // campo que divide o corpo em secoes (-1 = nao divide)
  char nome_secao[TAM_NOME];
  Secao *secoes, *ultima_secao;
  // area 3: rodape
  int mostrar_total;
} Modelo;

// campos no formato "nome:tipo", separados por ';' - ex.: "ID:I;Nome;Valor:M+"
// sem tipo = texto; '+' no fim = somar esse campo
void iniciar_modelo(Modelo *modelo, const char *titulo, const char *campos);

Campo *buscar_campo(Modelo *modelo, int coluna);

// deixa so os campos escolhidos, na ordem pedida (numeros como aparecem na tela)
void escolher_campos(Modelo *modelo, const int numeros[], int qtd);

// divide o corpo em secoes pelo valor da coluna (-1 = nao divide)
void definir_secao(Modelo *modelo, int coluna);

// recebe a linha como no printf, com os valores separados por ';'
// ex.: adicionar_linha(&modelo, "%d;%s;%.2f", id, nome, valor);
void adicionar_linha(Modelo *modelo, const char *formato, ...);

// escreve o TXT e libera as listas do modelo
void gravar_relatorio(Modelo *modelo, const char *caminho);

#endif
