# Algoritmo do Banqueiro

Trabalho prático desenvolvido no 5º período do curso de Engenharia de Computação na PUCMinas, na matéria de Sistemas Operacionais.

Este projeto implementa o Algoritmo do Banqueiro utilizando programação multithread com `pthread` em C. Essa tarefa de programação combina três tópicos diferentes: 
1. criar múltiplos threads;
2. prevenir condições de corrida;
3. evitar deadlocks.


## Descrição

O algoritmo garante que uma solicitação de recursos feita por um cliente só será atendida se o sistema permanecer em um **estado seguro** após a alocação. Os clientes solicitam e liberam recursos continuamente, e o banqueiro analisa cada solicitação usando o algoritmo de segurança descrito no livro:

> **SILBERSCHATZ, Abraham; GALVIN, Peter B.; GAGNE, Greg. Fundamentos de Sistemas Operacionais. 9ª Edição.**


## Estrutura de Dados

* `NUMBER_OF_CUSTOMERS` - Numero de clientes (5)
* `NUMBER_OF_RESOURCES` - Numero de recursos (3)
* `availableResources[3]` – Recursos disponíveis no sistema
* `maximumDemandOfCustomer[5][3]` – Demanda máxima de cada cliente
* `currentAllocationOfCustomer[5][3]` – Recursos atualmente alocados a cada cliente
* `remainingCustomerNeed[5][3]` – Recursos restantes que o cliente pode solicitar
* `MAX_TIME` - Tempo máximo de sleep (2000ms)



## Como Executar

#### Requisitos

- GCC com suporte a `pthread`

#### Compilação

No terminal, execute:

```bash
gcc -pthread bankersAlgorithm.c -o bankersAlgorithm
```

#### Execução

Execute passando a quantidade de recursos de cada tipo. Por exemplo:

```bash
./bankersAlgorithm 10 5 7
```

> Isso representa: 10 unidades do recurso 0, 5 do recurso 1 e 7 do recurso 2.


## 📝 Licença

Este projeto é apenas para fins acadêmicos e está licenciado sob a [MIT License](LICENSE).
