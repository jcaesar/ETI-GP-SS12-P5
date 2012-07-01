/*
 * Writes the collected data into file
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "pub_tool_libcbase.h" // VG_(strlen)
#include "pub_tool_libcfile.h" // VG_(open), VG_(write)
#include "pub_tool_vki.h" // VKI_O_TRUNC ... 
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic

void ssim_save_stats(HChar* fname)
{
	SysRes fd_res = VG_(open)(fname, VKI_O_WRONLY|VKI_O_TRUNC|VKI_O_CREAT, VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IWGRP);
    Int fd = sr_Res(fd_res);

    // no SESE for julius
    if (fd == -1) {
        char msg[100];
        VG_(sprintf)(msg, "Failed to open the file '%s' for writing.", fname);
        VG_(tool_panic)(msg);
        return;
    }

	/**
	 * FILE HEADER (4 bytes)
	 */

    // magic number
    byte magic_num[2] = {0xAF, 0xFE};
    // version of the file format
    byte version = 0x2;
    byte matr_count = (byte) traced_matrices_count;

	VG_(write)(fd, magic_num, 2*sizeof(byte));
	VG_(write)(fd, &version, sizeof(byte));
	VG_(write)(fd, &matr_count, sizeof(byte));

	/**
	 * MATRICES HEADER (N*34 bytes)
	 */

	// MH:MIBADR => initalize with the DH and MH size
	unsigned int mibaddr = 4 + traced_matrices_count*34;
	int i;
	for (i = 0; i < traced_matrices_count; ++i)
	{
		// MH:GM (2 bytes)
		VG_(write)(fd, &(traced_matrices[i].m), sizeof(ushort));

		// MH:GN (2 bytes)
		VG_(write)(fd, &(traced_matrices[i].n), sizeof(ushort));

		// MH:AZL (1 byte), limited to 8
		int tmp = traced_matrices[i].load_access_data.access_methods_count;
		byte accm_loads_count = tmp < 8 ? (byte)tmp : 8;
		VG_(write)(fd, &accm_loads_count, sizeof(byte));

        // MH:AZS (1 byte), limited to 8
		tmp = traced_matrices[i].store_access_data.access_methods_count;
		byte accm_stores_count = tmp < 8 ? (byte)tmp : 8;
		VG_(write)(fd, &accm_stores_count, sizeof(byte));

		// MH:ADR (8 byte)
		VG_(write)(fd, &(traced_matrices[i].start), sizeof(Addr));

		// MH:MIBADR (4 byte)
		VG_(write)(fd, &mibaddr, sizeof(unsigned int));
    
        // MH:ALH (4 byte)
        VG_(write)(fd, &(traced_matrices[i].loads.hits), sizeof(unsigned int));
        
        // MH:ALM (4 byte)
        VG_(write)(fd, &(traced_matrices[i].loads.misses), sizeof(unsigned int));
        
        // MH:ASH (4 byte)
        VG_(write)(fd, &(traced_matrices[i].stores.hits), sizeof(unsigned int));
        
        // MH:ASM (4 byte)
        VG_(write)(fd, &(traced_matrices[i].stores.misses), sizeof(unsigned int));

		// calculate the address of the next MIB
		// matrices
		mibaddr += 2 * traced_matrices[i].m * traced_matrices[i].n;
		// access methods
		mibaddr += 12 * (accm_loads_count + accm_stores_count);
		// name + \0
		mibaddr += VG_(strlen)(traced_matrices[i].name) + 1;
	}

	/**
	 * MIB - Matrix Informationblock
	 */
	for (i = 0; i < traced_matrices_count; ++i)
	{
		int rows = traced_matrices[i].m;
		int cols = traced_matrices[i].n;

		byte* loads_array = (byte*) VG_(malloc)("loads byte array", rows * cols);
		byte* stores_array = (byte*) VG_(malloc)("stores byte array", rows * cols);

		/**
		 * Bytearray (BA)
		 */
		int l;
		for(l = 0;  l < rows; ++l)
		{
			int k;
			for (k = 0; k < cols; ++k)
			{
				int offset = l*rows + k;

				// TODO clean up duplicates

				/**
				 * LOADS
				 */
				element_access_count load_count = traced_matrices[i].load_count[offset];

				if (load_count.misses == 0 && load_count.hits == 0)
				{
					// no access at all
					loads_array[offset] = BA_NO_ACCESS_VAL;
				}
				else
				{
					loads_array[offset] = (byte) (BA_MAX_HIT_VAL * ((float)load_count.hits) / (load_count.hits + load_count.misses));
				}

				/**
				 * STORES
				 */
				element_access_count store_count = traced_matrices[i].store_count[offset];

				if (store_count.misses == 0 && store_count.hits == 0)
				{
					// no access at all
					stores_array[offset] = BA_NO_ACCESS_VAL;
				}
				else
				{
					stores_array[offset] = (byte) (BA_MAX_HIT_VAL * ((float)store_count.hits) / (store_count.hits + store_count.misses));
				}
			}
		}

		VG_(write)(fd, loads_array, rows*cols);
		VG_(write)(fd, stores_array, rows*cols);

		// free all that memory
		VG_(free)(loads_array);
		VG_(free)(stores_array);
		VG_(free)(traced_matrices[i].load_count);
		VG_(free)(traced_matrices[i].store_count);

        // LOADS
        write_access_methods(fd, &(traced_matrices[i].load_access_data));
        // STORES
        write_access_methods(fd, &(traced_matrices[i].store_access_data));

		/**
		 * Name (NA)
		 */

		// matrix name + \0
		VG_(write)(fd, traced_matrices[i].name, VG_(strlen)(traced_matrices[i].name)+1);
        VG_(free)(traced_matrices[i].name);
	}

	VG_(close)(fd);
}

static void write_access_methods(Int fd, matrix_access_data* access_data) {
    /**
	 * Access methods (ZA) (2*N*12 bytes)
	 */

	int accm_count = access_data->access_methods_count;

	if (accm_count > 8)
    {
		// sort the access methods according to their cumulative hits and misses
		ssim_qsort(access_data->access_methods, 0, access_data->access_methods_count);
	}

	accm_count = accm_count > 8 ? 8 : accm_count;

	int k;
	for (k = 0; k < accm_count; ++k)
	{
		matrix_access_method accm = access_data->access_methods[k];

		// ZA:OM
		VG_(write)(fd, &(accm.offset_m), sizeof(ushort));

		// ZA:ON
		VG_(write)(fd, &(accm.offset_n), sizeof(ushort));

		// ZA:AH
		VG_(write)(fd, &(accm.hits), sizeof(unsigned int));

		// ZA:AM
		VG_(write)(fd, &(accm.misses), sizeof(unsigned int));
	}
}

/**
 * utitlity functions, because of the missing libc
 */


static void ssim_qsort(matrix_access_method arr[], int begin, int end)
{
	if (end > begin + 1)
	{
		unsigned int piv = arr[begin].hits + arr[begin].misses;
		int l = begin + 1, r = end;

		while (l < r)
		{
			unsigned int ele = arr[l].hits + arr[l].misses;

			if (ele > piv)
			{
				l++;
			}
			else
			{
				swap(&arr[l], &arr[--r]);
			}
		}

		swap(&arr[--l], &arr[begin]);
		ssim_qsort(arr, begin, l);
		ssim_qsort(arr, r, end);
	}
}

static void swap(matrix_access_method* a, matrix_access_method* b)
{
	matrix_access_method t = *a;
	*a = *b;
	*b = t;
}