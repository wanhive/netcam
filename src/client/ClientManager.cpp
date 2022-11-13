/*
 * ClientManager.cpp
 *
 * Copyright (C) 2020 Wanhive Systems Private Limited (info@wanhive.com)
 * This file is part of Wanhive Netcam.
 *
 * Wanhive Netcam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wanhive Netcam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wanhive Netcam. If not, see <https://www.gnu.org/licenses/>.
 *
 */

/*
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Apache 2.0 License
 *
 * Copyright (C) 2020 Wanhive Systems Private Limited (info@wanhive.com)
 * This program is part of Wanhive IoT Platform.
 * Check the COPYING file for the license.
 */

#include "ClientManager.h"
#ifdef WH_WITHOUT_STREAMER
#undef WH_WITH_STREAMER
#else
#define WH_WITH_STREAMER
#endif
#ifdef WH_WITH_STREAMER
#include "Streamer.h"
#endif
#include "Viewer.h"
#include <getopt.h>
#include <iostream>
#include <limits>

#define WH_PRODUCT_NAME "Wanhive Netcam"

//VERSION PATTERN: https://semver.org/spec/v2.0.0.html
#define WH_RELEASE_VERSION "0.4.0"
#define WH_RELEASE_NAME ""
#define WH_RELEASE_YEAR "2022"
#define WH_RELEASE_AUTHOR "Wanhive Systems Private Limited"
#define WH_RELEASE_EMAIL "info@wanhive.com"
#define WH_RELEASE_WEBSITE "www.wanhive.com"
#define WH_LICENSE_TEXT "GPL-3.0-or-later http://www.gnu.org/licenses/"

