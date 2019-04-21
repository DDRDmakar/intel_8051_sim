Intel 8051 simulator
====================
![#f03c15](https://placehold.it/15/f03c15/000000?text=+) `Warinig! Now this simulator works under UNIX-like systems only. Windows version is under develop`
  
MCS51 core program model, which allows you to run and debug programs  
(binary and text formats)  
add breakpoints, look memory contents and make memory dumps.  

Building project
----------------

To build the source code you need libraries:
- jansson
- pthread

**RHEL**  
su -c 'yum install jansson jansson-devel'  
su -c 'dnf install jansson jansson-devel'  
**Ubuntu, Debian**  
su -c 'apt-get install libjansson libjansson-dev'  
**Or**  
download and install it manually: [HERE](https://github.com/akheron/jansson)  
  
**Next you can just run "make ENDIANNESS=0" in the "build" directory. Then you should see this output, if there were no errors**  
(if you use big-endian machine, run "make ENDIANNESS=1")
```
[DDRDmakar@localhost build]$ make ENDIANNESS=0
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/main.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/mnemonic.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/file.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/error.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -D _POSIX_C_SOURCE=200809L  -c ../code/execute.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/tools.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -c ../code/binhex.c
gcc -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=0 -o sim8051 main.o mnemonic.o file.o error.o execute.o tools.o binhex.o
```
After build you get binary executable "sim8051". Run it.  
**IMPORTANT!!** - Binary file should stay in one directory with "resources" folder.  

Using emulator 
--------------
This emulator uses different modes, selected by flags:

Flag                | Description                    | Default value | Example
------------------- | -------------------------------|---------------|--------------------------
**-h --help**       | Display basic information about program (get this information in terminal) |   | -h
**-d --debug**      | Enable debugging messages (Information about breakpoints and savepoints) | | -d
**-i --infile**     | Name of input program file | **essential argument** | -i myfile
**-o --outfle**     | Name of output file and snapshots	| "memory" | -o myfile
**-c --clk**        | Time delay between instructions execution (ms) | 0 | -c 1000
**-v --verbose**    | Verbose mode. Shows instructions sequence in console | | -v
**-m --mode**       | Type of input program file (bin or text) | text mode | -m bin
**-b --break**      | Add breakpoint.	Argument is address (unsigned hex), where breakpoint will be placed.	^ before address means, that breakpoint will be activated before this instruction.	_ before address means, that breakpoint will be activated after this instruction. |   | -b ^2C
**--nobreak**       | Ignore breakpoints | | --nobreak
**-s --save**       | Add savepoint	Argument is address (unsigned hex), where savepoint will be placed.	^ before address means, that snapshot will be made before this instruction.	_ before address means, that snapshot will be made after this instruction. |   | -s _2C
**-z --convert**    | Convert input binary file into text snapshot |  | -z
**-e --end**        | Define program end point.	Argument is address (unsigned hex)	If PC is > this value, program finishes and saves final snapshot into file.	|   | -e E6
**--epm**           | Enable external program memory support (64 KiB) (default resident program memory is 4 KiB) | | --epm
**--edm**           | Enable external data memory support (64 KiB) (default resident data memory is 256 bytes) | | --edm
**--step**          | Step-by-step mode. Execute instructions one-by-one pressing Enter | | --step

**Running program**
As an example, let's run program, which performs bubble sort of array (16 elements)
```
[DDRDmakar@localhost bubblesort]$ '../../build/sim8051' -dv -c 10 --mode text -i sort.json -o snapshots/dump.json
[PC #0000]: #02 #00 #06 (LJMP_ad16)
[PC #0006]: #75 #81 #2E (MOV_ad_d)
[PC #0009]: #12 #01 #5E (LCALL_ad16)
[PC #015E]: #75 #82 #00 (MOV_ad_d)
[PC #0161]: #22         (RET)
[PC #000C]: #E5 #82     (MOV_a_ad)
...
[PC #00ED]: #87 #06     (MOV ad @r1)
[PC #00EF]: #09         (INC r1)
[PC #00F0]: #87 #07     (MOV ad @r1)
[PC #00F2]: #19         (DEC r1)
[PC #00F3]: #E5 #2A     (MOV a ad)
[PC #00F5]: #F5 #2E     (MOV ad a)
[PC #00F7]: #04         (INC a)
[PC #00F8]: #25 #E0     (ADD a ad)
[PC #00FA]: #24 #08     (ADD a #d)
[PC #00FC]: #F8         (MOV r0 a)
[PC #00FD]: #86 #04     (MOV ad @r0)
[PC #00FF]: #08         (INC r0)
...
[PC #010B]: #63 #F0 #80 (XRL ad #d)
[PC #010E]: #95 #F0     (SUBB a ad)
[PC #0110]: #50 #20     (JNC rel)
[PC #0132]: #05 #2A     (INC ad)
[PC #0134]: #E4         (CLR a)
[PC #0135]: #B5 #2A #98 (CJNE a ad rel)
[PC #00D0]: #C3         (CLR c)
[PC #00D1]: #E5 #2A     (MOV a ad)
[PC #00D3]: #9A         (SUBB a r2)
[PC #00D4]: #E5 #2B     (MOV a ad)
[PC #00D6]: #64 #80     (XRL a #d)
[PC #00D8]: #8B #F0     (MOV ad r3)
[PC #00DA]: #63 #F0 #80 (XRL ad #d)
[PC #00DD]: #95 #F0     (SUBB a ad)
[PC #00DF]: #50 #5B     (JNC rel)
[PC #013C]: #E5 #2C     (MOV a ad)
[PC #013E]: #45 #2D     (ORL a ad)
[PC #0140]: #60 #18     (JZ rel)
[PC #015A]: #22         (RET)
Program ended at #1000
```
After running this program we can read the dump file (snapshots/dump.json), where we see, that our integers were sorted correctly (addresses 0x0008 - 0x0027).

License
-------
```
MIT License

Copyright (c) 2018-2019 DDRDmakar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
