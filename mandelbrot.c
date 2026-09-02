#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

#define REAL_MIN -2.0
#define REAL_MAX 1.0
#define IMAG_MIN -1.5
#define IMAG_MAX 1.5

// Estrutura de dados para passar argumentos para as threads do Pthreads
typedef struct {
    int id_thread;
    int num_threads;
    int largura;
    int altura;
    int max_iteracoes;
    int *imagem;
} ThreadData;

int calcular_pixel(double c_real, double c_imag, int max_iteracoes) {
    double z_real = 0.0;
    double z_imag = 0.0;
    int iteracao = 0;

    while (iteracao < max_iteracoes) {
        double z_real_novo = z_real * z_real - z_imag * z_imag + c_real;
        double z_imag_novo = 2.0 * z_real * z_imag + c_imag;

        z_real = z_real_novo;
        z_imag = z_imag_novo;

        iteracao++;

        if (z_real * z_real + z_imag * z_imag > 4.0) {
            break;
        }
    }

    return iteracao;
}

int validar_argumentos(int argc, char *argv[],
                       int *largura, int *altura,
                       int *max_iteracoes, int *num_threads) {
    char *fim;

    if (argc != 5) {
        fprintf(stderr, "Erro: quantidade de argumentos invalida.\n");
        return 0;
    }

    *largura = strtol(argv[1], &fim, 10);
    if (*fim != '\0' || *largura <= 0) {
        fprintf(stderr, "Erro: largura invalida.\n");
        return 0;
    }

    *altura = strtol(argv[2], &fim, 10);
    if (*fim != '\0' || *altura <= 0) {
        fprintf(stderr, "Erro: altura invalida.\n");
        return 0;
    }

    *max_iteracoes = strtol(argv[3], &fim, 10);
    if (*fim != '\0' || *max_iteracoes <= 0) {
        fprintf(stderr, "Erro: numero maximo de iteracoes invalido.\n");
        return 0;
    }

    *num_threads = strtol(argv[4], &fim, 10);
    if (*fim != '\0' || *num_threads <= 0) {
        fprintf(stderr, "Erro: numero de threads invalido.\n");
        return 0;
    }

    return 1;
}

void executar_openmp(int largura, int altura, int max_iteracoes, int num_threads, int *imagem, double *tempo_out) {
    omp_set_num_threads(num_threads);

    double inicio = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < altura; y++) {
        double c_imag = IMAG_MAX - (double)y / altura * (IMAG_MAX - IMAG_MIN);

        for (int x = 0; x < largura; x++) {
            double c_real = REAL_MIN + (double)x / largura * (REAL_MAX - REAL_MIN);

            int iteracoes = calcular_pixel(c_real, c_imag, max_iteracoes);
            imagem[y * largura + x] = (iteracoes * 255) / max_iteracoes;
        }
    }

    double fim = omp_get_wtime();
    *tempo_out = fim - inicio;
}

// =============================================================================
// PTHREADS 1: Divisão por Blocos Contíguos
// =============================================================================
void *worker_pthreads1(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    int linhas_por_thread = data->altura / data->num_threads;
    int inicio_y = data->id_thread * linhas_por_thread;
    int fim_y = (data->id_thread == data->num_threads - 1) ? data->altura : inicio_y + linhas_por_thread;

    for (int y = inicio_y; y < fim_y; y++) {
        double c_imag = IMAG_MAX - (double)y / data->altura * (IMAG_MAX - IMAG_MIN);

        for (int x = 0; x < data->largura; x++) {
            double c_real = REAL_MIN + (double)x / data->largura * (REAL_MAX - REAL_MIN);

            int iteracoes = calcular_pixel(c_real, c_imag, data->max_iteracoes);
            data->imagem[y * data->largura + x] = (iteracoes * 255) / data->max_iteracoes;
        }
    }

    pthread_exit(NULL);
}

