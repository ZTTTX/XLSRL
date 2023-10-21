#pragma hls_top
int test_unroll(int x, int y, int z) {
  int ret = 0;
  int C;
  int D;
  C = x + y;
  D = C + z;
  ret = D + 1;
  return ret;
}
// #pragma hls_top
// int test_unroll(int x) {
//   int ret = 0;
//   #pragma hls_unroll yes
//   for(int i=0;i<32;++i) {
//     ret += x * i;
//   }
//   return ret;
// }