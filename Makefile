build: 
	g++ clk.cpp -o clk.out
	g++ -std=c++0x scheduler.cpp -o sch.out
	#g++ FilesGenerator.cpp -o Files.out
	g++ -std=c++0x processGenerator.cpp -o processGenerator
	g++ process.cpp -o process.out

clean:
	rm -f *.out  processes.txt
	
all: clean build

run:
	#clear
	./processGenerator
