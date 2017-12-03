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
const int passworderrorthreshold = 10;

int p1;
int p2;
int p3;
int switchpin;
int LEDpin;
bool inputpassword[ticrate*passwordtime+1];

bool passwords[3][ticrate*passwordtime+1];
//bool realpassword[ticrate*passwordtime+1];
ofstream logfile;
ofstream enterlog;
bool locked;
//FUNCTION SIGNATURES------------------------
int check(int readpin);
void checkForOnes(int readpin);
void getpassword(int readpin);
void savenewpassword(int readpin, int userid);
int geterror(int userid);
void readPassword();
void toggleLock();
void savepaswordstofile();
string getTime();
void log(string x);


int main(int argc, char **argv, char **envp)
{		
	locked = true;
	//start log file
	logfile.open("logfile.txt");
	enterlog.open("enterlog.txt");

	log("Starting program");

	//read input for pin numbers
	int gpioread = 3;
	p1 = 0; // pin 3 button 1
	p2 = 1; // pin 1 button 2
	p3 = 8; // pin 5 button 3
	switchpin = 18;

	//read realpassword from file
	readPassword();


	//SETUP PINS-----------------------------

	//setup relay	
	relayDriverInit(relayAddress);

	//request pins to be used

	gpio_request(gpioread, NULL);
	gpio_request(p1,NULL);
	gpio_request(p2,NULL);
	gpio_request(p3,NULL);
	gpio_request(switchpin,NULL);


	//set pin directions to input or output
	gpio_direction_input(gpioread);
	gpio_direction_input(p1);
	gpio_direction_input(p2);
	gpio_direction_input(p3);
	gpio_direction_input(switchpin);




	//MAIN LOOP-----------------------------
	while(true)
	{
		//check if there are any knocks

		if(gpio_get_value(switchpin))
		{
			savepaswordstofile();
			return 0;
		}
		if(!locked){
			if(gpio_get_value(p1))
			{
				cout << "Button 1 boiiii\n";
				savenewpassword(gpioread,0);
				continue;
			}
			else if(gpio_get_value(p2))
			{
				cout << "Button too boiiii\n";	
				savenewpassword(gpioread,1);
				continue;		
			}
			else if(gpio_get_value(p3))
			{
				cout << "Button III boiiii\n";
				savenewpassword(gpioread,2);
				continue;			
			}
			//if door is unlocked, then check if pressure is held for 2 seconds
			else if(gpio_get_value(gpioread))
			{
				//check for hold down
				checkForOnes(gpioread);
			}
		}
		else if(gpio_get_value(gpioread))
		{
			cout << "starting to record the inputie boii\n";
			//if knocks detetected, and door is locked, check if knock pattern matches
			if(locked)
			{
					log("INFO: Starting to record user input.");				
					int userthatjustopenedthedoor = check(gpioread);
					enterlog << "User #" << userthatjustopenedthedoor << " Has just unlocked the door at: "<<  getTime() << "\n"; 
					string inputie = "INFO: Recorded Input: ";
					for(int i = 0; i<ticrate*passwordtime; i++)
					{
						if(inputpassword[i])
						{
							inputie += "1";
						}
						else
						{
							inputie+="0";
						}
					}					
					log(inputie);	
					cout << inputie << endl;		
			}
		}

		usleep(1000000/ticrate);
	}
	//FREE PINS AND CLOSE FILES------------
	gpio_free(p1);
	gpio_free(p2);
	gpio_free(p3);
	gpio_free(switchpin);
	gpio_free(gpioread);


	return 0;
}

