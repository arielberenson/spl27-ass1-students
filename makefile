all: link

link: compile
	@echo "linking files"
	g++ -o bin/server bin/Main.o bin/Model.o bin/Server.o

compile: src/Main.cpp src/Model.cpp src/Server.cpp
	@echo "Compiling source code"
	g++ -g -Wall -Weffc++ -std=c++11 -c -Iinclude src/Main.cpp -o bin/Main.o
	g++ -g -Wall -Weffc++ -std=c++11 -c -Iinclude src/Model.cpp -o bin/Model.o
	g++ -g -Wall -Weffc++ -std=c++11 -c -Iinclude src/Server.cpp -o bin/Server.o

clean: 
	@echo "Cleaning bin directory"
	rm -rf bin/*