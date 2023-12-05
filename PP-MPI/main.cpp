#include <iostream>
#include <cmath>
#include <mpi.h>
#include <iomanip>

// mpiexec -np 3 .\MPI_Integration.exe 1 1000 1000000

float func(float x) {
    return log(x);
}

float rectangleMethod(float* x, int n) {
    float estimate = 0.0;

    for (int i = 0; i < n; i++) {
        float y = x[0] + (i + 0.5) * x[2];
        estimate += func(y);
    }
    estimate = estimate * x[2];

    return estimate;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << " You need to pass 3 parameters: a, b, n\n";
        return 0;
    }

    int my_rank, comm_sz, n = 1024, local_n;
    double a = 0.0, b = 3.0, h, local_a, local_b;
    double local_int, total_int = 0.0, z;
    int source;

    a = atof(argv[1]);
    b = atof(argv[2]);
    z = atof(argv[3]);
    n = (int)z;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    double startTime = MPI_Wtime();

    h = (b - a) / n;
    local_n = n / comm_sz;

    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;

    float x[3];
    x[0] = local_a;
    x[1] = local_b;
    x[2] = h;

    local_int = rectangleMethod(x, local_n);

    if (my_rank != 0) {
        MPI_Send(&local_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else {
        total_int = local_int;
        for (source = 1; source < comm_sz; source++) {
            MPI_Recv(&local_int, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            total_int += local_int;
        }
    }

    if (my_rank == 0) {
        double endTime = MPI_Wtime();
        std::cout << std::endl << "-------------------------------------";
        std::cout << std::endl << " For n=" << n << " Rectangles from a=" << a << " to b=" << b;
        std::cout << std::endl << " The Definite Integral Value is: " << std::fixed << std::setprecision(3) << total_int;
        std::cout << std::endl << "Total time: " << (endTime - startTime) * 1000 << " ms" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
