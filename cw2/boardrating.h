#ifndef BOARDRATING_H
#define BOARDRATING_H

#include <cuda_runtime_api.h>

namespace CudaFunctions {

template <int W, int H>
void board_rating(const bool* cells, const int* highest_cells, int count,
                  int4* board_output, int* board_output2) { abort(); }

// We have to hide the actual implementation of templated board_rating because
// otherwise we'd need to compile whatever used it (everything) with nvcc.
// nvcc and gcc 3.3 and qt don't play nice with each other on Ubuntu.
// Instead board_rating<W,H> has several specialisations that call
// _board_rating<W,H> to do the actual work.

template <> void board_rating<4,4>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<6,12>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<7,14>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<8,16>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<9,18>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<10,20>(const bool*, const int*, int, int4*, int*);
template <> void board_rating<16,16>(const bool*, const int*, int, int4*, int*);

} // namespace CudaFunctions

#endif // BOARDRATING_H
