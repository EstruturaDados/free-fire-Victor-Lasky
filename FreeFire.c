


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_COMPONENTES 20
#define NOME_SIZE 30
#define TIPO_SIZE 20

// Representa um componente da torre
typedef struct {
    char nome[NOME_SIZE];
    char tipo[TIPO_SIZE];
    int prioridade; // 1..10 (1 = maior prioridade)
} Componente;

// Tipo para indicar qual ordenacao foi aplicada por ultimo
typedef enum { NAO_ORDENADO, ORDENADO_POR_NOME, ORDENADO_POR_TIPO, ORDENADO_POR_PRIORIDADE } OrdemAtual;

/* ---------- Funções utilitárias ---------- */

// Remove '\n' deixado por fgets
void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

// Faz cópia em minúsculas de uma string em um buffer destino
void to_lower_copy(const char *src, char *dst, size_t dst_size) {
    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

/* ---------- Exibição ---------- */

// Mostra os componentes no vetor formatados
void mostrarComponentes(Componente comps[], int n) {
    printf("\n=== COMPONENTES (%d) ===\n", n);
    if (n == 0) {
        printf("Nenhum componente cadastrado.\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        printf("%2d) Nome: %-28s | Tipo: %-12s | Prioridade: %d\n",
               i + 1, comps[i].nome, comps[i].tipo, comps[i].prioridade);
    }
}

/* ---------- Algoritmos de ordenação (contagem de comparações + tempo) ---------- */

/*
  Bubble sort por nome (string).
  - Ordena strcmp-caso-insensitivo em ordem alfabética.
  - Retorna número de comparações (via ponteiro) e tempo em segundos (via ponteiro).
*/
long bubbleSortPorNome(Componente comps[], int n, double *tempoSegundos) {
    long comparacoes = 0;
    clock_t inicio = clock();

    for (int pass = 0; pass < n - 1; pass++) {
        int trocou = 0;
        for (int j = 0; j < n - 1 - pass; j++) {
            char a[NOME_SIZE], b[NOME_SIZE];
            to_lower_copy(comps[j].nome, a, sizeof(a));
            to_lower_copy(comps[j+1].nome, b, sizeof(b));
            comparacoes++;
            if (strcmp(a, b) > 0) {
                // troca
                Componente tmp = comps[j];
                comps[j] = comps[j+1];
                comps[j+1] = tmp;
                trocou = 1;
            }
        }
        if (!trocou) break; // otimização
    }

    clock_t fim = clock();
    *tempoSegundos = (double)(fim - inicio) / CLOCKS_PER_SEC;
    return comparacoes;
}

/*
  Insertion sort por tipo (string).
  - Ordena por tipo (case-insensitive), e para tipos iguais mantém relativo por nome.
*/
long insertionSortPorTipo(Componente comps[], int n, double *tempoSegundos) {
    long comparacoes = 0;
    clock_t inicio = clock();

    for (int i = 1; i < n; i++) {
        Componente chave = comps[i];
        int j = i - 1;
        char chaveTipo[TIPO_SIZE];
        to_lower_copy(chave.tipo, chaveTipo, sizeof(chaveTipo));

        while (j >= 0) {
            char tipoJ[TIPO_SIZE];
            to_lower_copy(comps[j].tipo, tipoJ, sizeof(tipoJ));
            comparacoes++;
            if (strcmp(tipoJ, chaveTipo) > 0) {
                comps[j + 1] = comps[j];
                j--;
            } else {
                break;
            }
        }
        comps[j + 1] = chave;
    }

    clock_t fim = clock();
    *tempoSegundos = (double)(fim - inicio) / CLOCKS_PER_SEC;
    return comparacoes;
}

/*
  Selection sort por prioridade (int).
  - Ordena em ordem crescente de prioridade (1 .. 10), onde 1 = maior prioridade.
*/
long selectionSortPorPrioridade(Componente comps[], int n, double *tempoSegundos) {
    long comparacoes = 0;
    clock_t inicio = clock();

    for (int i = 0; i < n - 1; i++) {
        int idxMin = i;
        for (int j = i + 1; j < n; j++) {
            comparacoes++;
            if (comps[j].prioridade < comps[idxMin].prioridade) {
                idxMin = j;
            }
        }
        if (idxMin != i) {
            Componente tmp = comps[i];
            comps[i] = comps[idxMin];
            comps[idxMin] = tmp;
        }
    }

    clock_t fim = clock();
    *tempoSegundos = (double)(fim - inicio) / CLOCKS_PER_SEC;
    return comparacoes;
}

