#ifndef BOARDRATING_H
#define BOARDRATING_H

#include <cuda_runtime_api.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

namespace CudaFunctions {

void board_rating(const bool* cells, const int* highest_cells, int count,
                  int4* board_output);

} // namespace CudaFunctions

#endif // BOARDRATING_H
