CLEAR_COMMAND := clear

sort:
	$(CLEAR_COMMAND)
	@echo "Deleting data.db..."
	-rm data.db
	@echo "Deleting out0.db..."
	-rm out0.db
	@echo "Deleting out1.db..."
	-rm out1.db
	@echo "Deleting out2.db..."
	-rm out2.db
	@echo "Deleting out3.db..."
	-rm out3.db
	@echo "Deleting out4.db..."
	-rm out4.db
	@echo "Deleting out5.db..."
	-rm out5.db
	@echo "Deleting out6.db..."
	-rm out6.db
	@echo "Deleting out7.db..."
	-rm out7.db
	@echo "Deleting out8.db..."
	-rm out8.db
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2

should_swap_test:
	@$(CLEAR_COMMAND)
	@echo " Compile should_swap_test ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/should_swap_test.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lhp_file -lbf -o ./build/should_swap_test -O2

chunk_functions_test:
	@$(CLEAR_COMMAND)
	@echo "Deleting chunk_test_file.db..."
	-rm chunk_test_file.db
	@echo " Compile chunk_functions_test ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/chunk_functions_test.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lhp_file -lbf -o ./build/chunk_functions_test -O2

sort_Chunk_test:
	@$(CLEAR_COMMAND)
	@echo "Deleting sort_Chunk_test.db..."
	-rm sort_Chunk_test.db
	@echo " Compile sort_Chunk_test ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_Chunk_test.c ./src/record.c ./s