/* ---------- Busca binária (só depois de ordenado por nome) ---------- */

/*
  Busca binária case-insensitive por nome.
  - Retorna índice do componente encontrado ou -1 se não encontrado.
  - também retorna o número de comparações via ponteiro comparacoes.
*/
int buscaBinariaPorNome(Componente comps[], int n, const char *nomeBusca, long *comparacoes) {
    *comparacoes = 0;
    int low = 0, high = n - 1;
    char alvo[NOME_SIZE];
    to_lower_copy(nomeBusca, alvo, sizeof(alvo));

    while (low <= high) {
        int mid = (low + high) / 2;
        char midNome[NOME_SIZE];
        to_lower_copy(comps[mid].nome, midNome, sizeof(midNome));
        (*comparacoes)++;
        int cmp = strcmp(midNome, alvo);
        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

/* ---------- Funções de cadastro e remoção ---------- */

void cadastrarComponente(Componente comps[], int *n) {
    if (*n >= MAX_COMPONENTES) {
        printf("\nMochila de componentes cheia (max %d)!\n", MAX_COMPONENTES);
        return;
    }
    Componente novo;
    printf("\nCadastro de componente (%d/%d)\n", *n + 1, MAX_COMPONENTES);

    printf("Nome: ");
    fgets(novo.nome, sizeof(novo.nome), stdin);
    trim_newline(novo.nome);

    printf("Tipo (controle, suporte, propulsao, etc): ");
    fgets(novo.tipo, sizeof(novo.tipo), stdin);
    trim_newline(novo.tipo);

    // ler prioridade com validação
    int p;
    while (1) {
        printf("Prioridade (1..10) (1 = maior prioridade): ");
        if (scanf("%d", &p) != 1) {
            printf("Entrada invalida. Tente novamente.\n");
            while (getchar() != '\n'); // limpa buffer
            continue;
        }
        if (p < 1 || p > 10) {
            printf("Valor fora do intervalo. Tente novamente.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // limpa final de linha
        break;
    }
    novo.prioridade = p;

    comps[*n] = novo;
    (*n)++;
    printf("Componente cadastrado com sucesso!\n");
}

/*
  Remove componente por nome (primeira ocorrência).
*/
void removerComponentePorNome(Componente comps[], int *n) {
    if (*n == 0) {
        printf("\nNenhum componente para remover.\n");
        return;
    }
    char nomeBusca[NOME_SIZE];
    printf("\nDigite o nome do componente a remover: ");
    fgets(nomeBusca, sizeof(nomeBusca), stdin);
    trim_newline(nomeBusca);

    // busca sequencial para encontrar índice
    int idx = -1;
    char alvoLower[NOME_SIZE];
    to_lower_copy(nomeBusca, alvoLower, sizeof(alvoLower));
    for (int i = 0; i < *n; i++) {
        char compNomeLower[NOME_SIZE];
        to_lower_copy(comps[i].nome, compNomeLower, sizeof(compNomeLower));
        if (strcmp(compNomeLower, alvoLower) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Componente '%s' nao encontrado.\n", nomeBusca);
        return;
    }

    // shift left para remover
    for (int j = idx; j < (*n) - 1; j++) {
        comps[j] = comps[j + 1];
    }
    (*n)--;
    printf("Componente '%s' removido com sucesso.\n", nomeBusca);
}

/* ---------- Programa principal (menu) ---------- */

int main() {
    Componente componentes[MAX_COMPONENTES];
    int total = 0;
    OrdemAtual ordemAtual = NAO_ORDENADO;
    int opcao;

    printf("=== Módulo: Montagem da Torre de Resgate ===\n");
    printf("Objetivo: organize componentes, escolha algoritmos e encontre o componente-chave.\n");

    do {
        printf("\n--- MENU ---\n");
        printf("1) Cadastrar componente\n");
        printf("2) Remover componente por nome\n");
        printf("3) Listar componentes\n");
        printf("4) Ordenar componentes (escolher algoritmo)\n");
        printf("5) Buscar componente-chave por nome (busca binaria - exige ordenacao por nome)\n");
        printf("0) Sair\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida. Tente novamente.\n");
            while (getchar() != '\n'); // limpa
            continue;
        }
        while (getchar() != '\n'); // limpa resto da linha

        switch (opcao) {
            case 1:
                cadastrarComponente(componentes, &total);
                mostrarComponentes(componentes, total);
                ordemAtual = NAO_ORDENADO;
                break;

            case 2:
                removerComponentePorNome(componentes, &total);
                mostrarComponentes(componentes, total);
                ordemAtual = NAO_ORDENADO;
                break;

            case 3:
                mostrarComponentes(componentes, total);
                break;

            case 4: {
                if (total == 0) {
                    printf("\nNao ha componentes para ordenar.\n");
                    break;
                }
                printf("\nEscolha o algoritmo de ordenacao:\n");
                printf("1) Bubble sort por NOME (string)  [recomendado para busca binaria]\n");
                printf("2) Insertion sort por TIPO (string)\n");
                printf("3) Selection sort por PRIORIDADE (int)  (1 = maior prioridade)\n");
                printf("Escolha: ");
                int escolha;
                if (scanf("%d", &escolha) != 1) {
                    printf("Entrada invalida.\n");
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');

                double tempo = 0.0;
                long comps = 0;
                if (escolha == 1) {
                    comps = bubbleSortPorNome(componentes, total, &tempo);
                    ordemAtual = ORDENADO_POR_NOME;
                    printf("\nBubble sort por NOME concluido.\n");
                } else if (escolha == 2) {
                    comps = insertionSortPorTipo(componentes, total, &tempo);
                    ordemAtual = ORDENADO_POR_TIPO;
                    printf("\nInsertion sort por TIPO concluido.\n");
                } else if (escolha == 3) {
                    comps = selectionSortPorPrioridade(componentes, total, &tempo);
                    ordemAtual = ORDENADO_POR_PRIORIDADE;
                    printf("\nSelection sort por PRIORIDADE concluido.\n");
                } else {
                    printf("Opcao invalida.\n");
                    break;
                }

                printf("Comparacoes realizadas: %ld\n", comps);
                printf("Tempo de execucao: %.6f segundos\n", tempo);
                mostrarComponentes(componentes, total);
                break;
            }

            case 5: {
                if (total == 0) {
                    printf("\nNao ha componentes para buscar.\n");
                    break;
                }
                if (ordemAtual != ORDENADO_POR_NOME) {
                    printf("\nA busca binaria exige que os componentes estejam ordenados por NOME.\n");
                    printf("Ordene por NOME (bubble sort) antes de usar a busca binaria.\n");
                    break;
                }
                char chave[NOME_SIZE];
                printf("\nDigite o NOME do componente-chave a procurar: ");
                fgets(chave, sizeof(chave), stdin);
                trim_newline(chave);

                long comparacoesBusca = 0;
                int idx = buscaBinariaPorNome(componentes, total, chave, &comparacoesBusca);
                if (idx != -1) {
                    printf("\n--- COMPONENTE-CHAVE ENCONTRADO ---\n");
                    printf("Nome: %s\nTipo: %s\nPrioridade: %d\n", componentes[idx].nome, componentes[idx].tipo, componentes[idx].prioridade);
                    printf("Comparacoes na busca binaria: %ld\n", comparacoesBusca);
                    printf("=> Confirmado: componente presente. Pode ser usado para ativar a torre!\n");
                } else {
                    printf("\nComponente '%s' NAO encontrado. Comparacoes: %ld\n", chave, comparacoesBusca);
                }
                break;
            }

            case 0:
                printf("Encerrando modulo. Boa sorte na fuga!\n");
                break;

            default:
                printf("Opcao invalida. Tente novamente.\n");
        }

    } while (opcao != 0);

    return 0;
}
