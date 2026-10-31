#include "modelo.h"
#include "leitura.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEM_REGISTROS "Nenhum registro encontrado"

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
    snprintf(titulo, TAM_VALOR, "%.30s: %.60s", modelo->nome_secao,
             l->valor[modelo->coluna_secao]);
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

static void liberar_modelo(Modelo *modelo) {
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
// GERACAO DO TXT
//

// escreve o valor do jeito que o campo pede: "39.90" vira "R$ 39,90" (moeda)
static void formatar(const Campo *c, const char *valor, char *destino) {
  char *fim;
  double numero = strtod(valor, &fim);

  if (c->tipo == TEXTO || fim == valor) { // texto, ou numero que falta ("-")
    snprintf(destino, TAM_VALOR, "%s", valor);
    return;
  }
  snprintf(destino, TAM_VALOR, "%s%.*f", c->tipo == MOEDA ? "R$ " : "",
           c->tipo == INTEIRO ? 0 : c->casas, numero);
  char *ponto = strchr(destino, '.');
  if (ponto != NULL)
    *ponto = ','; // padrao brasileiro
}

// soma a coluna nas linhas de uma secao (ou de todas, quando s == NULL)
static double somar(const Modelo *modelo, const Secao *s, int coluna) {
  double soma = 0;
  for (const Secao *x = modelo->secoes; x != NULL; x = x->prox)
    if (s == NULL || x == s)
      for (const Linha *l = x->linhas; l != NULL; l = l->prox)
        soma += atof(l->valor[coluna]);
  return soma;
}

static int contar(const Modelo *modelo, const Secao *s) {
  int total = 0;
  for (const Secao *x = modelo->secoes; x != NULL; x = x->prox)
    if (s == NULL || x == s)
      total += x->qtd;
  return total;
}

// textos do fim de uma secao (s != NULL) ou do rodape (s == NULL); devolve quantos sao
static int montar_resumo(const Modelo *modelo, const Secao *s,
                         char texto[][TAM_VALOR]) {
  char numero[TAM_VALOR], valor[TAM_VALOR];
  int n = 0;

  if (s != NULL || modelo->mostrar_total)
    snprintf(texto[n++], TAM_VALOR, "%s: %d",
             s != NULL ? "Registros na secao" : "Total de registros",
             contar(modelo, s));

  for (const Campo *c = modelo->campos; c != NULL; c = c->prox)
    if (c->somar && c->tipo != TEXTO) {
      snprintf(numero, TAM_VALOR, "%f", somar(modelo, s, c->coluna));
      formatar(c, numero, valor);
      snprintf(texto[n++], TAM_VALOR, "Soma de %.30s: %.60s", c->nome, valor);
    }
  return n;
}

static int maior(int largura, const char *texto) {
  int n = (int)strlen(texto);
  return n > largura ? n : largura;
}

// largura do texto numa linha que ocupa a tabela toda: "| texto |"
static int largura_interna(const Modelo *modelo) {
  int total = 0;
  for (const Campo *c = modelo->campos; c != NULL; c = c->prox)
    total += c->largura + 3; // "| " + valor + " "
  return total - 3;
}

// cada coluna fica do tamanho do maior valor; se titulo, secoes ou rodape
// nao couberem na tabela, a ultima coluna cresce
static void calcular_larguras(Modelo *modelo, const char *data) {
  char texto[MAX_CAMPOS + 1][TAM_VALOR];
  Campo *c, *ultimo = NULL;
  int precisa = maior(maior(0, modelo->titulo), data), n, i;

  for (c = modelo->campos; c != NULL; c = c->prox) {
    c->largura = maior(0, c->nome);
    for (Secao *s = modelo->secoes; s != NULL; s = s->prox)
      for (Linha *l = s->linhas; l != NULL; l = l->prox) {
        formatar(c, l->valor[c->coluna], texto[0]);
        c->largura = maior(c->largura, texto[0]);
      }
    ultimo = c;
  }

  if (modelo->secoes == NULL)
    precisa = maior(precisa, SEM_REGISTROS);
  if (modelo->coluna_secao >= 0)
    for (Secao *s = modelo->secoes; s != NULL; s = s->prox) {
      precisa = maior(precisa, s->titulo);
      n = montar_resumo(modelo, s, texto);
      for (i = 0; i < n; i++)
        precisa = maior(precisa, texto[i]);
    }
  n = montar_resumo(modelo, NULL, texto);
  for (i = 0; i < n; i++)
    precisa = maior(precisa, texto[i]);

  if (ultimo != NULL && precisa > largura_interna(modelo))
    ultimo->largura += precisa - largura_interna(modelo);
}

// +-----+------+ (com colunas) ou +------------+ (sem colunas)
static void borda(FILE *f, const Modelo *modelo, int com_colunas) {
  fputc('+', f);
  for (const Campo *c = modelo->campos; c != NULL; c = c->prox) {
    for (int j = 0; j < c->largura + 2; j++)
      fputc('-', f);
    fputc(c->prox == NULL || com_colunas ? '+' : '-', f);
  }
  fputc('\n', f);
}

static void linha_cheia(FILE *f, const Modelo *modelo, const char *texto) {
  fprintf(f, "| %-*s |\n", largura_interna(modelo), texto);
}

// uma linha da tabela; com l == NULL escreve os nomes dos campos
// texto fica alinhado a esquerda e numero a direita
static void linha_tabela(FILE *f, const Modelo *modelo, const Linha *l) {
  char valor[TAM_VALOR];
  for (const Campo *c = modelo->campos; c != NULL; c = c->prox) {
    if (l != NULL)
      formatar(c, l->valor[c->coluna], valor);
    else
      snprintf(valor, TAM_VALOR, "%s", c->nome);
    fprintf(f, c->tipo == TEXTO ? "| %-*s " : "| %*s ", c->largura, valor);
  }
  fprintf(f, "|\n");
}

static void cabecalho_tabela(FILE *f, const Modelo *modelo) {
  borda(f, modelo, 1);
  linha_tabela(f, modelo, NULL);
  borda(f, modelo, 1);
}

void gravar_relatorio(Modelo *modelo, const char *caminho) {
  char data[40] = "", texto[MAX_CAMPOS + 1][TAM_VALOR];
  int n, i;
  FILE *f = fopen(caminho, "w");

  if (f == NULL) {
    printf("Erro ao criar o arquivo '%s' (a pasta output existe?)\n", caminho);
    liberar_modelo(modelo);
    return;
  }

  if (modelo->mostrar_data) {
    time_t agora = time(NULL);
    strftime(data, sizeof(data), "Gerado em %d/%m/%Y %H:%M", localtime(&agora));
  }
  calcular_larguras(modelo, data);

  // area 1: cabecalho
  borda(f, modelo, 0);
  linha_cheia(f, modelo, modelo->titulo);
  if (modelo->mostrar_data)
    linha_cheia(f, modelo, data);
  borda(f, modelo, 0);

  // area 2: corpo, uma tabela por secao
  if (modelo->secoes == NULL) {
    fputc('\n', f);
    cabecalho_tabela(f, modelo);
    linha_cheia(f, modelo, SEM_REGISTROS);
    borda(f, modelo, 0);
  }
  for (Secao *s = modelo->secoes; s != NULL; s = s->prox) {
    fputc('\n', f);
    if (modelo->coluna_secao >= 0) {
      borda(f, modelo, 0);
      linha_cheia(f, modelo, s->titulo);
    }
    cabecalho_tabela(f, modelo);
    for (Linha *l = s->linhas; l != NULL; l = l->prox)
      linha_tabela(f, modelo, l);
    borda(f, modelo, 1);

    if (modelo->coluna_secao >= 0) { // fim da secao
      n = montar_resumo(modelo, s, texto);
      for (i = 0; i < n; i++)
        linha_cheia(f, modelo, texto[i]);
      borda(f, modelo, 0);
    }
  }

  // area 3: rodape
  n = montar_resumo(modelo, NULL, texto);
  if (n > 0) {
    fputc('\n', f);
    borda(f, modelo, 0);
    for (i = 0; i < n; i++)
      linha_cheia(f, modelo, texto[i]);
    borda(f, modelo, 0);
  }

  if (fclose(f) != 0)
    printf("Erro ao gravar o arquivo '%s'\n", caminho);
  else
    printf("Relatorio gerado em '%s' (%d registros)\n", caminho,
           contar(modelo, NULL));
  liberar_modelo(modelo);
}
