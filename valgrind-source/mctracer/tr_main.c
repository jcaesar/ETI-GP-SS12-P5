/*--------------------------------------------------------------------*/
/*--- An memtracer Valgrind tool.                        tr_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of McTracer, a Valgrind tool for memory tracing.
   It is mainly a copy of the Lackey example Valgrind tool.

   Written for ETI @ TUM, (C) 2010-2011 Josef Weidendorfer.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/


#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_options.h"
#include "pub_tool_machine.h"     // VG_(fnptr_to_fnentry)
#include "pub_tool_threadstate.h"

#include "mctracer.h"
#include "simplesim.h"

/*------------------------------------------------------------*/
/*--- Command line options                                 ---*/
/*------------------------------------------------------------*/


/* The name of the function at which tracing should start.
 * Override with command line option --fnstart.
 * If empty (default), starts tracing when entering "main".
 */
static Char* clo_fnstart = "main";

static Bool mt_process_cmd_line_option(Char* arg)
{
	if      VG_STR_CLO(arg, "--fnstart", clo_fnstart) {}
	else
		return False;

	tl_assert(clo_fnstart);
	return True;
}

static void mt_print_usage(void)
{
	VG_(printf)(
	    "    --fnstart=<name>        start tracing when entering this function [%s]\n",
	    clo_fnstart
	);
}

static void mt_print_debug_usage(void)
{
	VG_(printf)(
	    "    (none)\n"
	);
}

/*------------------------------------------------------------*/
/*--- Stuff for --basic-counts                             ---*/
/*------------------------------------------------------------*/

static void start_tracing(void)
{
	//TODO: Remove this function
}


/*------------------------------------------------------------*/
/*--- Stuff for memory access tracing                      ---*/
/*------------------------------------------------------------*/

#define MAX_DSIZE    512

typedef
IRExpr
IRAtom;

typedef
enum { Event_Ir, Event_Dr, Event_Dw }
EventKind;

typedef
struct
{
	EventKind  ekind;
	IRAtom*    addr;
	Int        size;
}
Event;

/* Up to this many unnotified events are allowed.  Must be at least two,
   so that reads and writes to the same address can be merged into a modify.
   Beyond that, larger numbers just potentially induce more spilling due to
   extending live ranges of address temporaries. */
#define N_EVENTS 4

/* Maintain an ordered list of memory events which are outstanding, in
   the sense that no IR has yet been generated to do the relevant
   helper calls.  The SB is scanned top to bottom and memory events
   are added to the end of the list, merging with the most recent
   notified event where possible (Dw immediately following Dr and
   having the same size and EA can be merged).

   This merging is done so that for architectures which have
   load-op-store instructions (x86, amd64), the instr is treated as if
   it makes just one memory reference (a modify), rather than two (a
   read followed by a write at the same address).

   At various points the list will need to be flushed, that is, IR
   generated from it.  That must happen before any possible exit from
   the block (the end, or an IRStmt_Exit).  Flushing also takes place
   when there is no space to add a new event.

   If we require the simulation statistics to be up to date with
   respect to possible memory exceptions, then the list would have to
   be flushed before each memory reference.  That's a pain so we don't
   bother.

   Flushing the list consists of walking it start to end and emitting
   instrumentation IR for each event, in the order in which they
   appear. */

static Event events[N_EVENTS];
static Int   events_used = 0;

static void print_trace_tid(void)
{
}

static VG_REGPARM(2) void trace_instr(Addr addr, SizeT size)
{
//    VG_(printf)("I %08lx,%lu\n", addr, size);
}

static void flushEvents(IRSB* sb)
{
	Int        i;
	Char*      helperName;
	void*      helperAddr;
	IRExpr**   argv;
	IRDirty*   di;
	Event*     ev;

	for (i = 0; i < events_used; i++)
	{

		ev = &events[i];

		// Decide on helper fn to call and args to pass it.
		switch (ev->ekind)
		{
		case Event_Ir:
			helperName = "trace_instr";
			helperAddr =  trace_instr;
			break;

		case Event_Dr:
			helperName = "trace_load";
			helperAddr =  ssim_load;
			break;

		case Event_Dw:
			helperName = "trace_store";
			helperAddr =  ssim_store;
			break;

		default:
			tl_assert(0);
		}

		// Add the helper.
		argv = mkIRExprVec_2( ev->addr, mkIRExpr_HWord( ev->size ) );
		di   = unsafeIRDirty_0_N( /*regparms*/2,
		                                      helperName, VG_(fnptr_to_fnentry)( helperAddr ),
		                                      argv );
		addStmtToIRSB( sb, IRStmt_Dirty(di) );
	}

	events_used = 0;
}

