#include "modelo.h"
#include "leitura.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// MONTAGEM DAS LISTAS
//

void iniciar_modelo(Modelo *modelo, const char *titulo, const char *campos) {
  char lista[MAX_CAMPOS * TAM_NOME];
  char *cursor = lista;
  Campo *fim = NULL;

  snprintf(modelo->titulo, TAM_VALOR, "%s", titulo);
  modelo->mostrar_data = 1;
  modelo->campos = NULL;
  modelo->qtd_campos = 0;
  modelo->coluna_secao = -1;
  modelo->nome_secao[0] = '\0';
  modelo->secoes = modelo->ultima_secao = NULL;
  modelo->mostrar_total = 1;

  // separa "ID:I;Nome;Valor:M+" com a mesma funcao que le os arquivos
  snprintf(lista, sizeof(lista), "%s", campos);
  while (cursor != NULL && modelo->qtd_campos < MAX_CAMPOS) {
    char *nome = proximo_campo(&cursor);
    char *tipo = strchr(nome, ':'); // "Valor:M+" -> nome "Valor", tipo "M+"
    if (tipo != NULL) {
      *tipo = '\0';
      tipo++;
    }

    Campo *c = alocar(sizeof(Campo));
    if (c == NULL)
      break;
    c->coluna = modelo->qtd_campos++;
    snprintf(c->nome, TAM_NOME, "%s", nome);
    c->tipo = tipo != NULL ? tipo[0] : TEXTO;
    c->casas = c->tipo == MOEDA ? 2 : 1;
    c->somar = tipo != NULL && strchr(tipo, '+') != NULL;
    c->largura = 0;

    c->prox = NULL;
    if (modelo->campos == NULL)
      modelo->campos = c;
    else
      fim->prox = c;
    fim = c;
  }
}

Campo *buscar_campo(Modelo *modelo, int coluna) {
  for (Campo *c = modelo->campos; c != NULL; c = c->prox)
    if (c->coluna == coluna)
      return c;
  return NULL;
}

static void liberar_campos(Campo *c) {
  while (c != NULL) {
    Campo *prox = c->prox;
    free(c);
    c = prox;
  }
}

// monta uma lista nova com copias dos campos escolhidos e joga a antiga fora
void escolher_campos(Modelo *modelo, const int numeros[], int qtd) {
  Campo *nova = NULL, *fim = NULL;

  for (int i = 0; i < qtd; i++) {
    Campo *c = buscar_campo(modelo, numeros[i] - 1);
    if (c == NULL)
      continue; // numero que nao existe
    Campo *copia = alocar(sizeof(Campo));
    if (copia == NULL)
      break;
    *copia = *c;

    copia->prox = NULL;
    if (nova == NULL)
      nova = copia;
    else
      fim->prox = copia;
    fim = copia;
  }

  if (nova == NULL)
    return; // nenhum numero valido: continua com todos
  liberar_campos(modelo->campos);
  modelo->campos = nova;
}

void definir_secao(Modelo *modelo, int coluna) {
  Campo *c = buscar_campo(modelo, coluna);
  modelo->coluna_secao = c != NULL ? coluna : -1;
  snprintf(modelo->nome_secao, TAM_NOME, "%s", c != NULL ? c->nome : "");
}

// acha a secao com esse titulo; se ainda nao existe, cria no fim da lista
static Secao *secao_com_titulo(Modelo *modelo, const char *titulo) {
  Secao *s;
  for (s = modelo->secoes; s != NULL; s = s->prox)
    if (strcmp(s->titulo, titulo) == 0)
      return s;

  s = alocar(sizeof(Secao));
  if (s == NULL)
    return NULL;
  snprintf(s->titulo, TAM_VALOR, "%s", titulo);
  s->qtd = 0;
  s->linhas = s->ultima = NULL;

  s->prox = NULL;
  if (modelo->secoes == NULL)
    modelo->secoes = s;
  else
    modelo->ultima_secao->prox = s;
  modelo->ultima_secao = s;
  return s;
}

