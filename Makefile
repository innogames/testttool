HOSTNAME!=hostname
CC=c++
CFLAGS=-pedantic -Wall -Wextra -D__HOSTNAME__=\"$(HOSTNAME)\" -I/usr/local/include -I/opt/libevent/include -g3
LD=c++
#LDFLAGS=-L/usr/local/lib -L/opt/libevent/lib
LDFLAGS=-L/usr/local/lib
#DLIBS=-lssl -lcrypto -levent
DLIBS=-lssl -lcrypto
SLIBS=/opt/libevent/lib/libevent_core.a /opt/libevent/lib/libevent.a /opt/libevent/lib/libevent_openssl.a
ALLOBJS=testtool.o lb_pool.o lb_node.o healthcheck.o healthcheck_http.o healthcheck_ping.o healthcheck_dns.o msg.o pfctl.o

all: testtool

testtool: $(ALLOBJS)
	$(LD) $(LDFLAGS) $(LIBS) $(DLIBS) $(ALLOBJS) $(SLIBS) -o $@

#%.o: %.cpp
#	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o testtool testtool.core

#	@echo '^' all dependencies: $^ - not supported in bsd?
#	@echo '?' more recent than the target: $?
#	@echo '+' keeps duplicates and gives you the entire list: $+
#	@echo '<' the first dependency: $<
#	@echo '@' the name of the target: $@
#	$(LD) $(LDFLAGS) $(LIBS) $(DLIBS) $(SLIBS) $(ALLOBJS) -o $@

	
testtool.o: testtool.cpp lb_pool.h lb_node.h healthcheck.h healthcheck_*.h msg.h
lb_pool: lb_pool.cpp lb_pool.h lb_node.h msg.h
lb_node.o: lb_node.cpp lb_node.h lb_pool.h healthcheck.h healthcheck_*.h msg.h
healthcheck.o: healthcheck.cpp healthcheck.h healthcheck_*.h msg.h
healthcheck_http.o: healthcheck_http.cpp healthcheck_http.h healthcheck.h lb_node.h msg.h
healthcheck_ping.o: healthcheck_ping.cpp healthcheck_ping.h healthcheck.h lb_node.h msg.h
healthcheck_dns.o: healthcheck_dns.cpp healthcheck_dns.h healthcheck.h lb_node.h msg.h
msg.o: msg.cpp msg.h

#tick.o: tick.c tick.h
#msg.o: msg.c msg.h
#pfctl.o: pfctl.c pfctl.h


