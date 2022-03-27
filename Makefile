bin=httpServer
cgi=test_cgi
cc=g++
LD_FLAGS=-std=c++11 -lpthread
src=main.cc
out_path=$(shell pwd)/output

.PHONY:all
all:$(bin) $(cgi)

$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)
$(cgi):test_cgi.cc
	$(cc) -o $@ $^ -std=c++11
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



