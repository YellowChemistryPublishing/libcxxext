# Assumes at least C11, C17 onwards is conditionally supported per dependent target.

add_library(sys.BuildSupport.stdcinc INTERFACE)
target_precompile_headers(sys.BuildSupport.stdcinc INTERFACE
    <assert.h>
    <complex.h>
    <ctype.h>
    <errno.h>
    <fenv.h>
    <float.h>
    <inttypes.h>
    <iso646.h>
    <limits.h>
    <locale.h>
    <math.h>
    <setjmp.h>
    <signal.h>
    <stdarg.h>
    <stdatomic.h>
    $<$<COMPILE_FEATURES:c_std_23>:$<1:<stdbit.h>>>
    <stdbool.h>
    $<$<COMPILE_FEATURES:c_std_23>:$<1:<stdckdint.h>>>
    <stddef.h>
    <stdint.h>
    <stdio.h>
    <stdlib.h>
    <string.h>
    <tgmath.h>
    <threads.h>
    <time.h>
    <uchar.h>
    <wchar.h>
    <wctype.h>
)
