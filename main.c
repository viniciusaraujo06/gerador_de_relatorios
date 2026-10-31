#include "interface.h"
#include "leitura.h"
#include "relatorio.h"

// compilar: gcc main.c leitura.c interface.c modelo.c impressao.c relatorio.c -o main
// rodar de dentro desta pasta (para achar input/ e output/): ./main ou .\main.exe

int main(void) {
  Usuario *usuarios = ler_usuarios("input/usuarios.txt");
  Assinatura *assinaturas = ler_assinaturas("input/assinaturas.txt");
  Midia *midias = ler_midias("input/midias.txt");
  Filme *filmes = ler_filmes("input/filmes.txt");
  Serie *series = ler_series("input/series.txt");
  MidiaConsumida *consumidas = ler_midias_consumidas("input/midias_consumidas.txt");

  int opcao = 0;
  while (opcao != 9) {
    opcao = menu_principal();
    switch (opcao) {
    case 1:
      gerarRelatorioUsuariosAssinaturas(usuarios, assinaturas, "output/relatorio-usuarios.txt");
      break;
    case 2:
      gerarRelatorioCatalogoMidias(midias, filmes, series, "output/relatorio-midias.txt");
      break;
    case 3:
      gerarRelatorioCatalogoSeries(midias, series, "output/relatorio-series.txt");
      break;
    case 4:
      gerarRelatorioCatalogoFilmes(midias, filmes, "output/relatorio-filmes.txt");
      break;
    case 5:
      gerarRelatorioConsumo(usuarios, midias, consumidas, 0, "output/relatorio-consumo-midias.txt");
      break;
    case 6:
      gerarRelatorioConsumo(usuarios, midias, consumidas, 'S', "output/relatorio-consumo-series.txt");
      break;
    case 7:
      gerarRelatorioConsumo(usuarios, midias, consumidas, 'F', "output/relatorio-consumo-filmes.txt");
      break;
    case 8:
      gerarRelatorioFaturamentoPagamento(usuarios, assinaturas, "output/relatorio-faturamento.txt");
      break;
    }
  }

  liberar_usuarios(usuarios);
  liberar_assinaturas(assinaturas);
  liberar_midias(midias);
  liberar_filmes(filmes);
  liberar_series(series);
  liberar_midias_consumidas(consumidas);
  return 0;
}
