#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "engine.c"
#include "reader.c"

const float SPEED = 0.01f;
const float TURN_SPEED = 0.01f;

bool w_down = false;
bool a_down = false;
bool s_down = false;
bool d_down = false;
bool left_down = false;
bool right_down = false;

void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		w_down = true;
	} else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		a_down = true;
	} else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		s_down = true;
	} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		d_down = true;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		left_down = true;
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		right_down = true;
	} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		w_down = false;
	} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		a_down = false;
	} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		s_down = false;
	} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		d_down = false;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		left_down = false;
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		right_down = false;
	}
}

int main (void) {
	Object triangle_obj = { .position = {0.0f, 1.0f, 0.0f}, .rotationZX = {0.0f, 0.0f} };
	read_obj("test.obj", &(triangle_obj.num_points), &(triangle_obj.num_faces), triangle_obj.points, triangle_obj.faces);

	Object camera = { .position = {0.0f, -1.0f, 0.5f}, .rotationZX = {0.0f, 0.0f} };
	Screen screen = { .halfwidth = 5, .halfheight = 5 };
	
	Object light = { .position = {0.5f, 0.5f, -0.5f} };
	normalize(light.position);

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(640, 480, "3D Engine", NULL, NULL);

	glfwMakeContextCurrent(window);
	
	glfwSwapInterval(60);
	
	glfwSetKeyCallback(window, key_callback);

	while (!glfwWindowShouldClose(window)) {
		triangle_obj.rotationZX[0] += 0.001f;
		
		if (w_down) {
			camera.position[0] += -SPEED * sin(camera.rotationZX[0]);
			camera.position[1] += SPEED * cos(camera.rotationZX[0]);
		}
		if (s_down) {
			camera.position[0] += SPEED * sin(camera.rotationZX[0]);
			camera.position[1] += -SPEED * cos(camera.rotationZX[0]);
		}
		if (a_down) {
			camera.position[0] += -SPEED * cos(camera.rotationZX[0]);
			camera.position[1] += -SPEED * sin(camera.rotationZX[0]);
		}
		if (d_down) {
			camera.position[0] += SPEED * cos(camera.rotationZX[0]);
			camera.position[1] += SPEED * sin(camera.rotationZX[0]);
		}
		if (left_down) {
			camera.rotationZX[0] += TURN_SPEED;
		}
		if (right_down) {
			camera.rotationZX[0] += -TURN_SPEED;
		}
		
		float transformed_points[triangle_obj.num_points][3];
		transform_batch(&triangle_obj, transformed_points);
		
		float directional_illuminations[triangle_obj.num_faces];
		directional_lighting_batch(transformed_points, triangle_obj.faces, triangle_obj.num_faces, &light, &camera, directional_illuminations);
		
		float illuminations[1][triangle_obj.num_faces];
		for (int i = 0; i < triangle_obj.num_faces; i++) {
			illuminations[0][i] = directional_illuminations[i];
		}
		float final_illuminations[triangle_obj.num_faces];
		final_illumination_batch(triangle_obj.num_faces, 1, illuminations, final_illuminations);

		float projected_points[triangle_obj.num_points][3];
		project_batch(transformed_points, triangle_obj.num_points, &camera, &screen, projected_points);
		
		int order[triangle_obj.num_faces];
		bool visible_faces[triangle_obj.num_faces];
		give_sorted_order(projected_points, triangle_obj.faces, triangle_obj.num_faces, order, visible_faces);

		glClear(GL_COLOR_BUFFER_BIT);
		
		for (int i = 0; i < triangle_obj.num_faces; i++) {
			int j = order[i];
			if (visible_faces[i]) {
				glBegin(GL_TRIANGLES);
					glColor3f(final_illuminations[j] + 0.1f, final_illuminations[j] + 0.1f, final_illuminations[j] + 0.1f);

					glVertex2f(projected_points[triangle_obj.faces[j][0]][0], projected_points[triangle_obj.faces[j][0]][2]);

					glVertex2f(projected_points[triangle_obj.faces[j][1]][0], projected_points[triangle_obj.faces[j][1]][2]);

					glVertex2f(projected_points[triangle_obj.faces[j][2]][0], projected_points[triangle_obj.faces[j][2]][2]);
				glEnd();
			}
		}

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
