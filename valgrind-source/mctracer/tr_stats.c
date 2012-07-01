static traced_matrix* find_matrix(Addr access)
{
	int i;
	int tmax = traced_matrices_count - stopped_matrices_count;

	for(i=0; i < tmax; ++i)
	{
		if (traced_matrices_index[i]->start <= access &&
		        traced_matrices_index[i]->end > access)
		{
			return traced_matrices_index[i];
		}
	}

	return 0;
}

static void update_matrix_access_stats(traced_matrix* matr, matrix_access_data* access_data, int is_hit, Addr addr, SizeT size)
{
	if(matr && access_data)
	{
		// transform addr to (m,n) representation
		Addr tmp = (addr - (*matr).start) / (*matr).ele_size;
		unsigned short n = tmp % (*matr).m;
		unsigned short m = (tmp - n) / (*matr).m;

		// ignore the first access
		if (access_data->last_access.m != SHRT_MIN && access_data->last_access.n != SHRT_MIN)
		{

			// calculate the current access method
			short offset_n = n - access_data->last_access.n;
			short offset_m = m - access_data->last_access.m;

			int i;
			int amc = access_data->access_methods_count;
			bool found = false;

			// look for the access method and increase the hit/miss count
			for (i=0; i < amc; ++i)
			{
				matrix_access_method* am = access_data->access_methods+i;

				if (am->offset_m == offset_m && am->offset_n == offset_n)
				{
					// we got a matching access method
					// ... now check for hit or miss
					if (is_hit)
					{
						am->hits++;
					}
					else
					{
						am->misses++;
					}

					found = true;
					break;
				}
			}

			// if no access method was found => add a new one
			if (!found)
			{
				matrix_access_method* am = (access_data->access_methods + access_data->access_methods_count);
				am->offset_m = offset_m;
				am->offset_n = offset_n;

				if (is_hit)
				{
					// it's a hit
					am->hits = 1;
					am->misses = 0;
				}
				else
				{
					// it's a miss
					am->misses = 1;
					am->hits = 0;
				}

				access_data->access_methods_count++;

				if (access_data->access_methods_count >= MAX_MATRIX_ACCESS_METHODS)
				{
					VG_(tool_panic)("Max. number of access methods exceeded.");
				}
			}
		}

		// update
		access_data->last_access.n = n;
		access_data->last_access.m = m;

	}
}

static void update_matrix_stats(Addr addr, SizeT size, char type)
{
	traced_matrix* matr = find_matrix(addr);

	if (matr)
	{
		int is_hit = cache_ref(addr, size);
		int offset = (addr - matr->start) / matr->ele_size;

        if (type == MATRIX_LOAD) {
            //update access method stats
            update_matrix_access_stats(matr, &(matr->load_access_data), is_hit, addr, size);

            // update general access stats
            if (is_hit) {
                matr->load_count[offset].hits++;
                matr->loads.hits++;
            } else {
                matr->load_count[offset].misses++;
                matr->loads.misses++;
            }
        } else {
            update_matrix_access_stats(matr, &(matr->store_access_data), is_hit, addr, size);

            if (is_hit) {
                matr->store_count[offset].hits++;
                matr->stores.hits++; 
            } else {
                matr->store_count[offset].misses++;
                matr->stores.misses++;
            }
        }
	}
}
