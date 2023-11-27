#include <iostream>
#include <cmath>
#include <mpi.h>

/*
ИНСТРУКЦИЯ ПО ЗАПУСКУ:
1) Открыть командную строку в директории с исполняемым файлом.
2) Выполнить команду mpiexec -np X ./MPI-Integration.exe
   X - желаемое число процессов
*/

double func(double x) {
    return log(x);
}

// Параллельный метод средних прямоугольников
double parallelRectangleIntegration(double a, double b, int n, int rank, int size) {
    double h = (b - a) / n;  // Шаг интегрирования
    double localSum = 0.0;

    // Локальное вычисление частичной суммы
    for (int i = rank + 1; i <= n; i += size) {
        double x = a + (i - 0.5) * h;
        localSum += func(x);
    }

    localSum *= h;  // Умножаем на шаг интегрирования

    // Обмен границами и сборка результатов
    if (rank == 0) {
        double totalSum = localSum;

        // Отправка локальной суммы другим процессам
        for (int i = 1; i < size; ++i) {
            MPI_Recv(&localSum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            totalSum += localSum;
        }

        return totalSum;
    }
    else {
        // Отправка локальной суммы нулевому процессу
        MPI_Send(&localSum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        return 0.0;  // Процессы, отличные от нулевого, возвращают фиктивное значение
    }
}

int main() {
    MPI_Init(NULL, NULL);

    // Number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Number of current process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Processor name
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("**********\nSent from process %d running on processor %s.\n\
        Number of processes is %d.\n\
        ***********************\n",
        rank, processor_name, size);

    double a = 1.0;  // Начальная граница интервала
    double b = 10000.0;  // Конечная граница интервала
    int n = 100000000; // Количество прямоугольников
    
    double startTime = MPI_Wtime();

    // Вызов параллельного метода средних прямоугольников
    double result = parallelRectangleIntegration(a, b, n, rank, size);

    double endTime = MPI_Wtime();

    if (rank == 0) {
        std::cout << "Integral value: " << result << std::endl;
        std::cout << "Total time: " << (endTime - startTime) * 1000 << " мс" << std::endl;
    }

    MPI_Finalize();

    return 0;
}
