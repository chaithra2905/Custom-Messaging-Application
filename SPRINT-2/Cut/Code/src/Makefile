all: release debug

release:
	gcc -o ../bin/ser_release -Werror server/server.c server/fl_trans.c
	gcc -o ../bin/cli_release -Werror client/client.c client/fl_trans.c

debug:
	gcc -o ../bin/ser_debug -Werror -g server/server.c server/fl_trans.c
	gcc -o ../bin/cli_debug -Werror -g client/client.c client/fl_trans.c

clean:
	rm -rf ../bin/*

