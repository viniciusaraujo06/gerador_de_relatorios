#ifndef MODELO_H
#define MODELO_H

// O MODELO DE UM RELATORIO
// Modelo nao e uma lista: e UMA struct por relatorio, com o que o usuario escolheu
// para cada area. Dentro dele ficam 3 listas: campos, secoes e, em cada secao, linhas.
//
//   +-----------------------------------+
//   | CONSUMO DE FILMES POR USUARIO     |   area 1: cabecalho (titulo e data)
//   | Gerado em 30/10/2026 23:24        |
//   +-----------------------------------+
//
//   +-----------------------------------+
//   | Usuario: Mariana Costa            |   area 2: corpo, dividido em secoes;
//   +-----------------------------------+   cada secao e um grupo de linhas com
//   +--------------+----------+---------+   o mesmo valor num campo (aqui: Usuario)
//   | Midia        | Favorito |    Nota |
//   +--------------+----------+---------+
//   | Inception    | Nao      |     8,0 |   uma Linha por registro; as colunas
//   | Interstellar | Sim      |     9,0 |   sao os Campos escolhidos
//   +--------------+----------+---------+
//   +-----------------------------------+
//   | Registros na secao: 2             |   resumo da secao
//   +-----------------------------------+
//
//   +-----------------------------------+
//   | Total de registros: 5             |   area 3: rodape (total e somas)
//   +-----------------------------------+
//
// Sem divisao em secoes, todas as linhas ficam numa secao so, sem titulo.
// Este arquivo so monta e consulta as listas; quem desenha o TXT e o impressao.c

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
  int largura;         // calculada na hora de imprimir
  struct Campo *prox;
} Campo;

// uma linha do corpo: um texto por coluna, ainda sem formatar ("39.90", "Ana")
typedef struct Linha {
  char valor[MAX_CAMPOS][TAM_VALOR];
  struct Linha *prox;
} Linha;

// um grupo de linhas do corpo (ex.: todas as linhas de um usuario)
typedef struct Secao {
  char titulo[TAM_VALOR]; // "Usuario: Ana Silva", ou "" quando o corpo nao e dividido
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

// recebe a linha como no printf, com os valores separados por ';', e poe na secao certa
// ex.: adicionar_linha(&modelo, "%d;%s;%.2f", id, nome, valor);
void adicionar_linha(Modelo *modelo, const char *formato, ...);

void liberar_modelo(Modelo *modelo);

// escreve o valor do jeito que o campo pede: "39.90" vira "R$ 39,90" (moeda)
void formatar_valor(const Campo *c, const char *valor, char *destino);

// textos do fim de uma secao e do rodape ("Registros...", "Soma de..."); devolvem quantos sao
int resumo_secao(const Modelo *modelo, const Secao *s, char texto[][TAM_VALOR]);
int resumo_rodape(const Modelo *modelo, char texto[][TAM_VALOR]);

int total_registros(const Modelo *modelo);

#endif
