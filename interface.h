#ifndef INTERFACE_H
#define INTERFACE_H

#include "modelo.h"

int menu_principal(void);

// pergunta ao usuario o que vai em cada area do modelo (cabecalho, corpo, rodape)
void configurar_modelo(Modelo *modelo);

#endif