// WARNING:  If you aren't interested in instruction reads, you can omit the
// code that adds calls to trace_instr() in flushEvents().  However, you
// must still call this function, addEvent_Ir() -- it is necessary to add
// the Ir events to the events list so that merging of paired load/store
// events into modify events works correctly.
static void addEvent_Ir ( IRSB* sb, IRAtom* iaddr, UInt isize )
{
	Event* evt;
	tl_assert( (VG_MIN_INSTR_SZB <= isize && isize <= VG_MAX_INSTR_SZB)
	           || VG_CLREQ_SZB == isize );
	if (events_used == N_EVENTS)
		flushEvents(sb);
	tl_assert(events_used >= 0 && events_used < N_EVENTS);
	evt = &events[events_used];
	evt->ekind = Event_Ir;
	evt->addr  = iaddr;
	evt->size  = isize;
	events_used++;
}

static
void addEvent_Dr ( IRSB* sb, IRAtom* daddr, Int dsize )
{
	Event* evt;
	tl_assert(isIRAtom(daddr));
	tl_assert(dsize >= 1 && dsize <= MAX_DSIZE);
	if (events_used == N_EVENTS)
		flushEvents(sb);
	tl_assert(events_used >= 0 && events_used < N_EVENTS);
	evt = &events[events_used];
	evt->ekind = Event_Dr;
	evt->addr  = daddr;
	evt->size  = dsize;
	events_used++;
}

static
void addEvent_Dw ( IRSB* sb, IRAtom* daddr, Int dsize )
{
	Event* evt;
	tl_assert(isIRAtom(daddr));
	tl_assert(dsize >= 1 && dsize <= MAX_DSIZE);

	if (events_used == N_EVENTS)
		flushEvents(sb);
	tl_assert(events_used >= 0 && events_used < N_EVENTS);
	evt = &events[events_used];
	evt->ekind = Event_Dw;
	evt->size  = dsize;
	evt->addr  = daddr;
	events_used++;
}


/*------------------------------------------------------------*/
/*--- Client requests                                      ---*/
/*------------------------------------------------------------*/


static
Bool mt_handle_client_request(ThreadId tid, UWord *args, UWord *ret)
{

	if (!VG_IS_TOOL_USERREQ('M','T',args[0]))
		return False;

	switch(args[0])
	{
	case VG_USERREQ__PRINT:
		print_trace_tid();
		VG_(printf)("P %s\n", (Char*)args[1]);
		*ret = 0;                 /* meaningless */
		break;

	case VG_USERREQ__PRINTA:
		print_trace_tid();
		VG_(printf)("A %08lx\n", (Addr)args[1]);
		*ret = 0;                 /* meaningless */
		break;

	case VG_USERREQ__PRINTU:
		print_trace_tid();
		VG_(printf)("U %lu\n", (UWord)args[1]);
		*ret = 0;                 /* meaningless */
		break;

	case VG_USERREQ__TRACING:
		VG_(tool_panic)("Client program used removed MCTRACER_TRACING_ON/OFF");

	case VG_USERREQ__TRACE_MATRIX:
		if(!ssim_matrix_tracing_start((Addr)args[1], args[2], args[3], args[4], (char*)args[5]))
			VG_(tool_panic)("Internal error: Could not trace matrix");
		*ret = 0;                 /* meaningless */
		break;
	case VG_USERREQ__UNTRACE_MATRIX:
		if(!ssim_matrix_tracing_stop((Addr)args[1]))
			VG_(tool_panic)("Invalid freeing notification");
		*ret = 0;                 /* meaningless */
		break;

	case VG_USERREQ__FLUSHCACHE:
		ssim_flush_cache();
		*ret = 0;                 /* meaningless */
		break;

	default:
		return False;
	}

	return True;
}

/*------------------------------------------------------------*/
/*--- Basic tool functions                                 ---*/
/*------------------------------------------------------------*/

static void mt_post_clo_init(void)
{
	ssim_init();
}

