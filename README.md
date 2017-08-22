# The Laminar Cortex Model 

#### Copyright (C) 2015-2017 Jiaxin Du.

This software implements the laminar cortex model, see

   Du J, Vegh V, & Reutens DC, [PLOS Compt Biol 8(10): e1002733](https://doi.org/10.1371/journal.pcbi.1002733); & [NeuroImage 94: 1-11](https://doi.org/10.1016/j.neuroimage.2014.03.015).

### Compile
In Linux, the program can be compiled using GNU C++ compiler, for example, 

``` g++ -o runlcm -O2 -fopenmp -lm runlcm.cpp src/*.cpp ```

where ```-fopenmp``` is optional, it enables the program to run on multi-procesors.

In Windows, the program can be compiled using Visual C++ compiler, just import 
the files into a new C++ project, and compile it. You may need to enable 
OpenMP parallellisation manually, see MSDN website for more information.

See [Makefile](Makefile) for more information.
### Run

A configuration is needed to run the program. The configuration file defines the
the architecture of the model. An example is provided in "para.cfg".  Use the 
following command to run the program

``` ./runlcm –p prefix –f para.cfg  -l run.log –o volt.dat```.

The options are

    -p prefix       Specify the prefix for all files. If no prefix is specified, 
                    the current directory will be used.
    -f para.cfg     Specify the parameter configuration file (input)
    -l run.log      Specify the log file name (output, will be created or rewritten)
    -o volt.dat     Specify the voltage output file name 
                    (output, will be created or rewritten).

### Analysis
  The output file is organised in the following format
 1. The first 1024 bytes store header information (i.e., 0 to 1023 byte).
    This section is organised as parameter name and value pairs. It may also contain 
	comments started with ```//```.
	
	These following parameters indicate the structure of the other sections
	```
	ELMT_NUM = 400;        //number of element
	NG_NUM = 14;           //number of neuron group
	NUM_SIZE = 4;          //data type size
	CFG_POS = 1024;        //configure section starting position (start from zero)
	CFG_LEN = 25684;       //configure section length
	DATA_POS = 26708;      //data section starting position
	DATA_LEN = 229427200;  //data section length
	BLOCK_SIZE = 22405;    //the size of a block size (voltage information at a time point)
	BLOCK_NUM = 10240;     //number of block in data section
	```
 2. The parameter configuration information is kept from ```CFG_POS``` to ```CFG_POS+CFG_LEN-1```.
    These are pure text information.
 
 3. The neuronal voltage info is stored from ```DATA_POS``` to ```DATA_POS+DATA_LEN-1```.
 
    They are divided into ```BLOCK_NUM``` blocks. Each block stores neuronal voltage information at 
	a time point. The data are stored in binary format using 4-byte floating type (i.e., ```float``` type in C and C++), and organised in the following order
	```
	neuron 1 in column 1, neuron 2 in column 1, neuron 3 in column 1, ...
	neuron 1 in column 2, neuron 2 in column 2, neuron 3 in column 2, ...
	```
	The order of the neuron groups is the same as their order in the parameter "NEURONS", e.g., 
	```NEURONS={neuron_A, neuron_B, ..., neuron_C};```.

    The order of the column is organsied as
    
    ```	
	column 1             ->           column n
	column n+1           ->           column 2n
	... 
	column n*(n-1)       ->           column n*n 
     ```
     .
	
 4. A program ```mktree``` is provided to demonstrate how to read the voltage file and store them 
    in a ROOT file.
  
    [ROOT](https://root.cern.ch) is free data analysis framework built on C++ from CERN.

    You need both the ROOT library and the LCM source code to compile the program.

    Assuming you have the LCM source code in directory "src" and a working root software installed, then you can compile the program using the following command

    ``` g++ -o mktree mktree.cpp src/*.cpp -O2 $(root-config --cflags --glibs) ```

    Usage:
    ```./mktree volt.dat volt.root```

    where "volt.dat" is the input file, which is the LCM output file, 
       and "volt.root" is the root file to be created. The third argument is optional. If not given, the root file 
       would be the same as the input file with the extension changed to ".root".
       
--------
Any problems or permission request for the program should be address to ```jiaxin_dot_du_at_outlook_dot_com``` or ```jiaxin_dot_du_at_uqconnect_dot_edu_dot_au``` (replace ```_dot_``` with ```.``` and ```_at_``` with ```@```).
	
The program is provided for the completeness of the foresaid and future publications. 
Though every effort has been made to assure the program behaves as prescribed, 
mistakes or "bugs" may persist. It is the responsibility of users to ensure the 
"correctness" of the program.

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along 
with this program; if not, see <http://www.gnu.org/licenses/>.