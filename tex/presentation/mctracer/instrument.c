IRSB* mt_instrument (..., IRSB* sbIn, ...) {
    IRSB* sbOut = deepCopyIRSB(sbIn);

    for (i=0; i < sbIn->stmts_used; i++) {
        switch (sbIn->stmts[i]->tag) {
             case Ist_Store:
		 /* Fuege Aufruf von ssim_store ein */
                 addStmtToIRSB(sbOut, ..., ssim_store);
 
             case Ist_Load:
		 /* Fuege Aufruf von ssim_load ein */
                 addStmtToIRSB(sbOut, ..., ssim_load);
        }
    }

    return sbOut;
}

VG_REGPARM(2) void ssim_store(Addr addr, SizeT size) {
     update_matrix_stats(addr, size, MATRIX_STORE);
}

VG_REGPARM(2) void ssim_load(Addr addr, SizeT size) {
     update_matrix_stats(addr, size, MATRIX_LOAD);
}
