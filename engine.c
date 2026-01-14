#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_POINTS 32
#define MAX_FACES 32

const float NEAR = 1.0f;

typedef struct Screen {
	float halfwidth;
	float halfheight;
} Screen;

typedef struct Object {
	float position[3];
	float rotationZX[2];
	float points[MAX_POINTS][3];
	int faces[MAX_FACES][3];
	int num_points;
	int num_faces;
} Object;

void add (float vec1[3], float vec2[3]) {
	vec1[0] += vec2[0];
	vec1[1] += vec2[1];
	vec1[2] += vec2[2];
}

void sub (float vec1[3], float vec2[3]) {
	vec1[0] -= vec2[0];
	vec1[1] -= vec2[1];
	vec1[2] -= vec2[2];
}

void mult (float vec[3], float k) {
	vec[0] *= k;
	vec[1] *= k;
	vec[2] *= k;
}

void rotation_matrix_z (float zero_matrix[3][3], float angle) {
	zero_matrix[0][0] = cos(angle);
	zero_matrix[1][0] = sin(angle);
	
	zero_matrix[0][1] = -sin(angle);
	zero_matrix[1][1] = cos(angle);

	zero_matrix[2][2] = 1;
}

void rotation_matrix_x (float zero_matrix[3][3], float angle) {
	zero_matrix[0][0] = 1;

	zero_matrix[1][1] = cos(angle);
	zero_matrix[2][1] = sin(angle);

	zero_matrix[1][2] = -sin(angle);
	zero_matrix[2][2] = cos(angle);
}

void matmul3x3w3x3 (float matrix1[3][3], float matrix2[3][3], float target_matrix[3][3]) {
	target_matrix[0][0] = matrix1[0][0] * matrix2[0][0] + matrix1[0][1] * matrix2[1][0] + matrix1[0][2] * matrix2[2][0];
	target_matrix[0][1] = matrix1[0][0] * matrix2[0][1] + matrix1[0][1] * matrix2[1][1] + matrix1[0][2] * matrix2[2][1];
	target_matrix[0][2] = matrix1[0][0] * matrix2[0][2] + matrix1[0][1] * matrix2[1][2] + matrix2[0][2] * matrix2[2][2];

	target_matrix[1][0] = matrix1[1][0] * matrix2[0][0] + matrix1[1][1] * matrix2[1][0] + matrix1[1][2] * matrix2[2][0];
	target_matrix[1][1] = matrix1[1][0] * matrix2[0][1] + matrix1[1][1] * matrix2[1][1] + matrix1[1][2] * matrix2[2][1];
	target_matrix[1][2] = matrix1[1][0] * matrix2[0][2] + matrix1[1][1] * matrix2[1][2] + matrix2[1][2] * matrix2[2][2];

	target_matrix[2][0] = matrix1[2][0] * matrix2[0][0] + matrix1[2][1] * matrix2[1][0] + matrix1[2][2] * matrix2[2][0];
	target_matrix[2][1] = matrix1[2][0] * matrix2[0][1] + matrix1[2][1] * matrix2[1][1] + matrix1[2][2] * matrix2[2][1];
	target_matrix[2][2] = matrix1[2][0] * matrix2[0][2] + matrix1[2][1] * matrix2[1][2] + matrix2[2][2] * matrix2[2][2];
}

void matmul3x3w1x3 (float matrix[3][3], float vec[3], float target_vec[3]) {
	target_vec[0] = matrix[0][0] * vec[0] + matrix[0][1] * vec[1] + matrix[0][2] * vec[2];
	target_vec[1] = matrix[1][0] * vec[0] + matrix[1][1] * vec[1] + matrix[1][2] * vec[2];
	target_vec[2] = matrix[2][0] * vec[0] + matrix[2][1] * vec[1] + matrix[2][2] * vec[2];
}

void copy_vector (float vec[3], float target_vec[3]) {
	target_vec[0] = vec[0];
	target_vec[1] = vec[1];
	target_vec[2] = vec[2];
}

