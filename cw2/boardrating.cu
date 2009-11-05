#include "boardrating.h"

#include <iostream>

using std::cerr;
using std::endl;

namespace CudaFunctions {

__global__ void board_rating_kernel(const bool* cells_, const int* highest_cells_,
                                    int4* output_) {
  const int x = threadIdx.x;
  const int board_id = blockIdx.x;
  
  // Get pointers to this thread's parts of the buffers
  const bool* cells = cells_ + board_id * BOARD_WIDTH * BOARD_HEIGHT + x;
  int highest_cell = *(highest_cells_ + board_id * BOARD_WIDTH + x);
  int4* const output = output_ + board_id * BOARD_WIDTH + x;
  
  // Shared memory used to store neighbouring highest_cells values
  __shared__ int shared_highest_cells[BOARD_WIDTH];
  
  // Fill our part of the shared memory
  shared_highest_cells[x] = highest_cell;
  __syncthreads();
  
  // Special cases for the well depth
  int well_depth;
  if (x == 0) {
    well_depth = highest_cell - shared_highest_cells[1];
  } else if (x == BOARD_WIDTH - 1) {
    well_depth = highest_cell - shared_highest_cells[BOARD_WIDTH - 1];
  } else {
    well_depth = highest_cell - max(shared_highest_cells[x-1], shared_highest_cells[x+1]);
  }
  
  // Initialise stuff
  int holes = -highest_cell;
  int connected_holes = 0;

  // Look at each cell down the board
  bool last_cell = false;
  for (int y=0 ; y<BOARD_HEIGHT ; ++y) {
    bool cell = *(cells + y*BOARD_WIDTH);

    holes += !cell;
    connected_holes += last_cell && !cell;
    
    last_cell = cell;
  }

  // Fill the output buffer
  *output = make_int4(holes, connected_holes, well_depth, 0);
}

__global__ void reduce_kernel(const int4* column_outputs_, const int* highest_cells_,
                              int4* rating_outputs_) {
  const int board_id = blockIdx.x;
  
  // Get pointers to this thread's parts of the buffers
  const int4* column_outputs = column_outputs_ + board_id * BOARD_WIDTH;
  const int* highest_cells = highest_cells_ + board_id * BOARD_WIDTH;
  int4* rating_output = rating_outputs_ + board_id;
  
  int holes = 0;
  int connected_holes = 0;
  int max_well_depth = 0;
  int pile_height = BOARD_HEIGHT;
  int max_pile_height = 0;
  
  for (int x=0 ; x<BOARD_WIDTH ; ++x) {
    const int4 column_output = column_outputs[x];
    const int highest_cell = highest_cells[x];
    
    holes += column_output.x;
    connected_holes += column_output.y;
    max_well_depth += max(column_output.z, 0);
    
    pile_height = min(pile_height, highest_cell);
    max_pile_height = max(max_pile_height, highest_cell);
  }
  
  *rating_output = make_int4(holes, connected_holes, max_well_depth, pile_height);
}

void handle_error(cudaError_t e) {
  if (e == cudaSuccess)
    return;

  cerr << "CUDA error: " << cudaGetErrorString(e) << endl;
  abort();
}

void board_rating(const bool* cells, const int* highest_cells, int count,
                  int4* board_output) {
  // First pass - compute ratings for each column
  bool* d_cells;
  int* d_highest_cells;
  int4* d_column_output;
  int4* d_board_output;
  
  const int cells_size         = count*BOARD_WIDTH*BOARD_HEIGHT * sizeof(bool);
  const int highest_cells_size = count*BOARD_WIDTH * sizeof(int);
  const int column_output_size = count*BOARD_WIDTH * sizeof(int4);
  const int board_output_size  = count * sizeof(int4);
  
  handle_error(cudaMalloc((void**)&d_cells, cells_size));
  handle_error(cudaMalloc((void**)&d_highest_cells, highest_cells_size));
  handle_error(cudaMalloc((void**)&d_column_output, column_output_size));
  
  handle_error(cudaMemcpy(d_cells, cells, cells_size, cudaMemcpyHostToDevice));
  handle_error(cudaMemcpy(d_highest_cells, highest_cells, highest_cells_size, cudaMemcpyHostToDevice));
  
  int threads_per_block = BOARD_WIDTH;
  int blocks = count;
  board_rating_kernel<<<blocks, threads_per_block>>>(d_cells, d_highest_cells, d_column_output);
  
  // Second pass - reduce per-column scores to per-board scores
  handle_error(cudaMalloc((void**)&d_board_output, board_output_size));
  
  threads_per_block = 1;
  reduce_kernel<<<blocks, threads_per_block>>>(d_column_output, d_highest_cells, d_board_output);

  cudaMemcpy(board_output, d_board_output, board_output_size, cudaMemcpyDeviceToHost);
  
  handle_error(cudaFree(d_cells));
  handle_error(cudaFree(d_highest_cells));
  handle_error(cudaFree(d_column_output));
  handle_error(cudaFree(d_board_output));
}

} //namespace CudaFunctions
