#include <iostream>
#include <cmath>
#include <mpi.h>

/*
Что не так:
Нужно чтобы нулевой процесс отправлял границы каждому процессу с помощью send,а процессы собирали
бы их с помощью receive. В конце нулевой процесс собирал бы результаты с помощью reduce
*/

/*
ИНСТРУКЦИЯ ПО ЗАПУСКУ:
1) Открыть командную строку в директории с исполняемым файлом.
2) Выполнить команду mpiexec -np X ./MPI-Integration.exe
   X - желаемое число процессов
*/

double func(double x) {
    return log(x);
}

double integrateSubInterval(double a, double b, int n) {
    double h = (b - a) / n;
    double localSum = 0.0;

    for (int i = 1; i <= n; ++i) {
        double x = a + (i - 0.5) * h;
        localSum += func(x);
    }

    localSum *= h;

    return localSum;
}

double parallelRectangleIntegration(double a, double b, int n, int rank, int size) {
    double h = (b - a) / n;
    double localSum = 0.0;

    for (int i = rank + 1; i <= n; i += size) {
        double x = a + (i - 0.5) * h;
        localSum += func(x);
    }

    localSum *= h;

    // Отправка границ другим процессам
    for (int i = 0; i < size; ++i) {
        if (i != rank) {
            MPI_Send(&a, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(&b, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        }
    }

    // Отправка локальной суммы нулевому процессу
    if (rank != 0) {
        MPI_Send(&localSum, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }

    return localSum;  // Процессы, отличные от нулевого, возвращают локальную сумму
}

int main() {
    MPI_Init(NULL, NULL);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("**********\nSent from process %d running on processor %s.\n\
        Number of processes is %d.\n\
        ***********************\n",
        rank, processor_name, size);

    double a = 1, b = 500;
    int n = 100000000;
    double localSum = 0.0;
    double totalSum = 0.0;

    double startTime = MPI_Wtime();

    // Нулевой процесс получает локальную сумму и границы от других процессов
    if (rank == 0) {
        for (int i = 1; i < size; ++i) {
            MPI_Recv(&localSum, 1, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            totalSum += localSum;
        }
    }
    else {
        // Процессы, отличные от нулевого, получают границы и интегрируют свой подинтервал
        MPI_Recv(&a, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&b, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        localSum = parallelRectangleIntegration(a, b, n, rank, size);
    }

    // Использование MPI_Reduce для получения окончательного результата
    MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double endTime = MPI_Wtime();

    if (rank == 0) {
        std::cout << "Integral value: " << totalSum << std::endl;
        std::cout << "Total time: " << (endTime - startTime) * 1000 << " ms" << std::endl;
    }

    MPI_Finalize();

    return 0;
}

