void savePassword(int user){	
	int tics = ticrate*passwordtime;
	int maxLineLength = ticrate*passwordTime;
	int index = 0;
	char newPassword[maxLineLength];	

	ofstream newPassFile;
	newPassFile.open("newPassword.txt");
	if(!newPassFile.is_open()){
		//log
	}
	
	for(int i=0; i<3; i++){
		for(int j=0; j<tics; j++){
			newPassFile << passwords[i][j];
		}
		newPassFile << "\n";
	}
	
	newPassFile.close();
}
