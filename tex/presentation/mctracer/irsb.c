typedef struct {
   ...

   IRStmt**   stmts;

   ...
} IRSB;

typedef struct {
    enum {
	...
        Ist_Store,
        Ist_Load,
	Ist_NoOp,
        ...
    } IRStmtTag, /* different kinds of statements */

    ...

    struct {
        Addr64 addr;   /* instruction address */
        Int len;    /* instruction length */
        UChar  delta;  /* addr = program counter as encoded in guest state - delta */
    } IMark;

    ...
} IRStmt;
	