//check if password is right
int check(int readpin)
{
	log("Calling check()");
	//read knock pattern into inputpassword array
	getpassword(readpin);
	//calculate error amount between realpassword and inputpassword
	for(int user = 0; user < 3; user++)
	{
		int val = geterror(user);
		string valstr = to_string(val);	
		cout << "Erorr valuie for user#"<< user << ": " << valstr << " boiii\n";
		log("INFO: erorr value: " + valstr);
		//check if error value is low enough to be considered same knocking pattern
		if(val <= passworderrorthreshold)
		{
			log("INFO: Password matched.");
			//toggle the door lock if pattern matches
			toggleLock();
			return user;

		}		
		else
		{
			log("INFO: Password does not match.");
		}
	}
	

}
//checking if pressure sensor is held down for 2 seconds
void checkForOnes(int readpin)
{
	log("Checking to see if user wants to lock the door");
	bool shouldLock = true;
	getpassword(readpin);
	for(int i=0; i<(ticrate*passwordtime) && shouldLock; i++)
	{
		if(!inputpassword[i])
		{
			shouldLock = false;
		}
	}
	//if held down for 2 seconds, toggle the lock
	if(shouldLock)
	{
		toggleLock();
	}
}

//retreive password from pressure sensor
void getpassword(int readpin)
{
	log("Calling getpassord() function to record user input");
	int tics = ticrate*passwordtime;
	int index = 0;
	//read pressure sensor for 2 seconds at a rate of ticrate
	while(index < tics)
	{
		//save inputted password to inputpassword array
		inputpassword[index] = gpio_get_value(readpin);
		usleep(1000000/ticrate);
		index++;
	}
}

void savenewpassword(int readpin, int userid){
	log("Calling savenewpassword() function to update a users password");
	relaySetChannel(relayAddress, 1, 1);
	while(!gpio_get_value(readpin))
	{
		usleep(1000000/ticrate);
	}
	getpassword(readpin);
	for(int i = 0 ; i<ticrate*passwordtime; i++)
	{
		passwords[userid][i] = inputpassword[i];
	}
	for(int i=0; i<3; i++)
	{
		relaySetChannel(relayAddress, 1, 0);
		usleep(150000);
		relaySetChannel(relayAddress, 1, 1);
		usleep(150000);
	}
	relaySetChannel(relayAddress, 1, 0);
}
//calculate error between inputted password and real password
int geterror(int userid)
{
	log("Calling geterror()");
	int tics = ticrate*passwordtime;
	int error = 0;
	int index = 0;
	//error is calulated as sum of indexes that dont match between inputpassword and realpassword
	while(index < tics)
	{
		if (inputpassword[index] != passwords[userid][index])
		{
			error++;
		}
		index++;
	}	
	
	return error;
}

//get the real password from the file
void readPassword()
{
	log("Calling readPassword()");
	bool done = false;
	char filename[] = {"newPassword.txt"};
	int maxLineLength = ticrate*passwordtime;
	string line;
	ifstream passFile;
	passFile.open(filename);
	for(int usernum = 0; usernum < 3; usernum++)
	{
		getline(passFile,line);
		//read realpassword from newPasword.txt (file created from savepassword.cpp) and save it into realpassword array
		for(int i=0; i<maxLineLength; i++)
		{
			passwords[usernum][i] = (bool)(line.at(i)-48);
		}
		log(line);
	}
	passFile.close();

}
void savepaswordstofile(){
	log("Calling savepaswordtofile() function");
	int tics = ticrate*passwordtime;
	int maxLineLength = ticrate*passwordtime;
	int index = 0;
	char newPassword[maxLineLength];	

	ofstream newPassFile;
	newPassFile.open("newPassword.txt");
	if(!newPassFile.is_open())
	{
		//log
	}
	
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<tics; j++)
		{
			newPassFile << passwords[i][j];
		}
		newPassFile << "\n";
	}
	
	newPassFile.close();
}
//getTime function for the log file
string getTime()
{
	time_t rawTime; //type allows the representation of time
	struct tm* timeFormatted; //structure the time into sec, min, hour...
	time(&rawTime); //Get time and set the argument to that value
	timeFormatted = localtime(&rawTime); //convert to our timezone
	string convert = ((string)(asctime(timeFormatted)));//convert tm to string
	convert = convert.substr(0, convert.size()-1); // removes the /n from the string 
	return convert;
}
//write strring to the log file
void log(string x)
{
	logfile << getTime() +" "+ x +"\n";
}
void toggleLock()
{
	log("INFO: Opening/Closing door.");
	//Spin the motor for one second
	relaySetChannel(relayAddress, 0, 1);
	sleep(1);
	relaySetChannel(relayAddress, 0, 0);
	//change locked state
	locked = !locked;


}
