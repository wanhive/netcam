/*
 * ClientManager.h
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

#ifndef CLIENT_CLIENTMANAGER_H_
#define CLIENT_CLIENTMANAGER_H_
#include <wanhive/wanhive.h>
#include <cstdio>

namespace wanhive {
/*
 * Real stuff happens here
 */
class ClientManager {
public:
	ClientManager() noexcept;
	~ClientManager();
	static void execute(int argc, char *const*argv) noexcept;
private:
	static void printHelp(FILE *stream) noexcept;
	static int parseOptions(int argc, char *const*argv) noexcept;
	static void processOptions() noexcept;
	static void executeHub() noexcept;
	static void f() noexcept;
	//-----------------------------------------------------------------
	static void installSignals();
	static void shutdown(int signum) noexcept;
private:
	static const char *programName;
	static bool menu;
	static char hubType;
	static unsigned long long hubId;
	static const char *configPath;
	static Hub *hub;
};

} /* namespace wanhive */

#endif /* CLIENT_CLIENTMANAGER_H_ */
