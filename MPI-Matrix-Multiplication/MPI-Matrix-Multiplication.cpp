#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

void fillMatrixAndVector(std::vector<std::vector<int>>& matrix, std::vector<int>& vector) {
    // Заполнение матрицы и вектора единицами (в данном случае, случайными значениями)
    // Замените этот код вашей логикой по заполнению
    srand(time(nullptr));
    for (auto& row : matrix) {
        for (int& element : row) {
            element = rand() % 10;  // Произвольный диапазон для примера
        }
    }

    for (int& element : vector) {
        element = rand() % 10;  // Произвольный диапазон для примера
    }
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int element : row) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
}

void printVector(const std::vector<int>& vector) {
    for (int element : vector) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int matrixSize = 4;  // Замените это значение на ваш размер матрицы

    // Процесс с рангом 0 заполняет матрицу и вектор
    std::vector<std::vector<int>> matrix;
    std::vector<int> vector(matrixSize);

    if (rank == 0) {
        matrix.resize(matrixSize, std::vector<int>(matrixSize));
        fillMatrixAndVector(matrix, vector);
    }

    // Рассылка матрицы и вектора с использованием коллективных взаимодействий
    MPI_Bcast(vector.data(), matrixSize, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Рассылка строк матрицы
        for (int i = 0; i < matrixSize; ++i) {
            MPI_Bcast(matrix[i].data(), matrixSize, MPI_INT, 0, MPI_COMM_WORLD);
        }
    }

    // Каждый процесс выполняет умножение своих строк на вектор
    std::vector<int> localResult(matrixSize, 0);
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            localResult[i] += matrix[i][j] * vector[j];
        }
    }

    // Сбор результатов с использованием декартовой топологии и коммуникатора строки решетки
    std::vector<int> globalResult(matrixSize, 0);
    MPI_Reduce(localResult.data(), globalResult.data(), matrixSize, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Вывод результатов на экран (может быть отключено для больших данных)
    if (rank == 0) {
        std::cout << "Matrix:" << std::endl;
        printMatrix(matrix);

        std::cout << "Vector:" << std::endl;
        printVector(vector);

        std::cout << "Result:" << std::endl;
        printVector(globalResult);
    }

    MPI_Finalize();

    return 0;
}
