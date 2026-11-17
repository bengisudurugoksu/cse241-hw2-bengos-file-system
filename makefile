all: clean compile run

compile: main.cpp
	@echo "-------------------------------------------"
	@echo "Compiling..."
	@g++ -o test main.cpp -lm

run:
	@echo "-------------------------------------------"
	@echo "Running the tests...."
	@echo "======================================================================="
	@./test
	@echo "======================================================================="
	@echo "Completed tests...."

clean:
	@echo "-------------------------------------------"
	@echo "Cleaning..."
	@rm -f *.o
	@rm -f test