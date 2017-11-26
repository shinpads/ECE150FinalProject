#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ugpio/ugpio.h>
#include <relay-exp.h>
const int ticrate = 20;
const int passwordtime = 2;
const int addr = 7;	
int main(int argc, char **argv, char **envp){

	const int gpioread, gpiowrite;
	bool inputpassword[ticrate*passwordtime];
	relayDriverInit(addr);
	gpiowrite = atoi(argv[1]);
	gpioread = atoi(argv[2]);

	gpio_request(gpiowrite, NULL);
	gpio_request(gpioread, NULL);

	gpio_direction_input(gpioread);
	gpio_direction_output(gpiowrite,0);
	bool val = 1;

	while(true){
		if(gpio_get_value(gpioread)){
			relaySetChannel(addr,0,1);
		}else{
			relaySetChannel(addr,0,0);
		}

		usleep(1000000/ticrate);
	}

	gpio_free(gpioread);
	gpio_free(gpiowrite);

	return 0;
}


void getpassword(int readpin, &int passwordinput[]){
	int tics = ticrate*passwordtime;
	int index = 0
	while(index < tics){
		passwordinput[index] = gpio_get_value(readpin);
		usleep(1000000/ticrate);
		index++;
	}
}

int geterror(int inputpassword[] ,int realpassword[]){
	int tics = ticrate*passwordtime;
	int error = 0;
	while(tics--){
		error += !(inputpassword[tics] == realpassword[tics]);
	}	
	return error;
}
