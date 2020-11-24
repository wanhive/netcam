//============================================================================
// Name        : wanhive-netcam.cpp
// Author      : Wanhive Systems Private Limited (info@wanhive.com)
// Version     :
// Copyright   : Copyright 2020 Wanhive Systems Private Limited
// License     : GPL-3.0-or-later
// Description : The main function
//============================================================================

#include "client/ClientManager.h"
#include <iostream>

int main(int argc, char *argv[]) {
	wanhive::ClientManager::execute(argc,argv);
	return 0;
}
