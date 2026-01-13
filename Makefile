main: main.c engine.c reader.c
	gcc main.c -lglfw -lGL -lm -o a.out
