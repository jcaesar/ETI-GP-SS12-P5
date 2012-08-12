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
	int i, j;
	ap->length = 0; // little hack to make sure current ap is not processed
	for(i = 0; i < clo_ssim_max_patterns_per_matrix; ++i)
		if(matr->access_patterns[i].length != 0)
			for(j = 0; j < matr->access_patterns[i].sequence_count; ++j)
				if(matr->access_patterns[i].sequences[j].next_pattern == ap)
					matr->access_patterns[i].sequences[j].next_pattern = 0; // TODO: eliminate equal sequences
	VG_(memset)(ap, 0, sizeof(access_pattern));
}

static void mark_pattern_findings(traced_matrix * matr, access_pattern * const ap, /*const*/ access_pattern ** const patterned_access)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	if(!patterned_access || ap->length == 0)
		return;

	int lastwas = -1; // variable to mark if the last cycle marked the pattern
	unsigned int j;
	for(j = 0; j < count; ++j) // loop over access buffer, loop variable is modified inside of the loop
	{
		if(ap->length + j >= count) // can't match pattern if it's longer than the remaining accesses
			break;

		unsigned int apstep;
		for(apstep = 0; apstep < ap->length; ++apstep) // loop over access method steps
			if(!coordinates_equal(ap->steps[apstep].offset, accbuf[j+apstep].offset))
				break;
		if(apstep == ap->length || (apstep > 0 && lastwas != -1)) // means that all were equal || means that the pattern continued but didn't finish
		{
			unsigned int k;
			for(k = 0; k < apstep; ++k)
				// The access statistics is not done here because we don't yet know which accesses will be saved for the next round
				patterned_access[j+k] = ap;
			if(apstep == ap->length)
			{
				if(lastwas == -1)
				{
					// matching pattern for the first time in this sequence. look backward and mark accesses which also belong to this pattern
					unsigned int l = ap->length;
					for(k = j - 1; k > (int64_t)j - ap->length && k != UINT_MAX; --k)
					{
						--l;
						if(patterned_access[k] || !coordinates_equal(ap->steps[l].offset, accbuf[k].offset))
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
				matrix_access_method temp[clo_ssim_max_pattern_length];
				VG_(memcpy)(temp, ap->steps, sizeof(matrix_access_method)*apstep);
				VG_(memmove)(ap->steps, ap->steps + apstep, sizeof(matrix_access_method)*(ap->length - apstep));
				VG_(memcpy)(ap->steps + (ap->length - apstep), temp, sizeof(matrix_access_method)*apstep);
				lastwas = -1;
			}
		}
		else
			lastwas = -1;
	}
}

// check whether oap is a subpattern of any other patterns and kill it, if so
// The usage and structure of this function is kind of a relict.
// The correct usage of it would be to rewrite it so it checks whether any pattern is subpattern to oap. That way, find_new_patterns would just need one call to this. It's not performance-relevant though, so I kept it.
static bool subpattern_elimination_check(traced_matrix * const matr, access_pattern * const oap, access_pattern ** const patterned_access)
{
	if(oap->length == 0)
		return true;
	unsigned int j;
	for(j = 0; j < clo_ssim_max_patterns_per_matrix; ++j) // loop over patterns which could eliminate oap
	{
		access_pattern * const iap = matr->access_patterns + j; // containing access pattern
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
				if(!coordinates_equal(oap->steps[l].offset, iap->steps[o].offset))
					break;
				++o;
			}
			if(l == oap->length)
			{
				if(oap->steps == 0)
					VG_(tool_panic)("inconsistent state of patterns");
				delete_access_pattern(matr, oap, patterned_access);
				mark_pattern_findings(matr, iap, patterned_access);
				return true;
			}
		}
	}
	return false;
}

static void find_new_patterns(traced_matrix * const matr, access_pattern ** const patterned_access)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	unsigned int i;
	for(i = 0; i <= count - clo_ssim_max_pattern_length*2; ++i)
	{
		if(patterned_access[i])
			continue;
		// single access repetition locating
		unsigned int length;
		for(length = 1; length <= clo_ssim_max_pattern_length; ++length)
			if(coordinates_equal(accbuf[i].offset, accbuf[i+length].offset))
				break;
		if(length > clo_ssim_max_pattern_length)
			continue;
		unsigned int j;
		// sequence equality check
		for(j = 1; j < length; ++j)
			if(!coordinates_equal(accbuf[i+j].offset, accbuf[i+length+j].offset))
				break;
		if(j < length)
			continue;
		access_pattern nap; // don't try anything funny like mark_pattern_finings on this one, many of my algorithms don't like patterns outside of matrices
		VG_(memset)(&nap, 0, sizeof(access_pattern));
		nap.steps = VG_(malloc)("access pattern",length*sizeof(access_pattern));
		for(j = 0; j < length; ++j)
		{
			nap.steps[j].offset = accbuf[i+j].offset;
			nap.steps[j].hits = 0;
			nap.steps[j].misses = 0;
		}
		nap.length = length;
		unsigned int matr_accesses = matr->loads.misses + matr->loads.hits + matr->stores.misses + matr->stores.hits;
		nap.accesses_before_lifetime = matr_accesses;
		if(!subpattern_elimination_check(matr, &nap, NULL)) // Should never return true, buuut, you know, safety first
		{
			// found a pattern, find a place to store it in.
			// (Keeping a list of all located patterns around would require a much more sofisticated pattern recognition algorithm
			//  it would probably be slower, too.)
			float max_expendability = 0;
			access_pattern * rap = 0; // replaced access pattern
			for(j = 0; j < clo_ssim_max_patterns_per_matrix; ++j)
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
			VG_(memcpy)(rap, &nap, sizeof(access_pattern));
			mark_pattern_findings(matr, rap, patterned_access);
			for(j = 0; j < clo_ssim_max_patterns_per_matrix; ++j)
				subpattern_elimination_check(matr, matr->access_patterns + j, patterned_access);
		}
	}
}

