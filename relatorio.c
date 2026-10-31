#include "relatorio.h"
#include "interface.h"
#include "modelo.h"
#include <stdio.h>

// todo relatorio segue o mesmo roteiro:
// 1. iniciar_modelo: titulo e campos padrao
// 2. configurar_modelo: o usuario muda o que quiser em cada area
// 3. adicionar_linha: uma linha por registro (valores separados por ';')
// 4. gravar_relatorio: escreve o TXT e libera as listas

//
// BUSCAS PELA CHAVE (ID) NAS LISTAS
//

static Assinatura *buscar_assinatura(Assinatura *lista, int id) {
  for (; lista != NULL; lista = lista->prox)
    if (lista->id == id)
      return lista;
  return NULL;
}

static Usuario *buscar_usuario(Usuario *lista, int id) {
  for (; lista != NULL; lista = lista->prox)
    if (lista->id == id)
      return lista;
  return NULL;
}

static Midia *buscar_midia(Midia *lista, int id) {
  for (; lista != NULL; lista = lista->prox)
    if (lista->id == id)
      return lista;
  return NULL;
}

static Filme *buscar_filme(Filme *lista, int id_midia) {
  for (; lista != NULL; lista = lista->prox)
    if (lista->id_midia == id_midia)
      return lista;
  return NULL;
}

static Serie *buscar_serie(Serie *lista, int id_midia) {
  for (; lista != NULL; lista = lista->prox)
    if (lista->id_midia == id_midia)
      return lista;
  return NULL;
}

//
// RELATORIOS
//

void gerarRelatorioUsuariosAssinaturas(Usuario *usuarios, Assinatura *assinaturas, const char *caminho_saida) {
  Modelo modelo;
  iniciar_modelo(&modelo, "RELATORIO DE USUARIOS E ASSINATURAS", "ID:I;Nome;Email;CPF;Plano;Valor:M;Pagamento;Vencimento");
  configurar_modelo(&modelo);

  for (Usuario *u = usuarios; u != NULL; u = u->prox) {
    Assinatura *a = buscar_assinatura(assinaturas, u->id_assinatura);
    if (a != NULL)
      adicionar_linha(&modelo, "%d;%s;%s;%s;%s;%.2f;%s;%s", u->id, u->nome, u->email, u->cpf, a->tipo, a->valor, a->metodo_pagamento, a->vencimento);
    else
      adicionar_linha(&modelo, "%d;%s;%s;%s;Nenhuma;-;-;-", u->id, u->nome, u->email, u->cpf);
  }
  gravar_relatorio(&modelo, caminho_saida);
}

// os 3 catalogos comecam com os mesmos campos da midia
#define CAMPOS_MIDIA "ID:I;Nome;Categoria;Diretor;Duracao (min):I;Lancamento;Avaliacao:D"

static void adicionar_midia(Modelo *modelo, const Midia *m, const char *resto) {
  adicionar_linha(modelo, "%d;%s;%s;%s;%d;%s;%.1f;%s", m->id, m->nome, m->categoria, m->diretor, m->duracao, m->data_lancamento, m->avaliacao_critica, resto);
}

void gerarRelatorioCatalogoMidias(Midia *midias, Filme *filmes, Serie *series, const char *caminho_saida) {
  Modelo modelo;
  char resto[TAM_VALOR];
  iniciar_modelo(&modelo, "CATALOGO DE MIDIAS", CAMPOS_MIDIA ";Tipo;Bilheteria:M;Episodios:I;Temporadas:I");
  configurar_modelo(&modelo);

  for (Midia *m = midias; m != NULL; m = m->prox) {
    Filme *f = buscar_filme(filmes, m->id);
    Serie *s = buscar_serie(series, m->id);
    if (f != NULL)
      snprintf(resto, TAM_VALOR, "Filme;%.2f;-;-", f->bilheteria);
    else if (s != NULL)
      snprintf(resto, TAM_VALOR, "Serie;-;%d;%d", s->numero_episodios, s->numero_temporadas);
    else
      snprintf(resto, TAM_VALOR, "%s;-;-;-", m->tipo == 'S' ? "Serie" : "Filme");
    adicionar_midia(&modelo, m, resto);
  }
  gravar_relatorio(&modelo, caminho_saida);
}

