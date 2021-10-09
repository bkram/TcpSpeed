linux:
	gcc TcpSpeed.c -DLINUX -o output/TcpSpeed
clean:
	rm -f output/TcpSpeed
