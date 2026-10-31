#include "leitura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 1024

void *alocar(size_t tamanho) {
  void *p = malloc(tamanho);
  if (p == NULL)
    printf("Erro de alocacao de memoria\n");
  return p;
}

// parecido com o strtok, mas nao pula campo vazio: "1;;x" sao 3 campos
// (o strtok veria so 2 e deslocaria as colunas seguintes)
char *proximo_campo(char **cursor) {
  static char vazio[1] = "";
  char *inicio = *cursor;
  if (inicio == NULL)
    return vazio;

  char *separador = inicio + strcspn(inicio, ";\t");
  if (*separador != '\0') {
    *separador = '\0';
    *cursor = separador + 1;
  } else {
    *cursor = NULL; // era o ultimo campo da linha
  }
  return inicio;
}

static int campo_inteiro(char **cursor) { return atoi(proximo_campo(cursor)); }

static double campo_numero(char **cursor) { return atof(proximo_campo(cursor)); }

static void campo_texto(char **cursor, char *destino, size_t tamanho) {
  snprintf(destino, tamanho, "%s", proximo_campo(cursor));
}

static FILE *abrir(const char *caminho) {
  FILE *f = fopen(caminho, "r");
  if (f == NULL)
    printf("Erro ao abrir arquivo '%s'\n", caminho);
  return f;
}

// le a proxima linha nao vazia, ja sem o "\n"; devolve 0 no fim do arquivo
static int proxima_linha(FILE *f, char *linha) {
  while (fgets(linha, TAM_LINHA, f) != NULL) {
    linha[strcspn(linha, "\r\n")] = '\0';
    if (linha[0] != '\0')
      return 1;
  }
  return 0;
}

// todas as leituras seguem o mesmo roteiro:
// abre o arquivo, cria um no por linha, preenche campo a campo
// e poe o no no fim da lista (inicio = primeiro no, fim = ultimo)

Assinatura *ler_assinaturas(const char *caminho) {
  Assinatura *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    Assinatura *a = alocar(sizeof(Assinatura));
    if (a == NULL)
      break;
    char *cursor = linha;
    a->id = campo_inteiro(&cursor);
    a->valor = campo_numero(&cursor);
    campo_texto(&cursor, a->metodo_pagamento, sizeof(a->metodo_pagamento));
    campo_texto(&cursor, a->vencimento, sizeof(a->vencimento));
    campo_texto(&cursor, a->tipo, sizeof(a->tipo));

    a->prox = NULL;
    if (inicio == NULL)
      inicio = a;
    else
      fim->prox = a;
    fim = a;
  }
  fclose(f);
  return inicio;
}

Usuario *ler_usuarios(const char *caminho) {
  Usuario *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    Usuario *u = alocar(sizeof(Usuario));
    if (u == NULL)
      break;
    char *cursor = linha;
    u->id = campo_inteiro(&cursor);
    campo_texto(&cursor, u->nome, sizeof(u->nome));
    campo_texto(&cursor, u->email, sizeof(u->email));
    campo_texto(&cursor, u->cpf, sizeof(u->cpf));
    u->id_assinatura = campo_inteiro(&cursor);

    u->prox = NULL;
    if (inicio == NULL)
      inicio = u;
    else
      fim->prox = u;
    fim = u;
  }
  fclose(f);
  return inicio;
}

Midia *ler_midias(const char *caminho) {
  Midia *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    Midia *m = alocar(sizeof(Midia));
    if (m == NULL)
      break;
    char *cursor = linha;
    m->id = campo_inteiro(&cursor);
    campo_texto(&cursor, m->nome, sizeof(m->nome));
    campo_texto(&cursor, m->categoria, sizeof(m->categoria));
    campo_texto(&cursor, m->diretor, sizeof(m->diretor));
    m->duracao = campo_inteiro(&cursor);
    campo_texto(&cursor, m->data_lancamento, sizeof(m->data_lancamento));
    m->avaliacao_critica = (float)campo_numero(&cursor);
    m->tipo = proximo_campo(&cursor)[0];

    m->prox = NULL;
    if (inicio == NULL)
      inicio = m;
    else
      fim->prox = m;
    fim = m;
  }
  fclose(f);
  return inicio;
}

Filme *ler_filmes(const char *caminho) {
  Filme *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    Filme *filme = alocar(sizeof(Filme));
    if (filme == NULL)
      break;
    char *cursor = linha;
    filme->id_midia = campo_inteiro(&cursor);
    filme->bilheteria = campo_numero(&cursor);

    filme->prox = NULL;
    if (inicio == NULL)
      inicio = filme;
    else
      fim->prox = filme;
    fim = filme;
  }
  fclose(f);
  return inicio;
}

Serie *ler_series(const char *caminho) {
  Serie *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    Serie *s = alocar(sizeof(Serie));
    if (s == NULL)
      break;
    char *cursor = linha;
    s->id_midia = campo_inteiro(&cursor);
    s->numero_episodios = campo_inteiro(&cursor);
    s->numero_temporadas = campo_inteiro(&cursor);

    s->prox = NULL;
    if (inicio == NULL)
      inicio = s;
    else
      fim->prox = s;
    fim = s;
  }
  fclose(f);
  return inicio;
}

MidiaConsumida *ler_midias_consumidas(const char *caminho) {
  MidiaConsumida *inicio = NULL, *fim = NULL;
  char linha[TAM_LINHA];
  FILE *f = abrir(caminho);
  if (f == NULL)
    return NULL;

  while (proxima_linha(f, linha)) {
    MidiaConsumida *mc = alocar(sizeof(MidiaConsumida));
    if (mc == NULL)
      break;
    char *cursor = linha;
    mc->id_usuario = campo_inteiro(&cursor);
    mc->id_midia = campo_inteiro(&cursor);
    mc->favorito = campo_inteiro(&cursor);
    mc->avaliacao_usuario = (float)campo_numero(&cursor);

    mc->prox = NULL;
    if (inicio == NULL)
      inicio = mc;
    else
      fim->prox = mc;
    fim = mc;
  }
  fclose(f);
  return inicio;
}

// liberacao: guarda o proximo antes de dar free no atual

void liberar_assinaturas(Assinatura *lista) {
  while (lista != NULL) {
    Assinatura *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_usuarios(Usuario *lista) {
  while (lista != NULL) {
    Usuario *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_midias(Midia *lista) {
  while (lista != NULL) {
    Midia *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_filmes(Filme *lista) {
  while (lista != NULL) {
    Filme *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_series(Serie *lista) {
  while (lista != NULL) {
    Serie *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}

void liberar_midias_consumidas(MidiaConsumida *lista) {
  while (lista != NULL) {
    MidiaConsumida *prox = lista->prox;
    free(lista);
    lista = prox;
  }
}
