#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <ugpio/ugpio.h>
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
		newPassword[index] = gpio_get_value(inputPin);				
		usleep(1000000/ticRate);
		index++;
	}
	
	newPassFile << newPassword;			
	newPassFile.close();		


}

int main(int argc, char **argv, char **envp){
	inputPin = atio(argv[1]);
	while(true){
		if(gpio_get_value(inputPin)){	
			savePassword();
		}
		usleep(1000000/ticRate);
	}
	return 0;
}
