# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles Yield testproj2 useritoa exectest add sub multiply div forktest execfork newtest multith rpc_locktest rpc_lockacretest rpc_conditiontest rpc_conditiontestsg rpc_conditionw rpc_conditions rpc_conditionb rpc_locktestrel rpc_locktestcrde rpc_senatortest visitor senator president operator
start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
visitor.o: visitor.c
	$(CC) $(CFLAGS) -c visitor.c
visitor: visitor.o visitor.o
	$(LD) $(LDFLAGS) start.o visitor.o -o visitor.coff
	../bin/coff2noff visitor.coff visitor

senator.o: senator.c
	$(CC) $(CFLAGS) -c senator.c
senator: senator.o start.o
	$(LD) $(LDFLAGS) start.o senator.o -o senator.coff
	../bin/coff2noff senator.coff senator
	
operator.o: operator.c
	$(CC) $(CFLAGS) -c operator.c
operator: operator.o start.o
	$(LD) $(LDFLAGS) start.o operator.o -o operator.coff
	../bin/coff2noff operator.coff operator

president.o: president.c
	$(CC) $(CFLAGS) -c president.c
president: president.o start.o
	$(LD) $(LDFLAGS) start.o president.o -o president.coff
	../bin/coff2noff president.coff president
	
rpc_senatortest.o: rpc_senatortest.c
	$(CC) $(CFLAGS) -c rpc_senatortest.c
rpc_senatortest: rpc_senatortest.o start.o
	$(LD) $(LDFLAGS) start.o rpc_senatortest.o -o rpc_senatortest.coff
	../bin/coff2noff rpc_senatortest.coff rpc_senatortest

Yield.o: Yield.c
	$(CC) $(CFLAGS) -c Yield.c
Yield: Yield.o start.o
	$(LD) $(LDFLAGS) start.o Yield.o -o Yield.coff
	../bin/coff2noff Yield.coff Yield
	
testproj2.o: testproj2.c
	$(CC) $(CFLAGS) -c testproj2.c
testproj2: testproj2.o start.o
	$(LD) $(LDFLAGS) start.o testproj2.o -o testproj2.coff
	../bin/coff2noff testproj2.coff testproj2	
	
useritoa.o: useritoa.c
	$(CC) $(CFLAGS) -c useritoa.c
useritoa: useritoa.o start.o
	$(LD) $(LDFLAGS) start.o useritoa.o -o useritoa.coff
	../bin/coff2noff useritoa.coff useritoa	
	
senate_user.o: senate_user.c
	$(CC) $(CFLAGS) -c senate_user.c
senate_user: senate_user.o start.o
	$(LD) $(LDFLAGS) start.o senate_user.o -o senate_user.coff
	../bin/coff2noff senate_user.coff senate_user	
	
exectest.o: exectest.c
	$(CC) $(CFLAGS) -c exectest.c
exectest: exectest.o start.o
	$(LD) $(LDFLAGS) start.o exectest.o -o exectest.coff
	../bin/coff2noff exectest.coff exectest	
	
add.o: add.c
	$(CC) $(CFLAGS) -c add.c
add: add.o start.o
	$(LD) $(LDFLAGS) start.o add.o -o add.coff
	../bin/coff2noff add.coff add

sub.o: sub.c
	$(CC) $(CFLAGS) -c sub.c
sub: sub.o start.o
	$(LD) $(LDFLAGS) start.o sub.o -o sub.coff
	../bin/coff2noff sub.coff sub	
	
multiply.o: multiply.c
	$(CC) $(CFLAGS) -c multiply.c
multiply: multiply.o start.o
	$(LD) $(LDFLAGS) start.o multiply.o -o multiply.coff
	../bin/coff2noff multiply.coff multiply
	
div.o: div.c
	$(CC) $(CFLAGS) -c div.c
div: div.o start.o
	$(LD) $(LDFLAGS) start.o div.o -o div.coff
	../bin/coff2noff div.coff div	
	
forktest.o: forktest.c
	$(CC) $(CFLAGS) -c forktest.c
forktest: forktest.o start.o
	$(LD) $(LDFLAGS) start.o forktest.o -o forktest.coff
	../bin/coff2noff forktest.coff forktest	

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles
	
newtest.o: newtest.c
	$(CC) $(CFLAGS) -c newtest.c
