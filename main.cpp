#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ugpio/ugpio.h>
#include <relay-exp.h>
#include <iostream>
#include <fstream>

using namespace std;
//GLOBAL VARIABLES-------------------------
const int ticrate = 20;
const int passwordtime = 2;
const int addr = 7;	
const int passworderrorthreshold = 5;

void check(bool inputpassword[], bool realpassword[], int readpin);
void getpassword(int readpin, bool realpassword[]);
int geterror(bool inputpassword[] ,bool realpassword[]);
void readPassword(bool* password);

int main(int argc, char **argv, char **envp){
	//INITAL VARIABLES-----------------------
	bool inputpassword[ticrate*passwordtime];
	bool realpassword[ticrate*passwordtime];
	bool locked = true;
	int gpioread, gpiowrite;
	
	gpiowrite = atoi(argv[1]);
	gpioread = atoi(argv[2]);

	//read realpassword from file
	readPassword(realpassword);
	//SETUP PINS-----------------------------	
	relayDriverInit(addr);

	gpio_request(gpiowrite, NULL);
	gpio_request(gpioread, NULL);

	gpio_direction_input(gpioread);
	gpio_direction_output(gpiowrite,0);
	bool val = 1;

	//MAIN LOOP-----------------------------
	while(true){
		if(locked){
			if(gpio_get_value(gpioread)){
				check(inputpassword, realpassword, gpioread);
			}
		}else{
			//check for hold down
			if(gpio_get_value(gpioread)){

			}
		}
		usleep(1000000/ticrate);
	}
	//FREE PINS AND CLOSE FILES------------
	gpio_free(gpioread);
	gpio_free(gpiowrite);

	return 0;
}

//check if password is right
void check(bool inputpassword[], bool realpassword[], int readpin){
	getpassword(readpin, inputpassword);
	int val = geterror(inputpassword,realpassword);

}

//retreive password from pressure sensor
void getpassword(int readpin, bool passwordinput[]){
	int tics = ticrate*passwordtime;
	int index = 0;
	while(index < tics){
		passwordinput[index] = gpio_get_value(readpin);
		usleep(1000000/ticrate);
		index++;
	}
}

//calculate error between inputted password and real password
int geterror(bool inputpassword[] ,bool realpassword[]){
	int tics = ticrate*passwordtime;
	int error = 0;
	while(tics--){
		error += !(inputpassword[tics] == realpassword[tics]);
	}	
	return error;
}

//get the real password from the file
void readPassword(bool password[]){
	bool done = false;
	char filename[] = {"password.txt"};
	int maxLineLength = ticrate*passwordtime;
	char line[maxLineLength];	
	ifstream passFile;
	passFile.open(filename);
	if(!passFile.is_open()){
		//log
	}	
	while(!done){ //Read from files
		if(!passFile.getline(line, maxLineLength+1)){ //Get next line
			done = true;
		}
		for(int i=0; i<maxLineLength; i++){
			password[i] = (bool)(line[i]-48);
		}
	}	
	passFile.close();
}
