
# Compile all target
all: hp bf ht

# Compile hp_main target
hp: 
	@echo " Compile hp_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,'$$ORIGIN/../lib' ./examples/hp_main.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main -O2

# Compile bf_main target
bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,'$$ORIGIN/../lib' ./examples/bf_main.c ./src/record.c -lbf -o ./build/bf_main -O2;

# Compile ht_main target
ht:
	@echo " Compile ht_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,'$$ORIGIN/../lib' ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o ./build/ht_main -O2

# Clean target
clean:
	@echo " Cleaning up... ";
	rm -f ./build/hp_main ./build/bf_main ./build/ht_main
	@echo " Clean done.";