static
IRSB* mt_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout,
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
	IRDirty*   di;
	Int        i;
	IRSB*      sbOut;
	Char       fnname[100];
	IRTypeEnv* tyenv = sbIn->tyenv;

	if (gWordTy != hWordTy)
	{
		/* We don't currently support this case. */
		VG_(tool_panic)("host/guest word size mismatch");
	}

	/* Set up SB */
	sbOut = deepCopyIRSBExceptStmts(sbIn);

	// Copy verbatim any IR preamble preceding the first IMark
	i = 0;
	while (i < sbIn->stmts_used && sbIn->stmts[i]->tag != Ist_IMark)
	{
		addStmtToIRSB( sbOut, sbIn->stmts[i] );
		i++;
	}

	events_used = 0;

	for (/*use current i*/; i < sbIn->stmts_used; i++)
	{
		IRStmt* st = sbIn->stmts[i];
		if (!st || st->tag == Ist_NoOp) continue;

		switch (st->tag)
		{
		case Ist_NoOp:
		case Ist_AbiHint:
		case Ist_Put:
		case Ist_PutI:
		case Ist_MBE:
			addStmtToIRSB( sbOut, st );
			break;

		case Ist_IMark:

			/* An unconditional branch to a known destination in the
			 * guest's instructions can be represented, in the IRSB to
			 * instrument, by the VEX statements that are the
			 * translation of that known destination. This feature is
			 * called 'SB chasing' and can be influenced by command
			 * line option --vex-guest-chase-thresh.
			 *
			 * To get an accurate count of the calls to a specific
			 * function, taking SB chasing into account, we need to
			 * check for each guest instruction (Ist_IMark) if it is
			 * the entry point of a function.
			 */
			tl_assert(clo_fnstart);
			if (clo_fnstart[0] &&
			        VG_(get_fnname_if_entry)(st->Ist.IMark.addr,
			                                 fnname, sizeof(fnname))
			        && 0 == VG_(strcmp)(fnname, clo_fnstart))
			{
				di = unsafeIRDirty_0_N(
				         0, "start_tracing",
				         VG_(fnptr_to_fnentry)( &start_tracing ),
				         mkIRExprVec_0() );
				addStmtToIRSB( sbOut, IRStmt_Dirty(di) );
			}

			// WARNING: do not remove this function call, even if you
			// aren't interested in instruction reads.  See the comment
			// above the function itself for more detail.
			addEvent_Ir( sbOut, mkIRExpr_HWord( (HWord)st->Ist.IMark.addr ),
			             st->Ist.IMark.len );
			addStmtToIRSB( sbOut, st );
			break;

		case Ist_WrTmp:
		{
			IRExpr* data = st->Ist.WrTmp.data;
			if (data->tag == Iex_Load)
			{
				addEvent_Dr( sbOut, data->Iex.Load.addr,
				             sizeofIRType(data->Iex.Load.ty) );
			}
		}
		addStmtToIRSB( sbOut, st );
		break;

		case Ist_Store:
		{
			IRExpr* data  = st->Ist.Store.data;
			addEvent_Dw( sbOut, st->Ist.Store.addr,
			             sizeofIRType(typeOfIRExpr(tyenv, data)) );
		}
		addStmtToIRSB( sbOut, st );
		break;

		case Ist_Dirty:
		{

			Int      dsize;
			IRDirty* d = st->Ist.Dirty.details;
			if (d->mFx != Ifx_None)
			{
				// This dirty helper accesses memory.  Collect the details.
				tl_assert(d->mAddr != NULL);
				tl_assert(d->mSize != 0);
				dsize = d->mSize;
				if (d->mFx == Ifx_Read || d->mFx == Ifx_Modify)
					addEvent_Dr( sbOut, d->mAddr, dsize );
				if (d->mFx == Ifx_Write || d->mFx == Ifx_Modify)
					addEvent_Dw( sbOut, d->mAddr, dsize );
			}
			else
			{
				tl_assert(d->mAddr == NULL);
				tl_assert(d->mSize == 0);
			}

			addStmtToIRSB( sbOut, st );
			break;
		}

		case Ist_CAS:
		{
			/* We treat it as a read and a write of the location.  I
			   think that is the same behaviour as it was before IRCAS
			   was introduced, since prior to that point, the Vex
			   front ends would translate a lock-prefixed instruction
			   into a (normal) read followed by a (normal) write. */

			Int    dataSize;
			IRCAS* cas = st->Ist.CAS.details;
			tl_assert(cas->addr != NULL);
			tl_assert(cas->dataLo != NULL);
			dataSize = sizeofIRType(typeOfIRExpr(tyenv, cas->dataLo));
			if (cas->dataHi != NULL)
				dataSize *= 2; /* since it's a doubleword-CAS */
			addEvent_Dr( sbOut, cas->addr, dataSize );
			addEvent_Dw( sbOut, cas->addr, dataSize );

			addStmtToIRSB( sbOut, st );
			break;
		}

		case Ist_Exit:
			flushEvents(sbOut);
			addStmtToIRSB( sbOut, st );      // Original statement
			break;

		default:
			tl_assert(0);
		}
	}

	/* At the end of the sbIn.  Flush outstandings. */
	flushEvents(sbOut);

	return sbOut;
}

static void mt_fini(Int exitcode)
{
	ssim_save_stats("/tmp/testfile.etis");
}

static void mt_pre_clo_init(void)
{
	VG_(details_name)            ("McTracer");
	VG_(details_version)         (NULL);
	VG_(details_description)     ("a memory tracer tool");
	VG_(details_copyright_author)(
	    "Copyright (C) 2002-2011, and GNU GPL'd, by NN & JW.");
	VG_(details_bug_reports_to)  (VG_BUGS_TO);
	VG_(details_avg_translation_sizeB) ( 200 );

	VG_(basic_tool_funcs)          (mt_post_clo_init,
	                                mt_instrument,
	                                mt_fini);
	VG_(needs_command_line_options)(mt_process_cmd_line_option,
	                                mt_print_usage,
	                                mt_print_debug_usage);
	VG_(needs_client_requests)     (mt_handle_client_request);
}

VG_DETERMINE_INTERFACE_VERSION(mt_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                tr_main.c ---*/
/*--------------------------------------------------------------------*/
