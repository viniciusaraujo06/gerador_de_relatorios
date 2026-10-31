#include "impressao.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// ate 1 linha de quantidade + 1 soma por campo
#define MAX_TEXTOS (MAX_CAMPOS + 1)

// cada coluna fica do tamanho do maior valor (ou do nome do campo), olhando as linhas
// de todas as secoes, para as tabelas de todas as secoes ficarem iguais
static void calcular_larguras(Modelo *modelo) {
  char valor[TAM_VALOR];
  for (Campo *c = modelo->campos; c != NULL; c = c->prox) {
    c->largura = (int)strlen(c->nome);
    for (Secao *s = modelo->secoes; s != NULL; s = s->prox)
      for (Linha *l = s->linhas; l != NULL; l = l->prox) {
        formatar_valor(c, l->valor[c->coluna], valor);
        if ((int)strlen(valor) > c->largura)
          c->largura = (int)strlen(valor);
      }
  }
}

//
// TABELA: so sabe desenhar campos e linhas, nao sabe o que e secao
//

// largura total da tabela: "| a | b |"
static int largura_tabela(const Campo *campos) {
  int total = 1;
  for (const Campo *c = campos; c != NULL; c = c->prox)
    total += c->largura + 3; // " " + valor + " |"
  return total;
}

static void borda_tabela(FILE *f, const Campo *campos) {
  for (const Campo *c = campos; c != NULL; c = c->prox) {
    fputc('+', f);
    for (int j = 0; j < c->largura + 2; j++)
      fputc('-', f);
  }
  fprintf(f, "+\n");
}

// uma linha da tabela; com l == NULL escreve os nomes dos campos
// texto fica alinhado a esquerda e numero a direita
static void linha_tabela(FILE *f, const Campo *campos, const Linha *l) {
  char valor[TAM_VALOR];
  for (const Campo *c = campos; c != NULL; c = c->prox) {
    if (l != NULL)
      formatar_valor(c, l->valor[c->coluna], valor);
    else
      snprintf(valor, TAM_VALOR, "%s", c->nome);
    fprintf(f, c->tipo == TEXTO ? "| %-*s " : "| %*s ", c->largura, valor);
  }
  fprintf(f, "|\n");
}

static void imprimir_tabela(FILE *f, const Campo *campos, const Linha *linhas) {
  borda_tabela(f, campos);
  linha_tabela(f, campos, NULL);
  borda_tabela(f, campos);
  for (const Linha *l = linhas; l != NULL; l = l->prox)
    linha_tabela(f, campos, l);
  borda_tabela(f, campos);
}

//
// CAIXA: textos que ocupam a largura toda (cabecalho, titulo e resumo da secao, rodape)
//

static void borda_caixa(FILE *f, int largura) {
  fputc('+', f);
  for (int j = 0; j < largura + 2; j++)
    fputc('-', f);
  fprintf(f, "+\n");
}

// fica da largura da tabela; se algum texto for maior, a caixa cresce
static void imprimir_caixa(FILE *f, char texto[][TAM_VALOR], int n, int largura_minima) {
  int largura = largura_minima - 4; // tira "| " e " |"
  for (int i = 0; i < n; i++)
    if ((int)strlen(texto[i]) > largura)
      largura = (int)strlen(texto[i]);

  borda_caixa(f, largura);
  for (int i = 0; i < n; i++)
    fprintf(f, "| %-*s |\n", largura, texto[i]);
  borda_caixa(f, largura);
}

//
// RELATORIO: junta as pecas na ordem das areas
//

void gravar_relatorio(Modelo *modelo, const char *caminho) {
  char texto[MAX_TEXTOS][TAM_VALOR];
  int n = 0;
  FILE *f = fopen(caminho, "w");
  if (f == NULL) {
    printf("Erro ao criar o arquivo '%s' (a pasta output existe?)\n", caminho);
    return;
  }

  calcular_larguras(modelo);
  int largura = largura_tabela(modelo->campos);

  // area 1: cabecalho
  snprintf(texto[n++], TAM_VALOR, "%s", modelo->titulo);
  if (modelo->mostrar_data) {
    time_t agora = time(NULL);
    strftime(texto[n++], TAM_VALOR, "Gerado em %d/%m/%Y %H:%M", localtime(&agora));
  }
  imprimir_caixa(f, texto, n, largura);

  // area 2: corpo, uma tabela por secao
  if (modelo->secoes == NULL) {
    fputc('\n', f);
    imprimir_tabela(f, modelo->campos, NULL);
    snprintf(texto[0], TAM_VALOR, "Nenhum registro encontrado");
    imprimir_caixa(f, texto, 1, largura);
  }
  for (Secao *s = modelo->secoes; s != NULL; s = s->prox) {
    fputc('\n', f);
    if (modelo->coluna_secao >= 0) {
      snprintf(texto[0], TAM_VALOR, "%s", s->titulo);
      imprimir_caixa(f, texto, 1, largura);
    }
    imprimir_tabela(f, modelo->campos, s->linhas);
    if (modelo->coluna_secao >= 0) {
      n = resumo_secao(modelo, s, texto);
      imprimir_caixa(f, texto, n, largura);
    }
  }

  // area 3: rodape
  n = resumo_rodape(modelo, texto);
  if (n > 0) {
    fputc('\n', f);
    imprimir_caixa(f, texto, n, largura);
  }

  if (fclose(f) != 0)
    printf("Erro ao gravar o arquivo '%s'\n", caminho);
  else
    printf("Relatorio gerado em '%s' (%d registros)\n", caminho, total_registros(modelo));
}
