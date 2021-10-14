para = srv

all:	${para}

srv:	srv.cpp my_function.h
		g++ srv.cpp -o srv 

clean:
		rm srv