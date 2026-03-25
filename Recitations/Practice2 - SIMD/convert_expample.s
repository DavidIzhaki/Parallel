.intel_syntax noprefix

.globl cfunction
cfunction:
    # Arguments:  i -> edi, d -> xmm0, l -> rsi, f -> xmm1

    # ---------------------------------
    # Compute (d + i) in xmm0
    # ---------------------------------
    vcvtsi2sd xmm2, xmm2, edi      # xmm2 = (double)i
    vaddsd    xmm0, xmm0, xmm2     # xmm0 = d + i

    # ---------------------------------
    # Compute (f * l) in xmm1
    # ---------------------------------
    vcvtsi2ss xmm2, xmm2, rsi      # xmm2 = (float)l
    vmulss    xmm1, xmm1, xmm2     # xmm1 = f * l

    # ---------------------------------
    # Convert denominator to double
    # ---------------------------------
    vcvtss2sd xmm1, xmm1, xmm1     # xmm1 = (double)(f * l)

    # ---------------------------------
    # Final division and return
    # ---------------------------------
    vdivsd    xmm0, xmm0, xmm1     # xmm0 = (d + i) / (double)(f * l)
    ret