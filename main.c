/*
 * main.c
 *
 *  Created on: Mar 2023
 *      Author: Ruben Agustin
 */
#include "main.h"
#define MAIN_H
#include "color_sensor.h"
#include "accelerometer.h"

#define WRITE_BUF_SIZE	1500

void start_menu();
void start_accelerometer();
void start_color_sensor();


void toggle_value(uint8_t *value);

// Thread stuff
pthread_t thread_color_sensor, thread_acc;
int thread_error;
uint8_t color_sensor_alive, accelerometer_alive;

uint8_t color_sensor_data_ready = 0;
uint8_t acc_data_ready = 0;

uint8_t flash = 0;

char color_sensor_msg[1500] = "";

float ax, ay, az;       // Acceleration values

// Represent data
void init_signals();
void restore_signals();
struct termios original_termios = {0}, modified_termios = {0};


void sigint_handler(int signum) {
	printf("Exiting, wait till everything is closed\n");
	if(accelerometer_alive) accelerometer_alive = 0;
	if(color_sensor_alive)	color_sensor_alive = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
	sleep(5);
    exit(1);
}

int main(){
	// Input stuff
	char c = 0;	// keyboard input
	int result;	// error check

	init_signals();

	signal(SIGINT, sigint_handler);

	start_menu();

	while(1){
		result = read(STDIN_FILENO, &c, 1);

		if(result > 0 && c == '1') { 	// Start/stop accelerometer
			c = 0;
			if(!accelerometer_alive){
				printf("Accelerometer starts\n");
				fflush(stdout);
				thread_error = pthread_create(&thread_color_sensor, NULL, &start_accelerometer, NULL);
				accelerometer_alive = 1;
			} else{		// Turn off accelerometer
				accelerometer_alive = 0; //kill(accelerometer_pid, SIGINT);
			}
//			toggle_value(&accelerometer_alive);
		}else if(result > 0 && c == '2') { 	// Start/stop color_sensor
			c = 0;
			if(!color_sensor_alive){
				printf("Color sensor starts\n");
				fflush(stdout);
				thread_error = pthread_create(&thread_acc, NULL, &start_color_sensor, NULL);
				color_sensor_alive = 1;
			}else {
				color_sensor_alive = 0;
			}
//			toggle_value(&color_sensor_alive);
		} if(result > 0 && c == 'f') {
			// Toggle flash from color sensor
			flash = 1;
			printf("FLASH\n");
		}
		// Represent data
		if(accelerometer_alive && color_sensor_alive){
			if(acc_data_ready && color_sensor_data_ready){
				printf("\n\nAcceleration: x = %.2f, y = %.2f, z = %.2f\n\n", ax, ay, az);
				fflush(stdout);
				sleep(1);
//					printf("%s", color_sensor_msg);
			}
		}
		if(accelerometer_alive && !color_sensor_alive){
			if(acc_data_ready){
				printf("Acceleration: x = %.2f, y = %.2f, z = %.2f\r", ax, ay, az);
				fflush(stdout);
			}
		}
		if(color_sensor_alive && !accelerometer_alive){
			if(color_sensor_data_ready)
				printf("%s", color_sensor_msg);
			fflush(stdout);
			}
		}

	return 0;
}

void start_color_sensor(){
	colors();
}

void start_accelerometer(){
	acceleration();
}

void inline toggle_value(uint8_t *value){
	*value = !(*value);
}

void start_menu(){
	fflush(stdin);
	printf("***************** Sensor measurements by Ruben Agustin *****************\n\n");
	fflush(stdout);
	printf("-- Press 1 to start/disable accelerometer\n");
	fflush(stdout);
	printf("-- Press 2 to start/disable color sensor");
	fflush(stdout);
	printf("\nPress a key to start: ");
	fflush(stdout);
}

void init_signals(){
    // Save the original terminal settings
    tcgetattr(STDIN_FILENO, &original_termios);

    // Copy the original settings and modify them
    modified_termios = original_termios;
    modified_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    modified_termios.c_cc[VMIN] = 0;    // Set VMIN to 0 to read input without blocking
    modified_termios.c_cc[VTIME] = 0;   // Disable timeout

    // Set the modified terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &modified_termios);

    // Set stdin to non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flags);
}

void restore_signals(){
    // Set the modified terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}
