/* mctracer.h */
#define SSIM_MATRIX_TRACING_START(addr, m, n, ele_size, name)   \
{
    unsigned int _qzz_res;                                      \
    VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0,                     \
                               VG_USERREQ__TRACE_MATRIX,        \
                               addr, m, n, ele_size, name);     \
}

#define SSIM_MATRIX_TRACING_STOP(...

/* tr_main.c */
Bool mt_handle_client_request(ThreadId tid, UWord *args, UWord *ret) {
    switch(args[0]) {
	case VG_USERREQ__TRACE_MATRIX:
            ssim_matrix_tracing_start((Addr)args[1], args[2], args[3], args[4], (char*)args[5])	
            break;

        case VG_USERREQ__UNTRACE_MATRIX:
            ssim_matrix_tracing_stop((Addr)args[1]);
            break;
    }

    return True;
}

