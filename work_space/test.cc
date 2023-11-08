#define N 2 // Define matrix size, e.g., 8x8

#pragma hls_top
void matrix_multiply(int a[N][N], int b[N][N], int result[N][N]) {
  // Initialize the result matrix
  #pragma hls_unroll yes
  for (int i = 0; i < N; i++) {
    #pragma hls_unroll yes
    for (int j = 0; j < N; j++) {
      result[i][j] = 0;
    }
  }

  // Perform matrix multiplication
  #pragma hls_unroll yes
  for (int i = 0; i < N; i++) {
    #pragma hls_unroll yes
    for (int j = 0; j < N; j++) {
      #pragma hls_unroll yes
      for (int k = 0; k < N; k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}


// int test_add(int a, int b){
//   return a+b;
// }

// #pragma hls_top
// int test_unroll(int x, int y, int z) {
//   int ret = 0;
//   int D = test_add(x, z);
//   int A = test_add(x, y);
//   ret = D + A;
//   return ret;
// }