static void reallocate_sequencearray(access_pattern * const cap)
{
	if(cap->sequence_count > cap->sequence_allocated) // oops, buffer full. Reallocate.
	{
		signed int newlen = cap->sequence_allocated * 2 - 16;
		if(newlen < 18)
			newlen = 18;
		pattern_sequence * new = VG_(malloc)("pattern sequence", newlen*sizeof(pattern_sequence));
		VG_(memcpy)(new, cap->sequences, cap->sequence_allocated * sizeof(pattern_sequence));
		VG_(free)(cap->sequences);
		cap->sequence_allocated = newlen;
		cap->sequences = new;
	}

}

static unsigned int find_sequences(traced_matrix * const matr, access_pattern ** const patterned_access)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	access_pattern * cap = matr->current_pattern; // convenience variable. I want references. :(
	// note on the condition: I've switched around between 3,2 and 1*clo_ssim_max_pattern_length multiple times. It should be 3 so a pattern that emerges on the last few accesses can be safely recognized and categorized.
	unsigned int i = 0;
	unsigned int pattern_offset = INT_MAX; // since we don't know which access of a pattern actually is the first, we have to trick around a little
	while(i <= count - 3*clo_ssim_max_pattern_length)
	{
		if(patterned_access[i] != cap)
		{
			if(cap)
			{
				unsigned int j;
				for(j = 0; j < cap->sequence_count; ++j) // can we find a sequence that looks like the current one?
				{
					if(cap->sequences[j].length == matr->current_sequence_length &&
					   coordinates_equal(cap->sequences[j].next_access.offset, accbuf[i].offset) &&
					   cap->sequences[j].next_pattern == patterned_access[i])
						break;
				}
				if(j >= cap->sequence_count) // There was no sequence like the current one
				{
					++(cap->sequence_count);
					reallocate_sequencearray(cap);
					cap->sequences[j].length = matr->current_sequence_length;
					cap->sequences[j].next_pattern = patterned_access[i];
					cap->sequences[j].next_access.offset = accbuf[i].offset;
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
			pattern_offset = INT_MAX;
		}
		if(cap)
		{
			if(pattern_offset == INT_MAX) // new pattern, we have to adjust the offset anew
			{
				pattern_offset = 0;
				while(!coordinates_equal(cap->steps[pattern_offset].offset, accbuf[i].offset))
				{
					++pattern_offset;
					if(pattern_offset > cap->length)
						VG_(tool_panic)("Invalid pattern marking buffer state: pattern marked where it did not belong");
				}
			}
			unsigned int j;
			for(j = 0; j < cap->length; ++j)
			{
				if(patterned_access[i+j] != cap)
					break;
				// Here is the best place to do the statistics on the pattern. Why?
				// Because we know which accesses will not be copied and have to be counted next round
				if(accbuf[i+j].is_hit)
					++cap->steps[pattern_offset].hits;
				else
					++cap->steps[pattern_offset].misses;
				if(++pattern_offset >= cap->length)
					pattern_offset = 0;
			}
			if(j >= cap->length)
				matr->current_sequence_length++;
			i += j;
		}
		else
			++i;
	}
	matr->current_pattern = cap;
	return i;
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
	static access_pattern ** patterned_access = 0; // If I didn't use it to 
	if(!patterned_access)
		patterned_access = VG_(malloc)("pattern findings in access event buffer",
		                               sizeof(access_pattern*)*MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH);
	VG_(memset)(patterned_access, 0, sizeof(access_pattern*)*MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH);

	// go over all the existing access patterns and find matching accesses
	unsigned int i;
	for(i = 0; i < clo_ssim_max_patterns_per_matrix; ++i) // loop over access patterns
		mark_pattern_findings(matr, matr->access_patterns + i, patterned_access);
	// find new patterns
	find_new_patterns(matr, patterned_access);
	// process sequences of patterns
	unsigned int uncounted = 
	find_sequences(matr, patterned_access); // this is also where the hit/miss statistics for the patterns' accesses are updated
	// preserve the last few accesses which can not be accounted to maximum pattern lengths
	// copy as much patterns as the sequence recognition hasn't processed
	unsigned int copylen = count - uncounted;
	VG_(memmove)(accbuf, accbuf + count - copylen, copylen*sizeof(access_event));
	matr->access_event_count = copylen;
}

void update_matrix_pattern_stats(traced_matrix * matr, unsigned short m, unsigned short n, bool is_hit)
{
	if(!matr || !matr->access_buffer)
		VG_(tool_panic)("internal error: matrix passed for pattern finding without access buffer.");
	// store new event
	access_event ev;
	ev.is_hit = is_hit;
	ev.offset.m = m;
	ev.offset.n = n;
	matr->access_buffer[matr->access_event_count] = ev;
	if(++matr->access_event_count == MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH)
		process_pattern_buffer(matr);
}
