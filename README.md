# Implementação 2 — Conjunto de Mandelbrot

Implementação do conjunto de Mandelbrot em C, desenvolvida para a disciplina de Infraestrutura de Software.

O projeto possui quatro implementações do cálculo:

- **Serial**
- **OpenMP**
- **Pthreads 1**
- **Pthreads 2**

As implementações paralelas têm como objetivo explorar diferentes formas de execução e divisão do trabalho, mantendo o mesmo resultado final.

## Objetivo

Implementar um programa capaz de gerar uma imagem do conjunto de Mandelbrot e comparar o tempo de execução entre uma implementação serial e diferentes abordagens de paralelismo.

O programa deve gerar a imagem para a região:

- Eixo real: `[-2.0, 1.0]`
- Eixo imaginário: `[-1.5, 1.5]`

A intensidade de cada pixel é determinada proporcionalmente ao número de iterações realizadas, com valores normalizados entre `0` e `255`.

## Tecnologias

- C
- OpenMP
- POSIX Threads (Pthreads)
- Make
- WSL

./mandelbrot [largura] [altura] [max_iteracoes] [num_threads]