void cross (float vec1[3], float vec2[3], float target_vec[3]) {
	target_vec[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	target_vec[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	target_vec[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

float dot (float vec1[3], float vec2[3]) {
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

void normalize (float vec[3]) {
	float length = sqrt(dot(vec, vec));
	vec[0] /= length;
	vec[1] /= length;
	vec[2] /= length;
}

void transform_batch (Object *object, float target_points[][3]) {
	float generated_rotation_matrix_z[3][3] = {0};
	rotation_matrix_z(generated_rotation_matrix_z, (object->rotationZX)[0]);

	float generated_rotation_matrix_x[3][3] = {0};
	rotation_matrix_x(generated_rotation_matrix_x, (object->rotationZX)[1]);

	float total_rotation_matrix[3][3] = {0};
	matmul3x3w3x3(generated_rotation_matrix_z, generated_rotation_matrix_x, total_rotation_matrix);

	for (int i = 0; i < object->num_points; i++) {
		float temp_vector[3] = {0};
		matmul3x3w1x3(total_rotation_matrix, (object->points)[i], temp_vector);
		add(temp_vector, object->position);

		copy_vector(temp_vector, target_points[i]);
	}
}

void project_batch (float points[][3], int n, Object *camera, Screen *screen, float target_points[][3]) {
	float generated_rotation_matrix_z[3][3] = {0};
	rotation_matrix_z(generated_rotation_matrix_z, -(camera->rotationZX)[0]);

	float generated_rotation_matrix_x[3][3] = {0};
	rotation_matrix_x(generated_rotation_matrix_x, -(camera->rotationZX)[1]);

	float total_rotation_matrix[3][3] = {0};
	matmul3x3w3x3(generated_rotation_matrix_z, generated_rotation_matrix_x, total_rotation_matrix);

	float camera_inverse_displacement[3] = {0};
	copy_vector(camera->position, camera_inverse_displacement);
	mult(camera_inverse_displacement, -1.0f);

	int j = 0;
	for (int i = 0; i < n; i++) {
		float temp_vector[3] = {0};
		copy_vector(points[i], temp_vector);
		add(temp_vector, camera_inverse_displacement);

		float temp_vector2[3] = {0};
		matmul3x3w1x3(total_rotation_matrix, temp_vector, temp_vector2);
		copy_vector(temp_vector2, temp_vector);

		temp_vector[0] /= temp_vector[1] * NEAR;
		temp_vector[2] /= temp_vector[1] * NEAR;
		
		if (fabs(temp_vector[0]) <= screen->halfwidth && fabs(temp_vector[2] <= screen->halfheight)) {
			copy_vector(temp_vector, target_points[j]);
			j++;
		}
	}
}

void directional_lighting_batch (float points[][3], int faces[][3], int num_faces, Object *light, Object *camera, float target_illumination[]) {
	for (int i = 0; i < num_faces; i++) {
		float diff1[3];
		float diff2[3];
		copy_vector(points[faces[i][0]], diff1);
		copy_vector(points[faces[i][2]], diff2);
		sub(diff1, points[faces[i][1]]);
		sub(diff2, points[faces[i][1]]);
		float cross_product[3];
		cross(diff1, diff2, cross_product);
		normalize(cross_product);

		float diffuse = fmaxf(-dot(cross_product, light->position), 0);
		target_illumination[i] = diffuse;
		
		float average[3] = {
			(points[faces[i][0]][0] + points[faces[i][1]][0] + points[faces[i][2]][0]) / 3.0f,
			(points[faces[i][0]][1] + points[faces[i][1]][1] + points[faces[i][2]][1]) / 3.0f,
			(points[faces[i][0]][2] + points[faces[i][1]][2] + points[faces[i][2]][2]) / 3.0f
		};
		float to_camera[3];
		copy_vector(camera->position, to_camera);
		sub(to_camera, average);
		float diff[3];
		copy_vector(to_camera, diff);
		sub(diff, cross_product);
		sub(to_camera, diff);
		sub(to_camera, diff);
		float specular = fmaxf(-dot(to_camera, light->position), 0);
		target_illumination[i] += specular;
	}
}

void final_illumination_batch (int num_faces, int num_illums, float illuminations[num_illums][num_faces], float final_illuminations[num_faces]) {
	for (int i = 0; i < num_faces; i++) {
		final_illuminations[i] = 0.0f;
		for (int j = 0; j < num_illums; j++) {
			final_illuminations[i] += illuminations[j][i];
		}
		final_illuminations[i] = (final_illuminations[i] >= 0) ? final_illuminations[i] : 0;
		final_illuminations[i] = final_illuminations[i] / (1 + final_illuminations[i]); //y = x / (1 + x)
	}
}

void give_sorted_order (float points[][3], int faces[][3], int num_faces, int order[], bool visible_faces[]) {
	float averages[num_faces];
	for (int i = 0; i < num_faces; i++) {
		averages[i] = (points[faces[i][0]][1] + points[faces[i][1]][1] + points[faces[i][2]][1]) / 3;
		order[i] = i;
	}
	for (int k = 1; k < num_faces; k++) {
		float temp = averages[k];
		int j = k - 1;
		while (j >= 0 && temp > averages[j]) {
			averages[j + 1] = averages[j];
			order[j + 1] = order[j];
			j--;
		}
		order[j + 1] = k;
		averages[j + 1] = temp;
	}
	
	for (int i = 0; i < num_faces; i++) {
		if (averages[i] < 1.5f) {
			visible_faces[i] = false;
		} else {
			visible_faces[i] = true;
		}
	}
}
