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

const int ticRate = 20;
const int passwordTime = 2;
int inputPin;
void savePassword(){	
	bool done = false;
	int tics = ticRate*passwordTime;
	int maxLineLength = ticRate*passwordTime;
	int index = 0;
	char newPassword[maxLineLength];	

	ofstream newPassFile;
	newPassFile.open("newPassword.txt");
	if(!newPassFile.is_open()){
		//log
	}
	
	while(index<tics){
		if(gpio_get_value(inputPin)){
			newPassFile << '1';
		}
		else{
			newPassFile <<  '0';
		}			
		usleep(1000000/ticRate);
		index++;
	}	
		
	newPassFile.close();		


}

int main(int argc, char **argv, char **envp){
	inputPin = atoi(argv[1]);
	while(true){
		if(gpio_get_value(inputPin)){	
			savePassword();
			return 0;
		}

		usleep(1000000/ticRate);
	}
	return 0;
}
