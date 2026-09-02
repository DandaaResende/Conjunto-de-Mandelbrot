CC = gcc
CFLAGS = -Wall -Wextra -O3 -fopenmp -pthread
TARGET = mandelbrot
SRC = mandelbrot.c

ARGS = 10 6 40 4

.PHONY: all run clean diff

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

diff: run
	@echo "Comparando resultados das imagens..."
	diff -q mandelbrot_dmsrb_serial.pgm mandelbrot_dmsrb_openmp.pgm
	diff -q mandelbrot_dmsrb_serial.pgm mandelbrot_dmsrb_pthreads1.pgm
	diff -q mandelbrot_dmsrb_serial.pgm mandelbrot_dmsrb_pthreads2.pgm
	@echo "Sucesso: Todas as imagens sao 100% identicas!"

clean:
	rm -f $(TARGET) *.pgm *.txt