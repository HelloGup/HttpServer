bin=httpServer
cgi=test_cgi
cc=g++
LD_FLAGS=-std=c++11 -lpthread
src=main.cc
out_path=$(shell pwd)/output
include=$(shell pwd)/mysql_lib/include
lib=$(shell pwd)/mysql_lib/lib
mysqlname=mysqlclient

.PHONY:all
all:$(bin) $(cgi)

$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)
$(cgi):test_cgi.cc
	$(cc) -o $@ $^ -I $(include) -L $(lib) -l $(mysqlname) -std=c++11
	mv $(cgi) $(shell pwd)/wwwroot/cgi

.PHONY:clean
clean:
	rm -f $(bin) $(shell pwd)/wwwroot/cgi/$(cgi)
	rm -rf $(out_path)

.PHONY:output
output:
	mkdir -p $(out_path)
	cp -rf $(shell pwd)/wwwroot $(out_path)
	cp $(bin) $(out_path)
	cp -rf $(shell pwd)/mysql_lib $(out_path)