namespace wanhive {

const char *ClientManager::programName = nullptr;
bool ClientManager::menu = false;
unsigned long long ClientManager::hubId = (unsigned long long) -1;
char ClientManager::hubType = '\0';
const char *ClientManager::configPath = nullptr;
Hub *ClientManager::hub = nullptr;

ClientManager::ClientManager() noexcept {

}

ClientManager::~ClientManager() {
}

void ClientManager::execute(int argc, char *const*argv) noexcept {
	if (parseOptions(argc, argv) == 0) {
		processOptions();
	}
}

void ClientManager::printHelp(FILE *stream) noexcept {
	fprintf(stream, "\n%s %s version %s\nCopyright \u00A9 %s %s.\n",
	WH_PRODUCT_NAME, WH_RELEASE_NAME, WH_RELEASE_VERSION,
	WH_RELEASE_YEAR, WH_RELEASE_AUTHOR);

	fprintf(stderr, "LICENSE %s\n\n", WH_LICENSE_TEXT);

	fprintf(stream, "Usage: %s [OPTIONS]\n", programName);
	fprintf(stream, "OPTIONS\n");
	fprintf(stream,
			"-c --config <path>      \tPathname of configuration file.\n");
	fprintf(stream, "-h --help               \tDisplay this information.\n");
	fprintf(stream,
			"-m --menu               \tDisplay menu of available options.\n");
	fprintf(stream, "-n --name   <identifier>\tSet hub's identifier.\n");
	fprintf(stream, "-t --type   <type>      \tSet hub's type.\n");

	fprintf(stream, "\n%s requires an external configuration file.\n"
			"If none is supplied via the command line then the program will\n"
			"try to read '%s' from the 'current working directory',\n"
			"the 'executable directory', %s, or\n"
			"%s in that order.\n\n", WH_PRODUCT_NAME, Identity::CONF_FILE_NAME,
			Identity::CONF_PATH, Identity::CONF_SYSTEM_PATH);

	fprintf(stream, "\nwebsite: %s   email: %s\n\n",
	WH_RELEASE_WEBSITE, WH_RELEASE_EMAIL);
}

int ClientManager::parseOptions(int argc, char *const*argv) noexcept {
	programName = nullptr;
	menu = false;
	hubId = (unsigned long long) -1;
	hubType = '\0';
	configPath = nullptr;
	hub = nullptr;
	//-----------------------------------------------------------------
	programName = strrchr(argv[0], Storage::DIR_SEPARATOR);
	programName = programName ? (programName + 1) : argv[0];
	//-----------------------------------------------------------------
	const char *shortOptions = "c:hmn:t:";
	const struct option longOptions[] = { { "config", 1, nullptr, 'c' }, {
			"help", 0, nullptr, 'h' }, { "menu", 0, nullptr, 'm' }, { "name", 1,
			nullptr, 'n' }, { "type", 1, nullptr, 't' }, { nullptr, 0, nullptr,
			0 } };
	//-----------------------------------------------------------------
	int nextOption;
	do {
		nextOption = getopt_long(argc, argv, shortOptions, longOptions,
				(int*) nullptr);
		switch (nextOption) {
		case 'c':
			configPath = optarg;
			break;
		case 'h':
			printHelp(stderr);
			return 1;
		case 'm':
			menu = true;
			break;
		case 'n':
			sscanf(optarg, "%llu", &hubId);
			break;
		case 't':
			sscanf(optarg, "%c", &hubType);
			break;
		case -1:	//We are done
			break;
		case '?': //Unknown option
			WH_LOG_ERROR("Invalid option");
			printHelp(stderr);
			return -1;
		default:
			WH_LOG_ERROR("?? read character code 0%o ??", nextOption);
			printHelp(stderr);
			return -1;
		}
	} while (nextOption != -1);
	//-----------------------------------------------------------------
	return 0;
}

void ClientManager::processOptions() noexcept {
	int option = 1; //Default option
	if (menu) {
		std::cout << "Select an option\n" << "1. NETCAM APPLICATION\n"
				<< "2. HELP\n" << "::";
		std::cin >> option;
		if (CommandLine::inputError()) {
			return;
		}
	}

	switch (option) {
	case 1:
#ifndef WH_WITH_STREAMER
		hubType = 'v';
#endif
		executeHub();
		break;
	case 2:
		f();
		break;
	default:
		std::cerr << "Invalid option" << std::endl;
		break;
	}
}

void ClientManager::executeHub() noexcept {
	hub = nullptr;
	unsigned int mode = 0;
	if (hubType == 's') {
		mode = 1;
	} else if (hubType == 'v') {
		mode = 2;
	} else if (hubType == '\0') {
		std::cout << "Select an option\n" << "1: Streamer (-ts)\n"
				<< "2: Viewer (-tv)\n" << ":: ";
		std::cin >> mode;
		if (CommandLine::inputError()) {
			return;
		}
	} else {
		std::cerr << "Invalid option" << std::endl;
		return;
	}

	if (hubId == (unsigned long long) -1) {
		std::cout << "Enter hub's identifier: ";
		std::cin >> hubId;
		if (CommandLine::inputError()) {
			return;
		}
	}

	try {
		if (mode == 1) {
#ifdef WH_WITH_STREAMER
			hub = new Streamer(hubId, configPath);
#else
			std::cerr << "Invalid option" << std::endl;
			return;
#endif
		} else if (mode == 2) {
			unsigned long long streamerId;
			std::cout << "Enter streamer's identifier: ";
			std::cin >> streamerId;
			if (CommandLine::inputError()) {
				return;
			}
			hub = new Viewer(hubId, streamerId, configPath);
		} else {
			std::cerr << "Invalid option" << std::endl;
			return;
		}

		/*
		 * No race condition because signals have been blocked before
		 * calling Hub::execute
		 */
		installSignals();
		if (hub->execute(nullptr)) {
			WH_LOG_INFO("Hub was terminated normally");
		} else {
			WH_LOG_ERROR("Hub was terminated due to error");
		}
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
	} catch (...) {
		WH_LOG_EXCEPTION_U();
	}
	delete hub;
	hub = nullptr;
}

void ClientManager::f() noexcept {
	printHelp(stderr);
}

void ClientManager::installSignals() {
	//Block all signals
	Signal::blockAll();
	//Suppress SIGPIPE
	Signal::ignore(SIGPIPE);
	//Install a dummy handler for SIGUSR1
	Signal::handle(SIGUSR1);
	//Following signals will initiate graceful shutdown
	Signal::handle(SIGINT, shutdown);
	Signal::handle(SIGTERM, shutdown);
	Signal::handle(SIGQUIT, shutdown);
	//SIGTSTP and SIGHUP not handled
	//Rest of the signals not handled
}

void ClientManager::shutdown(int signum) noexcept {
	hub->cancel();
}

} /* namespace wanhive */
