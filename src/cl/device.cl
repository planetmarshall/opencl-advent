#define __ptx_mad(a,b,c) ((a)*(b)+(c))

__attribute__((always_inline)) size_t _Z13get_global_idj(unsigned int dimindx) {
  switch (dimindx) {
    case 0: return __ptx_mad(__nvvm_read_ptx_sreg_ntid_x(), __nvvm_read_ptx_sreg_ctaid_x(), __nvvm_read_ptx_sreg_tid_x());
    case 1: return __ptx_mad(__nvvm_read_ptx_sreg_ntid_y(), __nvvm_read_ptx_sreg_ctaid_y(), __nvvm_read_ptx_sreg_tid_y());
    case 2: return __ptx_mad(__nvvm_read_ptx_sreg_ntid_z(), __nvvm_read_ptx_sreg_ctaid_z(), __nvvm_read_ptx_sreg_tid_z());
    default: return 0;
  }
}