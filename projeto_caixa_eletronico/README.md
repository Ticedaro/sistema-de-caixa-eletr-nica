# Sistema de Caixa Eletrônico

Projeto em linguagem C para praticar `struct`, `enum`, funções, ponteiros
e arquivos binários.

## Funcionalidades

- Cadastro e consulta de contas.
- Depósitos, saques e transferências.
- Listagem de contas.
- Encerramento lógico de conta.
- Salvamento dos dados em `contas.dat`.

## Compilar

```bash
gcc sistema_caixa.c -o sistema_caixa
```

No Windows, execute `sistema_caixa.exe`. No Linux, execute `./sistema_caixa`.

O arquivo `contas.dat` é criado automaticamente e mantém os dados entre
as execuções do programa.

Os desafios adicionais de histórico, extrato, reativação e exclusão física
não foram incluídos nesta versão.
