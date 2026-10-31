# Gerador de relatórios - streaming

Lê 6 arquivos de texto (`input/`), monta listas encadeadas e grava relatórios em TXT (`output/`),
com o modelo (cabeçalho, corpo e rodapé) escolhido pelo usuário.

## Compilar e rodar

```
gcc -Wall -Wextra main.c leitura.c interface.c modelo.c impressao.c relatorio.c -o main
./main          (Linux)
.\main.exe      (Windows)
```

Rode de dentro desta pasta, para o programa achar `input/` e `output/`.

## Módulos

| Arquivo | O que faz |
|---|---|
| `entidades.h` | uma struct por tabela (usuário, assinatura, mídia, filme, série, mídia consumida) |
| `leitura.c` | lê cada arquivo (campos separados por `;` ou tab) e monta a lista; libera as listas |
| `interface.c` | menu e perguntas do modelo: título, data, campos (nome, tipo, formato), seções, rodapé |
| `modelo.c` | listas do relatório (campos, seções, linhas), formato dos valores e resumos (contagem e somas); o desenho em `modelo.h` mostra o que é cada parte |
| `impressao.c` | desenha o TXT com `+`, `-` e `\|`: tabelas (não sabem o que é seção) e caixas de texto (cabeçalho, título e resumo de seção, rodapé) |
| `relatorio.c` | os 8 relatórios; cada um define os campos padrão e adiciona uma linha por registro |
| `main.c` | lê os arquivos, mostra o menu e libera a memória no fim |

## Modelo de relatório

- **Área 1 - cabeçalho:** título e data de geração.
- **Área 2 - corpo:** quais campos e em que ordem; nome, tipo (texto, inteiro, decimal, moeda)
  e formato (casas decimais) de cada um; campo que divide o corpo em seções.
- **Área 3 - rodapé:** total de registros e soma dos campos numéricos escolhidos
  (a soma também aparece no fim de cada seção).