void gerarRelatorioCatalogoSeries(Midia *midias, Serie *series, const char *caminho_saida) {
  Modelo modelo;
  char resto[TAM_VALOR];
  iniciar_modelo(&modelo, "CATALOGO DE SERIES", CAMPOS_MIDIA ";Episodios:I;Temporadas:I");
  configurar_modelo(&modelo);

  for (Midia *m = midias; m != NULL; m = m->prox) {
    Serie *s = buscar_serie(series, m->id);
    if (s == NULL)
      continue; // nao e serie
    snprintf(resto, TAM_VALOR, "%d;%d", s->numero_episodios, s->numero_temporadas);
    adicionar_midia(&modelo, m, resto);
  }
  gravar_relatorio(&modelo, caminho_saida);
}

void gerarRelatorioCatalogoFilmes(Midia *midias, Filme *filmes, const char *caminho_saida) {
  Modelo modelo;
  char resto[TAM_VALOR];
  iniciar_modelo(&modelo, "CATALOGO DE FILMES", CAMPOS_MIDIA ";Bilheteria:M+");
  configurar_modelo(&modelo);

  for (Midia *m = midias; m != NULL; m = m->prox) {
    Filme *f = buscar_filme(filmes, m->id);
    if (f == NULL)
      continue; // nao e filme
    snprintf(resto, TAM_VALOR, "%.2f", f->bilheteria);
    adicionar_midia(&modelo, m, resto);
  }
  gravar_relatorio(&modelo, caminho_saida);
}

// um relatorio so para os 3 consumos: o que muda e o tipo de midia que entra
void gerarRelatorioConsumo(Usuario *usuarios, Midia *midias, MidiaConsumida *consumidas, char tipo, const char *caminho_saida) {
  Modelo modelo;
  const char *titulo = "CONSUMO DE MIDIAS POR USUARIO";
  if (tipo == 'F')
    titulo = "CONSUMO DE FILMES POR USUARIO";
  else if (tipo == 'S')
    titulo = "CONSUMO DE SERIES POR USUARIO";

  iniciar_modelo(&modelo, titulo, "Usuario;Midia;Favorito;Avaliacao:D");
  definir_secao(&modelo, 0); // padrao: uma secao por usuario
  configurar_modelo(&modelo);

  for (MidiaConsumida *c = consumidas; c != NULL; c = c->prox) {
    Usuario *u = buscar_usuario(usuarios, c->id_usuario);
    Midia *m = buscar_midia(midias, c->id_midia);
    if (tipo != 0 && (m == NULL || m->tipo != tipo))
      continue; // midia de outro tipo
    adicionar_linha(&modelo, "%s;%s;%s;%.1f", u != NULL ? u->nome : "Desconhecido", m != NULL ? m->nome : "Desconhecida", c->favorito ? "Sim" : "Nao", c->avaliacao_usuario);
  }
  gravar_relatorio(&modelo, caminho_saida);
}

// percorre os usuarios (e nao as assinaturas): se dois usuarios usam a mesma
// assinatura, ela entra duas vezes no faturamento
void gerarRelatorioFaturamentoPagamento(Usuario *usuarios, Assinatura *assinaturas, const char *caminho_saida) {
  Modelo modelo;
  iniciar_modelo(&modelo, "FATURAMENTO POR METODO DE PAGAMENTO", "Metodo;Usuario;Plano;Valor:M+");
  definir_secao(&modelo, 0); // padrao: uma secao por metodo, com a soma de cada uma
  configurar_modelo(&modelo);

  for (Usuario *u = usuarios; u != NULL; u = u->prox) {
    Assinatura *a = buscar_assinatura(assinaturas, u->id_assinatura);
    if (a != NULL)
      adicionar_linha(&modelo, "%s;%s;%s;%.2f", a->metodo_pagamento, u->nome, a->tipo, a->valor);
  }
  gravar_relatorio(&modelo, caminho_saida);
}
