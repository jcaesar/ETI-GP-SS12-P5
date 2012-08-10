StandardInput[][] readData(File file) 
		throws FileNotFoundException, IOException{
        FileInputStream fis=new FileInputStream(file);
        
        //Check for correct file header and read number of matrices that are 
        //specified in the file
	//If no correct file header is found, an IOException is thrown
        int num_matrix=readHeader(fis);
        
        //First read all matrix headers as specified in the data format spec
        MatrixHeader[] headers=readMatrixHeaders(num_matrix,fis);
        
        //Second read all corresponding information, i.e. absolute matrices,
        //relative jumps, patterns and sequences
        StandardInput[][] inputs=readMatrices(fis,headers);
        
        return inputs;
    }

private static StandardInput[][] readMatrices(FileInputStream fis,
				MatrixHeader[] headers) throws IOException {
        //For every matrix first element in array is loads, second is stores
        StandardInput[][] input=new StandardInput[headers.length][2];
        
        for(int i=0; i<headers.length; i++){
            input[i]=new StandardInput[2];
            
            //Copy some general info from matrix headers
            input[i][0]=new StandardInput();
            input[i][0].setHits(headers[i].getLoadHits());
            input[i][0].setMisses(headers[i].getLoadMisses());
            
            input[i][1]=new StandardInput();
            input[i][1].setHits(headers[i].getStoreHits());
            input[i][1].setMisses(headers[i].getStoreMisses());
            
            //Read absolute matrix for load/stores
            input[i][0].setAbsoluteMatrix(readByteMatrix(fis,headers[i]));
            input[i][1].setAbsoluteMatrix(readByteMatrix(fis,headers[i]));
            
            //Read relative jumps for loads/stores
            input[i][0].setRelativeJumps(
			readRelativeJumps(fis,headers[i].getNumLoadJumps()));
            input[i][1].setRelativeJumps(
		       readRelativeJumps(fis,headers[i].getNumStoreJumps()));
            
            //Patterns and Sequences do not distinguish between Loads/Stores,
            //so only save them for Load
            Pattern[] patterns=readPatterns(fis,headers[i].getNumPatterns());
            input[i][0].setPatterns(patterns);
            input[i][0].setSequences(readSequences(
				fis,headers[i].getNumSequences(),patterns));
            
            //Read the matrix's name represented as C-style string
            String name=readName(fis);
            input[i][0].setName(name);
            input[i][1].setName(name);
        }
        return input;
    }
