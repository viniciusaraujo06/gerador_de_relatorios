#include "interface.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// LEITURA DO TECLADO
// em todas as perguntas, Enter mantem o valor padrao mostrado entre colchetes
//

// devolve 0 quando a entrada acabou (Ctrl+Z no Windows, Ctrl+D no Linux)
static int ler_linha(char *destino, int tamanho) {
  if (fgets(destino, tamanho, stdin) == NULL) {
    destino[0] = '\0';
    return 0;
  }
  if (strchr(destino, '\n') == NULL) { // linha maior que o buffer: descarta o resto
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
      ;
  }
  destino[strcspn(destino, "\r\n")] = '\0';
  return 1;
}

static int ler_numero(const char *mensagem, int padrao, int minimo, int maximo) {
  char buffer[64];
  int valor;
  while (1) {
    printf("%s [%d]: ", mensagem, padrao);
    if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
      return padrao;
    if (sscanf(buffer, "%d", &valor) == 1 && valor >= minimo && valor <= maximo)
      return valor;
    printf("Digite um numero de %d a %d.\n", minimo, maximo);
  }
}

// aceita so uma das letras de 'validas' (maiuscula ou minuscula)
static char ler_letra(const char *mensagem, const char *validas, char padrao) {
  char buffer[16];
  while (1) {
    printf("%s [%c]: ", mensagem, padrao);
    if (!ler_linha(buffer, sizeof(buffer)) || buffer[0] == '\0')
      return padrao;
    char letra = (char)toupper((unsigned char)buffer[0]);
    if (strchr(validas, letra) != NULL)
      return letra;
    printf("Use uma destas letras: %s\n", validas);
  }
}

static int sim(const char *mensagem, int padrao) {
  return ler_letra(mensagem, "SN", padrao ? 'S' : 'N') == 'S';
}

static void ler_texto(const char *mensagem, char *valor, int tamanho) {
  char buffer[TAM_VALOR];
  printf("%s [%s]: ", mensagem, valor);
  if (ler_linha(buffer, sizeof(buffer)) && buffer[0] != '\0')
    snprintf(valor, (size_t)tamanho, "%s", buffer);
}

// le varios numeros na mesma linha ("3 1 2" ou "3,1,2"); devolve quantos leu
static int ler_numeros(const char *mensagem, int numeros[], int max) {
  char buffer[128], *p = buffer, *fim;
  int qtd = 0;

  printf("%s: ", mensagem);
  if (!ler_linha(buffer, sizeof(buffer)))
    return 0;
  while (*p != '\0' && qtd < max) {
    long n = strtol(p, &fim, 10);
    if (fim == p) {
      p++; // nao e numero (espaco, virgula...): pula
    } else {
      numeros[qtd++] = (int)n;
      p = fim;
    }
  }
  return qtd;
}

int menu_principal(void) {
  printf("\n==============================================\n");
  printf("   GERADOR DE RELATORIOS - STREAMING\n");
  printf("==============================================\n");
  printf(" 1 - Relatorio de Usuarios e Assinaturas\n");
  printf(" 2 - Catalogo de Midias\n");
  printf(" 3 - Catalogo de Series\n");
  printf(" 4 - Catalogo de Filmes\n");
  printf(" 5 - Consumo de Midias por Usuario\n");
  printf(" 6 - Consumo de Series por Usuario\n");
  printf(" 7 - Consumo de Filmes por Usuario\n");
  printf(" 8 - Faturamento por Metodo de Pagamento\n");
  printf(" 9 - Sair\n");
  printf("==============================================\n");
  return ler_numero("Escolha uma opcao", 9, 1, 9);
}

//
// CONFIGURACAO DO MODELO
//

static void listar_campos(Modelo *modelo) {
  for (Campo *c = modelo->campos; c != NULL; c = c->prox)
    printf("  %2d - %s\n", c->coluna + 1, c->nome);
}

void configurar_modelo(Modelo *modelo) {
  int numeros[MAX_CAMPOS], qtd, i;
  Campo *c;

  printf("\n--- %s ---\n", modelo->titulo);
  if (sim("Usar o modelo padrao? (S/N)", 1))
    return;

  printf("\n[Area 1 - CABECALHO]\n");
  ler_texto("Titulo do relatorio", modelo->titulo, TAM_VALOR);
  modelo->mostrar_data = sim("Mostrar data de geracao? (S/N)", modelo->mostrar_data);

  printf("\n[Area 2 - CORPO]\nCampos disponiveis:\n");
  listar_campos(modelo);
  qtd = ler_numeros("Campos do relatorio, na ordem desejada (Enter = todos)", numeros, MAX_CAMPOS);
  escolher_campos(modelo, numeros, qtd);

  if (sim("Mudar nome, tipo e formato dos campos? (S/N)", 0))
    for (c = modelo->campos; c != NULL; c = c->prox) {
      printf("Campo %d\n", c->coluna + 1);
      ler_texto("  Nome", c->nome, TAM_NOME);
      c->tipo = ler_letra("  Tipo (T = texto, I = inteiro, D = decimal, M = moeda)", "TIDM", c->tipo);
      if (c->tipo == DECIMAL || c->tipo == MOEDA)
        c->casas = ler_numero("  Formato: casas decimais", c->casas, 0, 6);
    }

  listar_campos(modelo);
  definir_secao(modelo, ler_numero("Dividir em secoes pelo campo (0 = nao dividir)", modelo->coluna_secao + 1, 0, modelo->qtd_campos) - 1);

  printf("\n[Area 3 - RODAPE]\n");
  modelo->mostrar_total = sim("Mostrar total de registros? (S/N)", modelo->mostrar_total);
  qtd = ler_numeros("Somar quais campos numericos? (Enter = manter, 0 = nenhum)", numeros, MAX_CAMPOS);
  if (qtd > 0) {
    for (c = modelo->campos; c != NULL; c = c->prox)
      c->somar = 0;
    for (i = 0; i < qtd; i++) {
      c = buscar_campo(modelo, numeros[i] - 1);
      if (c != NULL)
        c->somar = 1;
    }
  }
}
