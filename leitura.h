#ifndef LEITURA_H
#define LEITURA_H

#include <stddef.h>
#include "entidades.h"

// malloc que avisa quando falta memoria
void *alocar(size_t tamanho);

// devolve o proximo campo da linha (separados por ';' ou <tab>) e avanca o cursor;
// quando a linha acaba devolve "" (campo vazio)
char *proximo_campo(char **cursor);

// cada funcao le um arquivo e devolve o inicio da lista, na ordem do arquivo
Assinatura *ler_assinaturas(const char *caminho);
Usuario *ler_usuarios(const char *caminho);
Midia *ler_midias(const char *caminho);
Filme *ler_filmes(const char *caminho);
Serie *ler_series(const char *caminho);
MidiaConsumida *ler_midias_consumidas(const char *caminho);

void liberar_assinaturas(Assinatura *lista);
void liberar_usuarios(Usuario *lista);
void liberar_midias(Midia *lista);
void liberar_filmes(Filme *lista);
void liberar_series(Serie *lista);
void liberar_midias_consumidas(MidiaConsumida *lista);

#endif