void adicionar_linha(Modelo *modelo, const char *formato, ...) {
  char texto[MAX_CAMPOS * TAM_VALOR], titulo[TAM_VALOR] = "";
  char *cursor = texto;
  va_list valores;

  Linha *l = alocar(sizeof(Linha));
  if (l == NULL)
    return;

  // escreve a linha igual a uma linha dos arquivos de entrada e separa os campos
  va_start(valores, formato);
  vsnprintf(texto, sizeof(texto), formato, valores);
  va_end(valores);
  for (int i = 0; i < MAX_CAMPOS; i++)
    snprintf(l->valor[i], TAM_VALOR, "%s", proximo_campo(&cursor));

  // sem divisao, todas as linhas vao para uma secao so (titulo "")
  if (modelo->coluna_secao >= 0)
    snprintf(titulo, TAM_VALOR, "%.30s: %.60s", modelo->nome_secao, l->valor[modelo->coluna_secao]);
  Secao *s = secao_com_titulo(modelo, titulo);
  if (s == NULL) {
    free(l);
    return;
  }

  l->prox = NULL;
  if (s->linhas == NULL)
    s->linhas = l;
  else
    s->ultima->prox = l;
  s->ultima = l;
  s->qtd++;
}

void liberar_modelo(Modelo *modelo) {
  Secao *s = modelo->secoes;
  while (s != NULL) {
    Linha *l = s->linhas;
    while (l != NULL) {
      Linha *prox = l->prox;
      free(l);
      l = prox;
    }
    Secao *prox = s->prox;
    free(s);
    s = prox;
  }
  liberar_campos(modelo->campos);
  modelo->secoes = modelo->ultima_secao = NULL;
  modelo->campos = NULL;
}

//
// FORMATO E RESUMOS (o que vai no fim de cada secao e no rodape)
//

// escreve o valor do jeito que o campo pede: "39.90" vira "R$ 39,90" (moeda)
void formatar_valor(const Campo *c, const char *valor, char *destino) {
  char *fim;
  double numero = strtod(valor, &fim);

  if (c->tipo == TEXTO || fim == valor) { // texto, ou numero que falta ("-")
    snprintf(destino, TAM_VALOR, "%s", valor);
    return;
  }
  snprintf(destino, TAM_VALOR, "%s%.*f", c->tipo == MOEDA ? "R$ " : "", c->tipo == INTEIRO ? 0 : c->casas, numero);
  char *ponto = strchr(destino, '.');
  if (ponto != NULL)
    *ponto = ','; // padrao brasileiro
}

// soma uma coluna numa lista de linhas
static double somar_linhas(const Linha *l, int coluna) {
  double soma = 0;
  for (; l != NULL; l = l->prox)
    soma += atof(l->valor[coluna]);
  return soma;
}

// "Soma de Valor: R$ 79,80"
static void texto_soma(const Campo *c, double soma, char *destino) {
  char numero[TAM_VALOR], valor[TAM_VALOR];
  snprintf(numero, TAM_VALOR, "%f", soma);
  formatar_valor(c, numero, valor);
  snprintf(destino, TAM_VALOR, "Soma de %.30s: %.60s", c->nome, valor);
}

int resumo_secao(const Modelo *modelo, const Secao *s, char texto[][TAM_VALOR]) {
  int n = 0;
  snprintf(texto[n++], TAM_VALOR, "Registros na secao: %d", s->qtd);
  for (const Campo *c = modelo->campos; c != NULL; c = c->prox)
    if (c->somar && c->tipo != TEXTO)
      texto_soma(c, somar_linhas(s->linhas, c->coluna), texto[n++]);
  return n;
}

int resumo_rodape(const Modelo *modelo, char texto[][TAM_VALOR]) {
  int n = 0;
  if (modelo->mostrar_total)
    snprintf(texto[n++], TAM_VALOR, "Total de registros: %d", total_registros(modelo));
  for (const Campo *c = modelo->campos; c != NULL; c = c->prox)
    if (c->somar && c->tipo != TEXTO) {
      double soma = 0;
      for (const Secao *s = modelo->secoes; s != NULL; s = s->prox)
        soma += somar_linhas(s->linhas, c->coluna);
      texto_soma(c, soma, texto[n++]);
    }
  return n;
}

int total_registros(const Modelo *modelo) {
  int total = 0;
  for (const Secao *s = modelo->secoes; s != NULL; s = s->prox)
    total += s->qtd;
  return total;
}
