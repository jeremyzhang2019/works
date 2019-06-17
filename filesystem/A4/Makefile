
CFLAGS=-std=gnu99 -Wall -g

ext2 :  ext2_mkdir.o ext2_cp.o ext2_ln.o ext2_rm.o ext2_restore.o ext2_checker.o ops.o
	gcc $(CFLAGS) -o ext2 $^

%.o : %.c ext2.h mydata.h
	gcc $(CFLAGS) -g -c $<

clean : 
	rm -f *.o ext2 *~
