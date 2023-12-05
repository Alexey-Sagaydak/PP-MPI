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

    // Создание декартовой топологии
    int dims[2] = { size, 1 };
    int periods[2] = { 0, 0 };
    int reorder = 0;
    MPI_Comm cartComm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartComm);

    // Получение координат процесса в решетке
    int coords[2];
    MPI_Cart_coords(cartComm, rank, 2, coords);

    // Процесс с рангом 0 заполняет матрицу и вектор
    std::vector<std::vector<int>> matrix;
    std::vector<int> vector(matrixSize);

    if (rank == 0) {
        matrix.resize(matrixSize, std::vector<int>(matrixSize));
        fillMatrixAndVector(matrix, vector);
    }

    // Рассылка матрицы и вектора с использованием коллективных взаимодействий
    MPI_Bcast(vector.data(), matrixSize, MPI_INT, 0, cartComm);

    if (rank == 0) {
        // Рассылка строк матрицы
        for (int i = 0; i < matrixSize; ++i) {
            MPI_Bcast(matrix[i].data(), matrixSize, MPI_INT, 0, cartComm);
        }
    }

    // Каждый процесс выполняет умножение своих строк на вектор
    std::vector<int> localResult(matrixSize, 0);
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            localResult[i] += matrix[i][j] * vector[j];
        }
    }

    // Сбор результатов с использованием коммуникатора строки решетки
    MPI_Comm rowComm;
    int remain_dims[2] = { 1, 0 };  // Оставляем только строки
    MPI_Cart_sub(cartComm, remain_dims, &rowComm);

    std::vector<int> globalResult(matrixSize, 0);
    MPI_Reduce(localResult.data(), globalResult.data(), matrixSize, MPI_INT, MPI_SUM, 0, rowComm);

    // Вывод результатов на экран (может быть отключено для больших данных)
    if (rank == 0) {
        std::cout << "Matrix:" << std::endl;
        printMatrix(matrix);

        std::cout << "Vector:" << std::endl;
        printVector(vector);

        std::cout << "Result:" << std::endl;
        printVector(globalResult);
    }

    MPI_Comm_free(&cartComm);
    MPI_Comm_free(&rowComm);

    MPI_Finalize();

    return 0;
}
