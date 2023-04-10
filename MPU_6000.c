#include <stdio.h>
#include <sys/ioctl.h>          // Both libraries are from Linux and is useful to manage I2C communications
#include <linux/i2c-dev.h>      //
#include <fcntl.h>              // To open file descriptors
#include <stdlib.h>             // To use exit (1) to end the program
#include <unistd.h>
#include <signal.h>             // To manage SIGTERM or SIGINT (Ctrl+C) signals
#include <stdint.h>             // To use int16_t
#include <math.h>
/** 
 * Authors: Hao Feng        Chen Fu
 *          Rubén           Agustín González
 * 
 * Date: 19/02/2023
 * 
 * Escape Sequences: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
 * References:       https://stackoverflow.com/questions/9974592/i2c-slave-ioctl-purpose
 * 
*/

#define ACCELERATION_RANGE 1    //2g, 4g, 8g or 16g (0, 1, 2 or 3)               pg 15 of registers       
#define I2C_ADDRESS 0x68        //with AD0 to 0, if AD0 is 1 then I2C is 0x69    pg 15 of datasheet 

#define ACCELERATION_SENSITIVITY  8192      // 2g -> 16384 LSB/g    4g -> 8192 LSB/g    8g -> 4096 LSB/g    16g -> 2048 LSB/g
                                        //   pg 26 of registers  
// Addresses of the registers (We have 16 bits of data that are distributed in 2 registers for each axis)         pg 7 of registers
#define REG_X_ADDR_HIGH 0x3B            // These values are in hexadecimal
#define REG_X_ADDR_LOW  0x3C
#define REG_Y_ADDR_HIGH 0x3D
#define REG_Y_ADDR_LOW  0x3E
#define REG_Z_ADDR_HIGH 0x3F
#define REG_Z_ADDR_LOW  0x40

// Offset of each axis
#define OFF_X 480
#define OFF_Y 66
#define OFF_Z 140

// Configuration of registers
#define ACCEL_CONFIG  0x1C      //pg 6 of registers
#define POWER_MANAGE  0x6B
// Functions
void SIGINT_Handler (int signal);

int Running = 1;

int main(){
    char values_axis[3][2];     // 3 axis and 2 bytes
    int16_t axis_raw[3];   // Values of each axis
    float axis_value[3];
    float x_angle, y_angle;
    int w_len = 2;

    char i2cFile[15] = "/dev/i2c-1";
    int fd_accelerometer = open(i2cFile, O_RDWR);               //Obtain file descriptor for RW
    ioctl(fd_accelerometer, I2C_SLAVE, I2C_ADDRESS);            // I assign the address of I2C Slave

    // What it happens when i can't open /dev/i2c-1 ????
    if (fd_accelerometer < 1){
        printf("ERROR: It's not possible open /dev/i2c-1 \n");
        exit(1);
    }
    // What it happens when i can't set I2C Slave ????
    if(ioctl(fd_accelerometer, I2C_SLAVE, I2C_ADDRESS) < 0){
        printf(" ERROR: It's not possible to set Slave address \n");
        exit(1);
    }
    
    printf("Setting SIGINT Handler...\n");
    signal(SIGINT, SIGINT_Handler);

    printf("Setting acceleration configuration...\n");

    char write_bytes[w_len];
    char write_byte;
     write_bytes[0] = POWER_MANAGE;     // Reset accelerometer
     write_bytes[1] = 0x00;
     write(fd_accelerometer, &write_bytes, 2); 

    write_bytes[0] = ACCEL_CONFIG;
    write_bytes[1] = 0x08;
    write(fd_accelerometer, &write_bytes, 2);
    
    // We need a loop to read accelerometer sensor every second
    printf("\033[?25l");          // Command to hide cursor
    printf("Reading Axis:\n");
    printf("\033[2E\n");
    while (Running == 1)
    {
        // Reading values of X axis
        write_byte = REG_X_ADDR_HIGH;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[0][1], 1);

        write_byte = REG_X_ADDR_LOW;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[0][0], 1);

        // Reading values of Y axis
        write_byte = REG_Y_ADDR_HIGH;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[1][1], 1);

        write_byte = REG_Y_ADDR_LOW;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[1][0], 1);

        // Reading values of Z axis
        write_byte = REG_Z_ADDR_HIGH;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[2][1], 1);

        write_byte = REG_Z_ADDR_LOW;
        write(fd_accelerometer, &write_byte, 1);
        read(fd_accelerometer, &values_axis[2][0], 1);

        axis_raw[0] = (values_axis[0][1] << 8) | (values_axis[0][0]);
        axis_raw[1] = (values_axis[1][1] << 8) | (values_axis[1][0]);
        axis_raw[2] = (values_axis[2][1] << 8) | (values_axis[2][0]);
        
        axis_value[0] = (float)(round(axis_raw[0] - OFF_X)/ACCELERATION_SENSITIVITY);
        axis_value[1] = (float)(round(axis_raw[1] - OFF_Y)/ACCELERATION_SENSITIVITY);
        axis_value[2] = (float)(round(axis_raw[2] - OFF_Z)/ACCELERATION_SENSITIVITY);
        
        printf("\033[38;5;119m\n");         // Set foreground
        printf("\033[4F");
        //printf("\033[2J");
        printf("\033[2K");       
        printf("X-Axis: %.2f \033[1E", axis_value[0]);
        printf("\033[2K");   
        printf("Y-Axis: %.2f \033[1E", axis_value[1]);
        printf("\033[2K");   
        printf("Z-Axis: %.2f \033[1E", axis_value[2]);

        sleep(1);        // It updates every 3 seconds
    }

    //Closing file descriptor
    printf("\033[38;5;4m\n");     // Set foreground

    printf("\033[2J");      // Erase the screen
    printf("\033[?25h");    // Make cursor visible
    printf("\033[H");       // Return to (0,0)
    //x_angle = atan2((double)axis_value[2],(double)axis_value[0]);
    //x_angle = atan2((double)axis_value[2],(double)axis_value[1]);
    //printf("Angle x: %.2f       Angle Y: %.2f", x_angle, y_angle);
    printf("Last values: X: %.2f    Y: %.2f    Z:  %.2f\n", axis_value[0], axis_value[1], axis_value[2]);
    close (fd_accelerometer);
    printf("Closing program\n");

    //printf("Angle x: %.2f       Angle Y: %.2f", x_angle, y_angle)
}

void SIGINT_Handler (int signal){
	Running = 0;        // End while loop
}
