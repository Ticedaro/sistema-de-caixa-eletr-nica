#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

typedef enum {
    CONTA_ENCERRADA,
    CONTA_ATIVA
} SituacaoConta;

typedef struct {
    long numero;
    char titular[100];
    char cpf[15];
    double saldo;
    SituacaoConta situacao;
} Conta;

FILE *arquivo;

int buscarConta(long numero, Conta *conta, long *posicao) {
    Conta aux;
    rewind(arquivo);

    while (fread(&aux, sizeof(Conta), 1, arquivo) == 1) {
        if (aux.numero == numero) {
            *conta = aux;
            *posicao = ftell(arquivo) - sizeof(Conta);
            return 1;
        }
    }
    return 0;
}

int atualizarConta(Conta conta, long posicao) {
    fseek(arquivo, posicao, SEEK_SET);
    if (fwrite(&conta, sizeof(Conta), 1, arquivo) != 1)
        return 0;
    fflush(arquivo);
    return 1;
}

void cadastrarConta(void) {
    Conta conta, aux;
    long posicao;

    printf("\nNumero da conta: ");
    scanf("%ld", &conta.numero);

    if (conta.numero <= 0) {
        printf("Numero invalido.\n");
        return;
    }

    if (buscarConta(conta.numero, &aux, &posicao)) {
        printf("Esse numero ja existe.\n");
        return;
    }

    printf("Nome do titular: ");
    scanf(" %99[^\n]", conta.titular);
    printf("CPF: ");
    scanf(" %14s", conta.cpf);

    conta.saldo = 0;
    conta.situacao = CONTA_ATIVA;

    fseek(arquivo, 0, SEEK_END);
    if (fwrite(&conta, sizeof(Conta), 1, arquivo) == 1) {
        fflush(arquivo);
        printf("Conta cadastrada!\n");
    } else {
        printf("Erro ao cadastrar conta.\n");
    }
}

void consultarConta(void) {
    Conta conta;
    long numero, posicao;

    printf("\nNumero da conta: ");
    scanf("%ld", &numero);

    if (!buscarConta(numero, &conta, &posicao)) {
        printf("Conta nao encontrada.\n");
        return;
    }

    printf("\nNumero: %ld\n", conta.numero);
    printf("Titular: %s\n", conta.titular);
    printf("CPF: %s\n", conta.cpf);
    printf("Saldo: R$ %.2f\n", conta.saldo);
    printf("Situacao: %s\n",
           conta.situacao == CONTA_ATIVA ? "ATIVA" : "ENCERRADA");
}

int depositar(long numero, double valor) {
    Conta conta;
    long posicao;

    if (valor <= 0) {
        printf("Valor invalido.\n");
        return 0;
    }
    if (!buscarConta(numero, &conta, &posicao)) {
        printf("Conta nao encontrada.\n");
        return 0;
    }
    if (conta.situacao == CONTA_ENCERRADA) {
        printf("Conta encerrada.\n");
        return 0;
    }

    conta.saldo += valor;
    return atualizarConta(conta, posicao);
}

int sacar(long numero, double valor) {
    Conta conta;
    long posicao;

    if (valor <= 0) {
        printf("Valor invalido.\n");
        return 0;
    }
    if (!buscarConta(numero, &conta, &posicao)) {
        printf("Conta nao encontrada.\n");
        return 0;
    }
    if (conta.situacao == CONTA_ENCERRADA) {
        printf("Conta encerrada.\n");
        return 0;
    }
    if (conta.saldo < valor) {
        printf("Saldo insuficiente.\n");
        return 0;
    }

    conta.saldo -= valor;
    return atualizarConta(conta, posicao);
}