newtest: newtest.o start.o
	$(LD) $(LDFLAGS) start.o newtest.o -o newtest.coff
	../bin/coff2noff newtest.coff newtest
	
execfork.o: execfork.c
	$(CC) $(CFLAGS) -c execfork.c
execfork: execfork.o start.o
	$(LD) $(LDFLAGS) start.o execfork.o -o execfork.coff
	../bin/coff2noff execfork.coff execfork

locktest.o: locktest.c
	$(CC) $(CFLAGS) -c locktest.c
locktest: locktest.o start.o
	$(LD) $(LDFLAGS) start.o locktest.o -o locktest.coff
	../bin/coff2noff locktest.coff locktest
	
multith.o: multith.c
	$(CC) $(CFLAGS) -c multith.c
multith: multith.o start.o
	$(LD) $(LDFLAGS) start.o multith.o -o multith.coff
	../bin/coff2noff multith.coff multith
	
rpc_locktest.o: rpc_locktest.c
	$(CC) $(CFLAGS) -c rpc_locktest.c
rpc_locktest: rpc_locktest.o start.o
	$(LD) $(LDFLAGS) start.o rpc_locktest.o -o rpc_locktest.coff
	../bin/coff2noff rpc_locktest.coff rpc_locktest
	
rpc_locktestcrde.o: rpc_locktestcrde.c
	$(CC) $(CFLAGS) -c rpc_locktestcrde.c
rpc_locktestcrde: rpc_locktestcrde.o start.o
	$(LD) $(LDFLAGS) start.o rpc_locktestcrde.o -o rpc_locktestcrde.coff
	../bin/coff2noff rpc_locktestcrde.coff rpc_locktestcrde
	
rpc_locktestrel.o: rpc_locktestrel.c
	$(CC) $(CFLAGS) -c rpc_locktestrel.c
rpc_locktestrel: rpc_locktestrel.o start.o
	$(LD) $(LDFLAGS) start.o rpc_locktestrel.o -o rpc_locktestrel.coff
	../bin/coff2noff rpc_locktestrel.coff rpc_locktestrel
	
rpc_lockacretest.o: rpc_lockacretest.c
	$(CC) $(CFLAGS) -c rpc_lockacretest.c
rpc_lockacretest: rpc_lockacretest.o start.o
	$(LD) $(LDFLAGS) start.o rpc_lockacretest.o -o rpc_lockacretest.coff
	../bin/coff2noff rpc_lockacretest.coff rpc_lockacretest
	
rpc_conditiontest.o: rpc_conditiontest.c
	$(CC) $(CFLAGS) -c rpc_conditiontest.c
rpc_conditiontest: rpc_conditiontest.o start.o
	$(LD) $(LDFLAGS) start.o rpc_conditiontest.o -o rpc_conditiontest.coff
	../bin/coff2noff rpc_conditiontest.coff rpc_conditiontest
	
rpc_conditiontestsg.o: rpc_conditiontestsg.c
	$(CC) $(CFLAGS) -c rpc_conditiontestsg.c
rpc_conditiontestsg: rpc_conditiontestsg.o start.o
	$(LD) $(LDFLAGS) start.o rpc_conditiontestsg.o -o rpc_conditiontestsg.coff
	../bin/coff2noff rpc_conditiontestsg.coff rpc_conditiontestsg
	
rpc_conditionw.o: rpc_conditionw.c
	$(CC) $(CFLAGS) -c rpc_conditionw.c
rpc_conditionw: rpc_conditionw.o start.o
	$(LD) $(LDFLAGS) start.o rpc_conditionw.o -o rpc_conditionw.coff
	../bin/coff2noff rpc_conditionw.coff rpc_conditionw
	
rpc_conditions.o: rpc_conditions.c
	$(CC) $(CFLAGS) -c rpc_conditions.c
rpc_conditions: rpc_conditions.o start.o
	$(LD) $(LDFLAGS) start.o rpc_conditions.o -o rpc_conditions.coff
	../bin/coff2noff rpc_conditions.coff rpc_conditions
	
rpc_conditionb.o: rpc_conditionb.c
	$(CC) $(CFLAGS) -c rpc_conditionb.c
rpc_conditionb: rpc_conditionb.o start.o
	$(LD) $(LDFLAGS) start.o rpc_conditionb.o -o rpc_conditionb.coff
	../bin/coff2noff rpc_conditionb.coff rpc_conditionb
	
clean:
	rm -f *.o *.coff
