#include <clc/clc.h>
/*
template<class T> T add( T x, T y )
{
  return x + y;
}
*/

float add(float a, float b) {
    return a + b;
}


__kernel void test( __global float* a, __global float* b)
{
  int index = get_global_id(0);
  a[index] = add(b[index], b[index+1]);
}