/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \addtogroup realmd Realm Daemon
/// @{
/// \file

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "RealmList.h"

#include "Config/Config.h"
#include "Log.h"
#include "AuthSocket.h"
#include "SystemConfig.h"
#include "Version.h"
#include "Util.h"
#include <openssl/opensslv.h>
#include <openssl/crypto.h>

//#include <ace/ACE.h>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <sstream>

#include "SessionManager.h"

#ifdef WIN32
#include "ServiceWin32.h"
char serviceName[] = "realmd";
char serviceLongName[] = "MaNGOS realmd service";
char serviceDescription[] = "Massive Network Game Object Server";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#else
#include "PosixDaemon.h"
#endif

bool StartDB();
void UnhookSignals();
void HookSignals();

bool stopEvent = false;                                     ///< Setting it to true stops the server

DatabaseType LoginDatabase;                                 ///< Accessor to the realm server database

/// Print out the usage string for this program on the console.
void usage(boost::program_options::options_description const& desc, const char* prog)
{
    std::ostringstream ss;
    ss << desc;
    sLog.outString("Usage: \n %s [<options>]\n%s", prog, ss.str().c_str());
}

/// Launch the realm server
extern int main(int argc, char** argv)
{
    ///- Command line parsing
    std::string cfg_file;
    std::string serviceDaemonMode;

    boost::program_options::options_description description("Allowed options");
    description.add_options()
        ("version,v", "print version and exit")
        ("help,h", "print commandline help and exit")
        ("config,c", boost::program_options::value<std::string>(&cfg_file)->default_value(_REALMD_CONFIG), "use as configuration file")
#ifdef WIN32
        ("service,s", boost::program_options::value<std::string>(&serviceDaemonMode), "running as service, arg functions: run, install, uninstall")
#else
        ("service,s", boost::program_options::value<std::string>(&serviceDaemonMode), "running as daemon, arg functions: run, stop")
#endif
        ;

    // parse option
    boost::program_options::variables_map vm;

    try {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
            options(description).run(), vm);
        boost::program_options::notify(vm);
    }
    catch(boost::program_options::unknown_option const& ex)
    {
        sLog.outError("Runtime-Error: unknown option %s", ex.get_option_name().c_str());
        usage(description, argv[0]);
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }
    catch(boost::program_options::invalid_command_line_syntax const& ex)
    {
        sLog.outError("Runtime-Error: invalid syntax for option %s", ex.get_option_name().c_str());
        usage(description, argv[0]);
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    if (vm.count("version"))
    {
        return 0;
    }

    if (vm.count("help"))
    {
        usage(description, argv[0]);
        return 0;
    }

    if (!serviceDaemonMode.empty())
    {
#ifdef WIN32
        char const* const serviceModes[] =  { "run", "install", "uninstall", NULL };
#else
        char const* const serviceModes[] =  { "run", "stop", NULL };
#endif
        char const* const* mode_ptr = &serviceModes[0];
        for(; *mode_ptr != NULL; ++mode_ptr)
            if (*mode_ptr == serviceDaemonMode)
                break;

        if (!*mode_ptr)
        {
            sLog.outError("Runtime-Error: -s unsupported argument %s", serviceDaemonMode.c_str());
            usage(description, argv[0]);
            Log::WaitBeforeContinueIfNeed();
            return 1;
        }
    }

#ifdef WIN32                                                // windows service command need execute before config read
    switch (serviceDaemonMode[0])
    {
    case 'i':
        if (WinServiceInstall())
            sLog.outString("Installing service");
        return 1;
    case 'u':
        if (WinServiceUninstall())
            sLog.outString("Uninstalling service");
        return 1;
    case 'r':
        WinServiceRun();
        break;
    }
#endif

    if (!sConfig.SetSource(cfg_file, "RealmdConf"))
    {
        sLog.outError("Could not find configuration file %s.", cfg_file);
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

#ifndef WIN32                                               // posix daemon commands need apply after config read
    switch (serviceDaemonMode)
    {
        case 'r':
            startDaemon();
            break;
        case 's':
            stopDaemon();
            break;
    }
#endif

    sLog.Initialize();

    sLog.outString("<Ctrl-C> to stop.\n");
    sLog.outString("Using configuration file %s.", cfg_file);

    ///- Check the version of the configuration file
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION)
    {
        sLog.outError("*****************************************************************************");
        sLog.outError(" WARNING: Your realmd.conf version indicates your conf file is out of date!");
        sLog.outError("          Please check for updates, as your current default values may cause");
        sLog.outError("          strange behavior.");
        sLog.outError("*****************************************************************************");
        Log::WaitBeforeContinueIfNeed();
    }

    DETAIL_LOG("%s (Library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    if (SSLeay() < 0x009080bfL)
    {
        DETAIL_LOG("WARNING: Outdated version of OpenSSL lib. Logins to server may not work!");
        DETAIL_LOG("WARNING: Minimal required version [OpenSSL 0.9.8k]");
    }

    //DETAIL_LOG("Using ACE: %s", ACE_VERSION);
    DETAIL_LOG("Using BOOST: %i.%i.%i", BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);

    sLog.outBasic("Max allowed open files is %d", boost::asio::socket_base::max_connections);

    /// realmd PID file creation
    std::string pidfile = sConfig.GetStringDefault("PidFile", "");
    if (!pidfile.empty())
    {
        uint32 pid = CreatePIDFile(pidfile);
        if (!pid)
        {
            sLog.outError("Cannot create PID file %s.\n", pidfile.c_str());
            Log::WaitBeforeContinueIfNeed();
            return 1;
        }

        sLog.outString("Daemon PID: %u\n", pid);
    }

    ///- Initialize the database connection
    //if (!StartDB())
    //{
    //    Log::WaitBeforeContinueIfNeed();
    //    return 1;
    //}

    ///- Get the list of realms for the server
    //sRealmList.Initialize(sConfig.GetIntDefault("RealmsStateUpdateDelay", 20));
    //if (sRealmList.size() == 0)
    //{
    //    sLog.outError("No valid realms specified.");
    //    Log::WaitBeforeContinueIfNeed();
    //    return 1;
    //}

    // cleanup query
    // set expired bans to inactive
    //LoginDatabase.BeginTransaction();
    //LoginDatabase.Execute("UPDATE account_banned SET active = 0 WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");
    //LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");
    //LoginDatabase.CommitTransaction();

    ///- Launch the listening network socket

    uint16 rmport = sConfig.GetIntDefault("RealmServerPort", DEFAULT_REALMSERVER_PORT);
    std::string bind_ip = sConfig.GetStringDefault("BindIP", "0.0.0.0");

    std::auto_ptr< SessionManager > manager( new SessionManager() );
    if ( !manager->StartNetwork( rmport, bind_ip ))
    {
        sLog.outError("MaNGOS realmd can not bind to %s:%d", bind_ip.c_str(), rmport);
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    ///- Catch termination signals
    HookSignals();

    ///- Handle affinity for multiple processors and process priority on Windows
#ifdef WIN32
    {
        HANDLE hProcess = GetCurrentProcess();

        uint32 Aff = sConfig.GetIntDefault("UseProcessors", 0);
        if (Aff > 0)
        {
            ULONG_PTR appAff;
            ULONG_PTR sysAff;

            if (GetProcessAffinityMask(hProcess, &appAff, &sysAff))
            {
                ULONG_PTR curAff = Aff & appAff;            // remove non accessible processors

                if (!curAff)
                {
                    sLog.outError("Processors marked in UseProcessors bitmask (hex) %x not accessible for realmd. Accessible processors bitmask (hex): %x", Aff, appAff);
                }
                else
                {
                    if (SetProcessAffinityMask(hProcess, curAff))
                        sLog.outString("Using processors (bitmask, hex): %x", curAff);
                    else
                        sLog.outError("Can't set used processors (hex): %x", curAff);
                }
            }
            sLog.outString();
        }

        bool Prio = sConfig.GetBoolDefault("ProcessPriority", false);

        if (Prio)
        {
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                sLog.outString("realmd process priority class set to HIGH");
            else
                sLog.outError("Can't set realmd process priority class.");
            sLog.outString();
        }
    }
#endif

    // server has started up successfully => enable async DB requests
    LoginDatabase.AllowAsyncTransactions();

    // maximum counter for next ping
    uint32 numLoops = (sConfig.GetIntDefault("MaxPingTime", 30) * (MINUTE * 1000000 / 100000));
    uint32 loopCounter = 0;

#ifndef WIN32
    detachDaemon();
#endif
    ///- Wait for termination signal
    while (!stopEvent)
    {
        // dont move this outside the loop, the reactor will modify it
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));

        if ((++loopCounter) == numLoops)
        {
            loopCounter = 0;
            DETAIL_LOG("Ping MySQL to keep connection alive");
            LoginDatabase.Ping();
        }
#ifdef WIN32
        if (m_ServiceStatus == 0) stopEvent = true;
        while (m_ServiceStatus == 2) Sleep(1000);
#endif
    }

    manager->StopNetwork();
    manager.reset();

    ///- Wait for the delay thread to exit
    LoginDatabase.HaltDelayThread();

    ///- Remove signal handling before leaving
    UnhookSignals();

    sLog.outString("Halting process...");
    return 0;
}

/// Handle termination signals
/** Put the global variable stopEvent to 'true' if a termination signal is caught **/
void OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGTERM:
            stopEvent = true;
            break;
#ifdef _WIN32
        case SIGBREAK:
            stopEvent = true;
            break;
#endif
    }

    signal(s, OnSignal);
}

/// Initialize connection to the database
bool StartDB()
{
    std::string dbstring = sConfig.GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        sLog.outError("Database not specified");
        return false;
    }

    sLog.outString("Login Database total connections: %i", 1 + 1);

    if (!LoginDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
        return false;
    }

	if (!LoginDatabase.CheckRequiredField("realmd_db_version", MANGOS_DB_VERSION_LIB_VERSION))
    {
        ///- Wait for already started DB delay threads to end
        LoginDatabase.HaltDelayThread();
        return false;
    }

    return true;
}

/// Define hook 'OnSignal' for all termination signals
void HookSignals()
{
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, OnSignal);
#endif
}

/// Unhook the signals before leaving
void UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
#ifdef _WIN32
    signal(SIGBREAK, 0);
#endif
}

/// @}
