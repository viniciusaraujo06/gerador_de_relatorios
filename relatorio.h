#ifndef RELATORIO_H
#define RELATORIO_H

#include "entidades.h"

// os relatorios disponiveis; todos gravam em caminho_saida

void gerarRelatorioUsuariosAssinaturas(Usuario *usuarios, Assinatura *assinaturas,
                                       const char *caminho_saida);

void gerarRelatorioCatalogoMidias(Midia *midias, Filme *filmes, Serie *series,
                                  const char *caminho_saida);

void gerarRelatorioCatalogoSeries(Midia *midias, Serie *series,
                                  const char *caminho_saida);

void gerarRelatorioCatalogoFilmes(Midia *midias, Filme *filmes,
                                  const char *caminho_saida);

// tipo: 'F' so filmes, 'S' so series, 0 = todas as midias
void gerarRelatorioConsumo(Usuario *usuarios, Midia *midias,
                           MidiaConsumida *consumidas, char tipo,
                           const char *caminho_saida);

void gerarRelatorioFaturamentoPagamento(Usuario *usuarios, Assinatura *assinaturas,
                                        const char *caminho_saida);

#endif