int transferir(long origem, long destino, double valor) {
    Conta contaOrigem, contaDestino;
    long posOrigem, posDestino;
    double saldoAntigo;

    if (origem == destino || valor <= 0) {
        printf("Dados da transferencia invalidos.\n");
        return 0;
    }
    if (!buscarConta(origem, &contaOrigem, &posOrigem)) {
        printf("Conta de origem nao encontrada.\n");
        return 0;
    }
    if (!buscarConta(destino, &contaDestino, &posDestino)) {
        printf("Conta de destino nao encontrada.\n");
        return 0;
    }
    if (contaOrigem.situacao != CONTA_ATIVA ||
        contaDestino.situacao != CONTA_ATIVA) {
        printf("As duas contas precisam estar ativas.\n");
        return 0;
    }
    if (contaOrigem.saldo < valor) {
        printf("Saldo insuficiente.\n");
        return 0;
    }

    saldoAntigo = contaOrigem.saldo;
    contaOrigem.saldo -= valor;
    contaDestino.saldo += valor;

    if (!atualizarConta(contaOrigem, posOrigem)) {
        printf("Erro ao atualizar a origem.\n");
        return 0;
    }
    if (!atualizarConta(contaDestino, posDestino)) {
        contaOrigem.saldo = saldoAntigo;
        atualizarConta(contaOrigem, posOrigem);
        printf("Erro ao atualizar o destino.\n");
        return 0;
    }
    return 1;
}

void listarContas(void) {
    Conta conta;
    int quantidade = 0;

    rewind(arquivo);
    printf("\n--- LISTA DE CONTAS ---\n");

    while (fread(&conta, sizeof(Conta), 1, arquivo) == 1) {
        printf("\nNumero: %ld\n", conta.numero);
        printf("Titular: %s\n", conta.titular);
        printf("CPF: %s\n", conta.cpf);
        printf("Saldo: R$ %.2f\n", conta.saldo);
        printf("Situacao: %s\n",
               conta.situacao == CONTA_ATIVA ? "ATIVA" : "ENCERRADA");
        quantidade++;
    }

    if (quantidade == 0)
        printf("Nenhuma conta cadastrada.\n");
}

int encerrarConta(long numero) {
    Conta conta;
    long posicao;

    if (!buscarConta(numero, &conta, &posicao)) {
        printf("Conta nao encontrada.\n");
        return 0;
    }
    if (conta.situacao == CONTA_ENCERRADA) {
        printf("Essa conta ja esta encerrada.\n");
        return 0;
    }
    if (conta.saldo != 0) {
        printf("A conta precisa estar com saldo zero.\n");
        return 0;
    }

    conta.situacao = CONTA_ENCERRADA;
    return atualizarConta(conta, posicao);
}

int main(void) {
    int opcao;
    long numero, origem, destino;
    double valor;

    setlocale(LC_ALL, "");

    arquivo = fopen("contas.dat", "r+b");
    if (arquivo == NULL)
        arquivo = fopen("contas.dat", "w+b");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    do {
        printf("\n===== CAIXA ELETRONICO =====\n");
        printf("1 - Cadastrar conta\n");
        printf("2 - Consultar conta\n");
        printf("3 - Deposito\n");
        printf("4 - Saque\n");
        printf("5 - Transferencia\n");
        printf("6 - Listar contas\n");
        printf("7 - Encerrar conta\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                cadastrarConta();
                break;
            case 2:
                consultarConta();
                break;
            case 3:
                printf("Numero da conta: ");
                scanf("%ld", &numero);
                printf("Valor do deposito: ");
                scanf("%lf", &valor);
                if (depositar(numero, valor))
                    printf("Deposito realizado!\n");
                else
                    printf("Nao foi possivel realizar o deposito.\n");
                break;
            case 4:
                printf("Numero da conta: ");
                scanf("%ld", &numero);
                printf("Valor do saque: ");
                scanf("%lf", &valor);
                if (sacar(numero, valor))
                    printf("Saque realizado!\n");
                else
                    printf("Nao foi possivel realizar o saque.\n");
                break;
            case 5:
                printf("Conta de origem: ");
                scanf("%ld", &origem);
                printf("Conta de destino: ");
                scanf("%ld", &destino);
                printf("Valor da transferencia: ");
                scanf("%lf", &valor);
                if (transferir(origem, destino, valor))
                    printf("Transferencia realizada!\n");
                else
                    printf("Nao foi possivel realizar a transferencia.\n");
                break;
            case 6:
                listarContas();
                break;
            case 7:
                printf("Numero da conta: ");
                scanf("%ld", &numero);
                if (encerrarConta(numero))
                    printf("Conta encerrada!\n");
                else
                    printf("Nao foi possivel encerrar a conta.\n");
                break;
            case 0:
                printf("Saindo do sistema...\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    fclose(arquivo);
    return 0;
}
