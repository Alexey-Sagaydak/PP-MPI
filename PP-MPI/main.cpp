#include <iostream>
#include <cmath>
#include <mpi.h>

/*
��� �� ���:
����� ����� ������� ������� ��������� ������� ������� �������� � ������� send,� �������� ��������
�� �� � ������� receive. � ����� ������� ������� ������� �� ���������� � ������� reduce
*/

/*
���������� �� �������:
1) ������� ��������� ������ � ���������� � ����������� ������.
2) ��������� ������� mpiexec -np X ./MPI-Integration.exe
   X - �������� ����� ���������
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

    // �������� ������ ������ ���������
    for (int i = 0; i < size; ++i) {
        if (i != rank) {
            MPI_Send(&a, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(&b, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        }
    }

    // �������� ��������� ����� �������� ��������
    if (rank != 0) {
        MPI_Send(&localSum, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }

    return localSum;  // ��������, �������� �� ��������, ���������� ��������� �����
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

    // ������� ������� �������� ��������� ����� � ������� �� ������ ���������
    if (rank == 0) {
        for (int i = 1; i < size; ++i) {
            MPI_Recv(&localSum, 1, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            totalSum += localSum;
        }
    }
    else {
        // ��������, �������� �� ��������, �������� ������� � ����������� ���� �����������
        MPI_Recv(&a, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&b, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        localSum = parallelRectangleIntegration(a, b, n, rank, size);
    }

    // ������������� MPI_Reduce ��� ��������� �������������� ����������
    MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double endTime = MPI_Wtime();

    if (rank == 0) {
        std::cout << "Integral value: " << totalSum << std::endl;
        std::cout << "Total time: " << (endTime - startTime) * 1000 << " ms" << std::endl;
    }

    MPI_Finalize();

    return 0;
}

