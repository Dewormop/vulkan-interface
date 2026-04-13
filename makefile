all: compile run
compile: application.cpp
	g++ application.cpp -o application.exe -Ilibs/include -Iheaders -Llibs/lib -lvulkan-1 -lglfw3 -lgdi32 -std=c++23
run: application.exe
	./application.exe
graphic_shader:
	slangc src/shaders/graphic_shader.slang -emit-spirv-directly -fvk-use-entrypoint-name -entry vertex_main -entry fragment_main -o src/shaders/graphic_shader.spv