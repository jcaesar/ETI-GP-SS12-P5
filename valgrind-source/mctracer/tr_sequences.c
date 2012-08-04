/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

/**
 * TODO: replace C datatypes with the corresponding valgrind types
 *
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "pub_tool_libcbase.h" // VG_(strlen), VG_(memset)
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic)

static void delete_access_pattern(traced_matrix * matr, access_pattern * const ap, access_pattern ** const patterned_access)
{
	VG_(free)(ap->steps);
	VG_(free)(ap->sequences);
	// remove markings of ap in the access buffer
	if(patterned_access)
	{
		int i;
		for(i = 0; i < matr->access_event_count; ++i)
			if(patterned_access[i] == ap)
				patterned_access[i] = 0;
	}
	// remove usages of ap in the sequences
/*	int i, j;
	ap->length = 0; // little hack to make sure current ap is not processed
	for(i = 0; i < MAX_PATTERNS_PER_MATRIX; ++i)
		if(matr->access_patterns[i].length != 0)
			for(j = 0; j < matr->access_patterns[i].sequence_count; ++j)
				if(matr->access_patterns[i].sequences[j].next_pattern == ap)
					matr->access_patterns[i].sequences[j].next_pattern = 0; // TODO: eliminate equal sequences*/
	VG_(memset)(ap, 0, sizeof(access_pattern));
}

static void mark_pattern_findings(traced_matrix * matr, access_pattern * const ap, /*const*/ access_pattern ** const patterned_access)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	if(ap->length == 0) 
		return;

	int lastwas = -1; // variable to mark if the last cycle marked the pattern
	unsigned int j;
	for(j = 0; j < count; ++j) // loop over access buffer, loop variable is modified inside of the loop
	{
		if(ap->length + j >= count) // can't match pattern if it's longer than the remaining accesses
			break;
			
		unsigned int apstep;
		for(apstep = 0; apstep < ap->length; ++apstep) // loop over access method steps
			if(ap->steps[apstep].offset_m != accbuf[j+apstep].offset.m ||
			   ap->steps[apstep].offset_n != accbuf[j+apstep].offset.n)
				break;
		if(apstep == ap->length || (apstep > 0 && lastwas != -1)) // means that all were equal || means that the pattern continued but didn't finish
		{
			unsigned int k;
			for(k = 0; k < apstep; ++k)
			{
				if(accbuf[j+k].is_hit)
					++ap->steps[k].hits;
				else
					++ap->steps[k].misses;
				patterned_access[j+k] = ap;
			}
			if(apstep == ap->length)
			{
				if(lastwas == -1)
				{
					// matching pattern for the first time in this sequence. look backward and mark accesses which also belong to this pattern
					unsigned int l = ap->length;
					for(k = j - 1; k > j - ap->length; --k)
					{
						--l;
						if(patterned_access[k] ||
						   ap->steps[l].offset_m != accbuf[k].offset.m ||
						   ap->steps[l].offset_n != accbuf[k].offset.n)
							break;
						patterned_access[k] = ap;

					}
				}
				lastwas = j; 
				j += ap->length - 1; // -1 to counter loop increment
				++(ap->occurences);
			}
			else
			{
				// what we found and marked was shorter than the pattern.
				// that means, that we could have matched a full pattern if it was in a different rotation
				// => rotate the pattern
				matrix_access_method temp[MAX_PATTERN_LENGTH];
				VG_(memcpy)(temp, ap->steps, sizeof(matrix_access_method)*apstep);
				VG_(memmove)(ap->steps, ap->steps + apstep, sizeof(matrix_access_method)*(ap->length - apstep));
				VG_(memcpy)(ap->steps + (ap->length - apstep), temp, sizeof(matrix_access_method)*apstep);
			}
		}
		else
			lastwas = -1;
	}
}