int executar_pthreads1(int largura, int altura, int max_iteracoes, int num_threads, int *imagem, double *tempo_out) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *data = malloc(num_threads * sizeof(ThreadData));

    if (!threads || !data) {
        fprintf(stderr, "Erro: falha na alocacao de memoria para pthreads.\n");
        free(threads);
        free(data);
        return 0;
    }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < num_threads; i++) {
        data[i].id_thread = i;
        data[i].num_threads = num_threads;
        data[i].largura = largura;
        data[i].altura = altura;
        data[i].max_iteracoes = max_iteracoes;
        data[i].imagem = imagem;

        if (pthread_create(&threads[i], NULL, worker_pthreads1, &data[i]) != 0) {
            fprintf(stderr, "Erro: falha ao criar thread do Pthreads 1.\n");
            free(threads);
            free(data);
            return 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    *tempo_out = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    free(threads);
    free(data);
    return 1;
}

int salvar_imagem(const char *nome_arquivo, int *imagem, int largura, int altura) {
    FILE *arquivo = fopen(nome_arquivo, "w");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: falha ao criar arquivo de saida %s.\n", nome_arquivo);
        return 0;
    }

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            fprintf(arquivo, "%d", imagem[y * largura + x]);

            if (x < largura - 1) {
                fprintf(arquivo, " ");
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    return 1;
}

int main(int argc, char *argv[]) {
    int largura;
    int altura;
    int max_iteracoes;
    int num_threads;

    if (!validar_argumentos(argc, argv,
                            &largura, &altura,
                            &max_iteracoes, &num_threads)) {
        return EXIT_FAILURE;
    }

    int *imagem = malloc((size_t)largura * altura * sizeof(int));

    if (imagem == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria.\n");
        return EXIT_FAILURE;
    }

    // ==========================================
    // 1. EXECUÇÃO SERIAL
    // ==========================================
    struct timespec inicio_s, fim_s;
    clock_gettime(CLOCK_MONOTONIC, &inicio_s);

    for (int y = 0; y < altura; y++) {
        double c_imag = IMAG_MAX - (double)y / altura * (IMAG_MAX - IMAG_MIN);

        for (int x = 0; x < largura; x++) {
            double c_real = REAL_MIN + (double)x / largura * (REAL_MAX - REAL_MIN);

            int iteracoes = calcular_pixel(c_real, c_imag, max_iteracoes);

            imagem[y * largura + x] = (iteracoes * 255) / max_iteracoes;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &fim_s);
    double tempo_serial = (fim_s.tv_sec - inicio_s.tv_sec) + 
                         (fim_s.tv_nsec - inicio_s.tv_nsec) / 1e9;

    if (!salvar_imagem("mandelbrot_dmsrb_serial.pgm", imagem, largura, altura)) {
        free(imagem);
        return EXIT_FAILURE;
    }

    // ==========================================
    // 2. EXECUÇÃO OPENMP
    // ==========================================
    double tempo_openmp = 0.0;
    executar_openmp(largura, altura, max_iteracoes, num_threads, imagem, &tempo_openmp);

    if (!salvar_imagem("mandelbrot_dmsrb_openmp.pgm", imagem, largura, altura)) {
        free(imagem);
        return EXIT_FAILURE;
    }

    // ==========================================
    // 3. EXECUÇÃO PTHREADS 1 (Blocos Contíguos)
    // ==========================================
    double tempo_pthreads1 = 0.0;
    if (!executar_pthreads1(largura, altura, max_iteracoes, num_threads, imagem, &tempo_pthreads1)) {
        free(imagem);
        return EXIT_FAILURE;
    }

    if (!salvar_imagem("mandelbrot_dmsrb_pthreads1.pgm", imagem, largura, altura)) {
        free(imagem);
        return EXIT_FAILURE;
    }

    // ==========================================
    // REGISTRO DOS TEMPOS
    // ==========================================
    FILE *arquivo_tempo = fopen("times.txt", "w");

    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro: falha ao criar times.txt.\n");
        free(imagem);
        return EXIT_FAILURE;
    }

    fprintf(arquivo_tempo, "Serial: %.6f\n", tempo_serial);
    fprintf(arquivo_tempo, "OpenMP: %.6f\n", tempo_openmp);
    fprintf(arquivo_tempo, "Pthreads 1: %.6f\n", tempo_pthreads1);

    fclose(arquivo_tempo);
    free(imagem);

    return EXIT_SUCCESS;
}