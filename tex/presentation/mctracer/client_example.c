double* matrix = (double*) calloc(100*100, sizeof(double));

SIM_MATRIX_TRACING_START(matrix, 100, 100, sizeof(double), "name");

...

/* Arbeite mit der Matrix */

...

SSIM_MATRIX_TRACING_STOP(matrix)
