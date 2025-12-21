obj-m += mymemory.o
mymemory-objs := src/memory.o

clean:
	rm -f tests/test_memory tests/test_main tests/run_tests main libmymemory.so

lib:
	gcc -shared -fPIC -DUNIT_TESTING -Dprintk=printf -DKERN_INFO=\"\" -DKERN_CONT=\"\" -DKERN_ERR=\"\" -I src src/memory.c -o libmymemory.so

main: lib
	gcc -I src src/main.c -L. -lmymemory -o main
	@echo "Build complete. Run with: LD_LIBRARY_PATH=. ./main"

run-main: main
	@echo "--- Running Main ---"
	LD_LIBRARY_PATH=. ./main
	@echo "--- Checking Dependencies ---"
	LD_LIBRARY_PATH=. ldd main
