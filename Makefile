linux:
	gcc TcpSpeed.c -Wimplicit-function-declaration -DLINUX -o output/TcpSpeed
clean:
	rm -f output/TcpSpeed
