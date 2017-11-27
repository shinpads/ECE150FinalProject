#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ugpio/ugpio.h>
#include <relay-exp.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>

using namespace std;
//GLOBAL VARIABLES-------------------------
const int ticrate = 40;
const int passwordtime = 2;
const int relayAddress = 7;	
const int passworderrorthreshold = 15;
bool inputpassword[ticrate*passwordtime+1];
bool realpassword[ticrate*passwordtime+1];
ofstream logfile;
bool locked;
//FUNCTION SIGNATURES------------------------
void check(int readpin);
void checkForOnes(int readpin);
void getpassword(int readpin);
int geterror();
void readPassword();
void toggleLock();
string getTime();
void log(string x);
int main(int argc, char **argv, char **envp){		
	locked = true;
	//start log file
	logfile.open("logfile.txt");
	//read input for pin numbers
	int gpiowrite = atoi(argv[1]);
	int gpioread = atoi(argv[2]);
	//read realpassword from file
	readPassword();
	//SETUP PINS-----------------------------

	//setup relay	
	relayDriverInit(relayAddress);

	//request pins to be used
	gpio_request(gpiowrite, NULL);
	gpio_request(gpioread, NULL);

	//set pin directions to input or output
	gpio_direction_input(gpioread);
	gpio_direction_output(gpiowrite,0);



	//MAIN LOOP-----------------------------
	while(true){
		//check if there are any knocks
		if(gpio_get_value(gpioread)){
			//if knocks detetected, and door is locked, check if knock pattern matches
			if(locked){
					log("INFO: Starting to record user input.");				
					check(gpioread);
					string inputie = "INFO: Recorded Input: ";
					for(int i = 0; i<ticrate*passwordtime; i++){
						if(inputpassword[i]){
							inputie += "1";
						}else{inputie+="0";}
					}
					cout << inputie;
					log(inputie);			
			}
			//if door is unlocked, then check if pressure is held for 2 seconds
			else{
				//check for hold down
					checkForOnes(gpioread);
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
void check(int readpin){
	//read knock pattern into inputpassword array
	getpassword(readpin);
	//calculate error amount between realpassword and inputpassword
	int val = geterror();
	string valstr = to_string(val);	
	log("INFO: erorr value: " + valstr);
	//check if error value is low enough to be considered same knocking pattern
	if(val <= passworderrorthreshold){
		log("INFO: Password matched.");
		//toggle the door lock if pattern matches
		toggleLock();
	}
	else{
		log("INFO: Password does not match.");
	}
	

}
//checking if pressure sensor is held down for 2 seconds
void checkForOnes(int readpin){
	bool shouldLock = true;
	getpassword(readpin);
	for(int i=0; i<(ticrate*passwordtime) && shouldLock; i++){
		if(!inputpassword[i]){
			shouldLock = false;
		}
	}
	//if held down for 2 seconds, toggle the lock
	if(shouldLock){
		toggleLock();
	}
}

//retreive password from pressure sensor
void getpassword(int readpin){
	int tics = ticrate*passwordtime;
	int index = 0;
	//read pressure sensor for 2 seconds at a rate of ticrate
	while(index < tics){
		//save inputted password to inputpassword array
		inputpassword[index] = gpio_get_value(readpin);
		usleep(1000000/ticrate);
		index++;
	}
}

//calculate error between inputted password and real password
int geterror(){
	int tics = ticrate*passwordtime;
	int error = 0;
	int index = 0;
	//error is calulated as sum of indexes that dont match between inputpassword and realpassword
	while(index < tics){
		if (inputpassword[index] != realpassword[index]){
			error++;
		}
		index++;
	}	
	
	return error;
}

//get the real password from the file
void readPassword(){
	bool done = false;
	char filename[] = {"newPassword.txt"};
	int maxLineLength = ticrate*passwordtime;
	string line;
	ifstream passFile;
	passFile.open(filename);
	getline(passFile,line);
	//read realpassword from newPasword.txt (file created from savepassword.cpp) and save it into realpassword array
	for(int i=0; i<maxLineLength; i++){
		realpassword[i] = (bool)(line.at(i)-48);
	}
		
	passFile.close();
	log(line);
}
//getTime function for the log file
string getTime(){
	time_t rawTime; //type allows the representation of time
	struct tm* timeFormatted; //structure the time into sec, min, hour...
	time(&rawTime); //Get time and set the argument to that value
	timeFormatted = localtime(&rawTime); //convert to our timezone
	string convert = ((string)(asctime(timeFormatted)));//convert tm to string
	convert = convert.substr(0, convert.size()-1); // removes the /n from the string 
	return convert;
}
//write strring to the log file
void log(string x){
	logfile << getTime() +" "+ x +"\n";
}
void toggleLock(){
	log("INFO: Opening/Closing door.");
	//Spin the motor for one second
	relaySetChannel(relayAddress, 0, 1);
	sleep(1);
	relaySetChannel(relayAddress, 0, 0);
	//change locked state
	locked = !locked;
}
