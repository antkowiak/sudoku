
all : gnu clang

gnu : Makefile src/main.cpp src/*.h
	g++ -g -std=c++14 \
	-Wall -Wextra -Wpedantic \
	src/main.cpp -o sudoku_gnu

run : 
	chmod 755 sudoku_gnu sudoku_clang
	./sudoku_gnu && ./sudoku_clang

run_gnu : 
	chmod 755 sudoku_gnu
	./sudoku_gnu

clang : Makefile src/main.cpp src/*.h
	clang++ -g -std=c++14 \
	-Wall -Wextra -Wpedantic \
	src/main.cpp -o sudoku_clang

run_clang : 
	chmod 755 sudoku_clang
	./sudoku_clang

format :
	clang-format -i -style=file src/main.cpp
	clang-format -i -style=file src/*.h

line_endings :
	dos2unix src/*.cpp src/*.h

clean :
	\rm -f sudoku_gnu sudoku_clang

strip :
	strip sudoku_gnu sudoku_clang