// check whether oap is a subpattern of any other patterns and kill it, if so
static void subpattern_elimination_check(traced_matrix * const matr, access_pattern * const oap, access_pattern ** const patterned_access)
{
	if(oap->length == 0)
		return;
	unsigned int j;
	for(j = 0; j < MAX_PATTERNS_PER_MATRIX; ++j) // loop over patterns which could eliminate oap
	{
		access_pattern * const iap = matr->access_patterns + j; // inner access pattern	
		if(iap == oap)
			continue;
		if(iap->length < oap->length)
			continue;
		unsigned int k;
		for(k = 0; k < iap->length; ++k)
		{
			unsigned int l, o;
			for(l = 0, o = k; l < oap->length; ++l)
			{
				if(o >= iap->length)
					o = 0;
				if(oap->steps[l].offset_n != iap->steps[o].offset_n ||
				   oap->steps[l].offset_m != iap->steps[o].offset_m)
					break;
				++o;
			}
			if(l == oap->length)
			{
				if(oap->steps == 0)
					VG_(tool_panic)("inconsistent state of patterns");
				delete_access_pattern(matr, oap, patterned_access);
				mark_pattern_findings(matr, iap, patterned_access);
				return;
			}
		}
	}
}

void process_pattern_buffer(traced_matrix * matr)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	// array for marking, for each access event, to which pattern it belongs
	// it would fit onto the stack, but I don't feel good about putting it there.
	// it could also be done with uint8_t indices to matr->access_patterns, if you think it consumes too much mem
	// This is a multi-purpose-variable. It is used for checking if an access event has already been processed,
	//  and for sequence computation
	static access_pattern ** patterned_access = 0;
	if(!patterned_access);
		patterned_access = VG_(malloc)("pattern findings in access event buffer",
		                               sizeof(access_pattern*)*MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH);
	VG_(memset)(patterned_access, 0, sizeof(access_pattern*)*MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH);

	// go over all the existing access patterns and find matching accesses
	unsigned int i;
	for(i = 0; i < MAX_PATTERNS_PER_MATRIX; ++i) // loop over access patterns
		mark_pattern_findings(matr, matr->access_patterns + i, patterned_access);
	// find new patterns
	for(i = 0; i < count - MAX_PATTERN_LENGTH*2; ++i)
	{
		if(patterned_access[i])
			continue;
		// single access repetition locating
		unsigned int length;
		for(length = 1; length <= MAX_PATTERN_LENGTH; ++length)
			if(accbuf[i].offset.n == accbuf[i+length].offset.n &&
			   accbuf[i].offset.m == accbuf[i+length].offset.m)
				break;
		if(length > MAX_PATTERN_LENGTH)
			continue;
		unsigned int j;
		// sequence equality check
		for(j = 1; j < length; ++j)
			if(accbuf[i+j].offset.n != accbuf[i+length+j].offset.n ||
			   accbuf[i+j].offset.m != accbuf[i+length+j].offset.m)
				break;
		if(j < length)
			continue;
		// found a pattern, find a place to store it in.
		// (Keeping a list of all located patterns around would require a much more sofisticated pattern recognition algorithm
		//  it would probably be slower, too.)
		unsigned int matr_accesses = matr->loads.misses + matr->loads.hits + matr->stores.misses + matr->stores.hits;
		float max_expendability = 0;
		access_pattern * rap = 0; // replaced access pattern
		for(j = 0; j < MAX_PATTERNS_PER_MATRIX; ++j)
		{
			access_pattern * const ap = matr->access_patterns + j;
			if(ap->length == 0)
			{
				rap = ap;
				break;
			}
			// the relevance of a pattern is computed by the fraction of accesses that match that pattern since that pattern emerged.
			float expendability = (double)(matr_accesses - ap->accesses_before_lifetime + 1000) / (ap->occurences * ap->length + 1000); // the constant summands prevent instabilities with small numbers
			if(expendability > max_expendability)
			{
				rap = ap;
				max_expendability = expendability;
			}
		}
		delete_access_pattern(matr, rap, patterned_access);
		rap->steps = VG_(malloc)("access pattern",length*sizeof(access_pattern));
		for(j = 0; j < length; ++j)
		{
			rap->steps[j].offset_n = accbuf[i+j].offset.n;
			rap->steps[j].offset_m = accbuf[i+j].offset.m;
			rap->steps[j].hits = 0;
			rap->steps[j].misses = 0;
		}
		rap->length = length;
		// rap->occurences = 0; done by deletion
		rap->accesses_before_lifetime = matr_accesses;
		mark_pattern_findings(matr, rap, patterned_access);
		{
		}
	}
	// consistency check: eliminate patterns which are subpatterns to others
	for(i = 0; i < MAX_PATTERNS_PER_MATRIX; ++i) // loop over patterns which may be eliminated
		subpattern_elimination_check(matr, matr->access_patterns + i, patterned_access); 
	// process sequences of patterns
	// if we already have a pattern, skip the first few accesses that could be a part of it. If they were, they were counted in the previous iteration
	access_pattern * cap = matr->current_pattern; // convenience variable. I want references. :(
	// note on the loop variable: the processing of patterns actually always lags a step behind
	for(i = 0; i <= count - MAX_PATTERN_LENGTH; i += cap ? cap->length : 1)
	{
		// still within the same ap? just count
		if(patterned_access[i] == cap)
		{
			if(cap)
			{
				// before we're ready to jump over the whole pattern length (done by loop increment), we have to check whether the pattern is complete
				// since mark_pattern_findings is allowed to mark sequences which are not a multiple of the pattern
				unsigned int j;
				for(j = 1; j < cap->length; ++j)
					if(patterned_access[i+j] != cap)
						break; // skips the continue for the outermost loop
				if(j >= cap->length)
				{
					matr->current_sequence_length++;
					continue; // continues the outermost loop
				}
				else
					i += j;
			}
			else
				continue;
		}
		if(cap)
		{
			matr->current_sequence_length++;
			unsigned int j;
			for(j = 0; j < cap->sequence_count; ++j)
			{
				if(cap->sequences[j].length == matr->current_sequence_length &&
				   cap->sequences[j].next_access.offset_n == accbuf[i].offset.n &&
				   cap->sequences[j].next_access.offset_m == accbuf[i].offset.m &&
				   cap->sequences[j].next_pattern == patterned_access[i])
					break;
			}
			if(j >= cap->sequence_count)
			{
				++(cap->sequence_count);
				if(cap->sequence_count > cap->sequence_allocated) // oops, buffer full. Reallocate.
				{
					signed int newlen = cap->sequence_allocated * 2 - 16;
					if(newlen < 32)
						newlen = 32;
					pattern_sequence * new = VG_(malloc)("pattern sequence", newlen*sizeof(pattern_sequence));
					VG_(memcpy)(new, cap->sequences, cap->sequence_allocated * sizeof(pattern_sequence));
					cap->sequence_allocated = newlen;
					cap->sequences = new;
				}
				cap->sequences[j].length = matr->current_sequence_length;
				cap->sequences[j].next_pattern = patterned_access[i];
				cap->sequences[j].next_access.offset_n = accbuf[i].offset.n;
				cap->sequences[j].next_access.offset_m = accbuf[i].offset.m;
				cap->sequences[j].occurences = 0;
			}
			++(cap->sequences[j].occurences);
			if(accbuf[i].is_hit)
				++(cap->sequences[j].next_access.hits);
			else
				++(cap->sequences[j].next_access.misses);

		}
		cap = patterned_access[i];
		matr->current_sequence_length = 0;
	}
	matr->current_pattern = cap;
	// preserve the last few accesses which can not be accounted to maximum pattern lengths
	// copy as much patterns as the sequence recognition hasn't processed
	unsigned int copylen = count - i - (cap? cap->length:0);
	VG_(memmove)(accbuf, accbuf + count - copylen, copylen);
	matr->access_event_count = copylen;
}

void update_matrix_pattern_stats(traced_matrix * matr, short offset_n, short offset_m, bool is_hit)
{
	if(!matr || !matr->access_buffer)
		VG_(tool_panic)("internal error: matrix passed for pattern finding without access buffer.");
	// store new event
	access_event ev;
	ev.is_hit = is_hit;
	ev.offset.n = offset_n;
	ev.offset.m = offset_m;
	matr->access_buffer[matr->access_event_count] = ev;
	if(++matr->access_event_count == MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH)
		process_pattern_buffer(matr);
}
