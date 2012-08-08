/* Auszug aus valgrind.h */
/* Implementierung fuer 64bit Linux */
#define VALGRIND_DO_CLIENT_REQUEST(_zzq_request, _zzq_arg1, ..., _zzq_arg5)
{
    _zzq_args[0] = (unsigned int)(_zzq_request);                  \
    _zzq_args[1] = (unsigned int)(_zzq_arg1);                     \
    _zzq_args[2] = (unsigned int)(_zzq_arg2);                     \
    _zzq_args[3] = (unsigned int)(_zzq_arg3);                     \
    _zzq_args[4] = (unsigned int)(_zzq_arg4);                     \
    _zzq_args[5] = (unsigned int)(_zzq_arg5);                     \
    __asm__ volatile ( \
                      /* Rotiere 64bit Register EDI, 64-mal */
                      "roll $3,  %%edi ; roll $13, %%edi\n\t" \ 
                      "roll $29, %%edi ; roll $19, %%edi\n\t" \
                      "xchgl %%ebx, %%ebx" \
                      : "=d" (_zzq_result) \
                      /* Lege die Anfangsadresse der Parameter in EAX */
                      : "a" (&_zzq_args[0]), "0" (_zzq_default) \
                      : "cc", "memory"); \
    _zzq_result; \
}
