build_dir := build
$(shell mkdir -p $(build_dir))

.PHONY: all

all:
	g++ -std=c++11 main_event_app.cpp EventApp.cpp -lcurl -o $(build_dir)/eventapp

original:
	g++ -std=c++11 main.cpp -lcurl -o $(build_dir)/eventapporiginal	
