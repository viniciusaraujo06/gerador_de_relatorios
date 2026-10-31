#ifndef IMPRESSAO_H
#define IMPRESSAO_H

#include "modelo.h"

// desenha o relatorio no TXT com '+', '-' e '|': cabecalho, secoes (tabelas) e rodape
void gravar_relatorio(Modelo *modelo, const char *caminho);

#endif
