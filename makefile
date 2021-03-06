build_dir := build
$(shell mkdir -p $(build_dir))

.PHONY: all

all:
	g++ -std=c++11 main_event_app.cpp EventApp.cpp -lcurl -o $(build_dir)/eventapp

tests:
	g++ -std=c++11 tests.cpp EventApp.cpp -lcurl -o $(build_dir)/tests

test_stringsearch:
	g++ -std=c++11 -g test_StringSearch.cpp StringSearch.cpp -o $(build_dir)/test_stringsearch
