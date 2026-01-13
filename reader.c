#include <stdio.h>
#include <string.h>

#define MAX_LENGTH 256

void read_obj (char path[], int *num_points, int *num_faces, float points[][3], int faces[][3]) {
	FILE *filepointer = fopen(path, "r");
	
	char line[MAX_LENGTH] = "";
	int read;
	int i = 0;
	int point_i = 0;
	int face_i = 0;
	while ((read = fgetc(filepointer)) != EOF) {
		line[i] = (char) read;
		if (read == '\n') {
			char mode = line[0];
			
			if (mode == 'v') {
				int k = 2;
				for (int j = 0; j < 3; j++) {
					char coord[5] = "";
					int coord_k = 0;
					while (line[k] != ' ' && line[k] != '\n') {
						coord[coord_k] = line[k];
						k++;
						coord_k++;
					}
					points[point_i][j] = atof(coord);
					k++;
				}
				
				point_i++;
			} else if (mode == 'f') {
				int k = 2;
				for (int j = 0; j < 3; j++) {
					char index[2] = "";
					int index_k = 0;
					while (line[k] != ' ' && line[k] != '\n') {
						index[index_k] = line[k];
						k++;
						index_k++;
					}
					faces[face_i][j] = atoi(index);
					index_k = 0;
					k++;
				}
				
				face_i++;
			}
			
			i = 0;
		} else {
			i++;
		}
	}
	
	*num_points = point_i;
	*num_faces = face_i;
}
