/*
template<class T> T add( T x, T y )
{
  return x + y;
}
*/

__kernel void count_calories( __global int* values, __global int* indices, __global int* partial_sums)
{
  int index = get_global_id(0);
  partial_sums[index] = indices[index] + values[index];
}