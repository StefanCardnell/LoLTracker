/*

UNIX COMPILING:

Compiling using either static or dynamic linking:
    g++ -std=c++11 -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/ main.cpp functions.cpp -lcurl -lncursesw -ljsoncpp -lboost_program_options -o leagueTracker
-l will automatically use lib<name>.so files if it finds them, rather than the static .a counterparts. (Source: https://linux.die.net/man/1/ld)
By default libraries are looked for in /lib, /usr/lib and directories specified in /etc/ld.so.conf (look up how to specify directories in /etc/ld.so.conf)

To statically link jsoncpp, boost_program_options and other implicit standard library files:
    g++ -std=c++11 -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/ main.cpp functions.cpp -Wl,-Bstatic -ljsoncpp -lboost_program_options -Wl,-Bdynamic -lncurses -lcurl -static-libstdc++ -static-libgcc -o leagueTracker
-Wl,-Bdynamic and -Wl,-Bstatic enforce dynamic/static linking and prohibit static/dynamic linking respectively.

PI SETUP:

To run the program automatically on start-up, make sure the Pi logs in automatically and add this (or equivalent) to the end of ~/.bashrc:
    /home/pi/LeagueGame/leagueTracker

The PiTFT screen font may be too large to display the program. Run
    sudo dpkg-reconfigure console-setup
And select UTF8 -> Guess Optimal character set -> Terminus -> 6x12.

*/

#include <curses.h> //curses must be included first otherwise there are macro redefinition errors of MOUSE_MOVED. curl includes wincon.h which also defined MOUSE_MOVED
#include <curl/curl.h>
#include <json/json.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <iostream>
#include <string>
#include <exception> //for std::exception
#include <map>
#include <cstring> //needed for strlen
#include <chrono>
#include "functions.h"

using std::map; using std::string;
using std::istream; using std::cout; using std::endl;

const map<string, string> queues = {{"0", "Custom Game"}, {"8", "Normal 3v3"}, {"2", "Normal 5v5 Blind Pick"},
                                    {"14", "Normal 5v5 Draft Pick"}, {"9", "Ranked Flex (Twisted Treeline)"}, {"42", "Ranked Team 5v5"},
                                    {"16", "Dominion 5v5 Blind Pick"}, {"17", "Dominion 5v5 Draft Pick"}, {"25", "Dominion Coop vs AI"},
                                    {"31", "Coop vs AI Intro"}, {"32", "Coop vs AI Beginner"}, {"33", "Coop vs AI Intermediate"},
                                    {"52", "Twisted Treeline Coop vs AI"}, {"61", "Teambuilder"}, {"65", "ARAM"},
                                    {"70", "One for all"}, {"72", "Snowdown Showdown 1v1"}, {"73", "Snowdown Showdown 2v2"},
                                    {"75", "SR 6x6 Hexakill"}, {"76", "URF"}, {"83", "URF against AI"},
                                    {"91", "Doom Bots Rank 1"}, {"92", "Doom Bots Rank 2"}, {"93", "Doom Bots Rank 5"},
                                    {"96", "Ascension"}, {"98", "Twisted Treeline 6x6 Hexakill"}, {"100", "Butcher's Bridge ARAM"},
                                    {"300", "King Poro"}, {"310", "Nemesis Pick"}, {"313", "Black Market Brawlers"},
                                    {"315", "Nexus Siege"}, {"317", "Definitely Not Dominion"}, {"318", "ARURF"},
                                    {"400", "Normal 5v5 Draft Pick"}, {"420", "Ranked Solo (Summoner's Rift)"}, {"440", "Ranked Flex (Summoner's Rift)"}
                                   };

const map<string, string> serverlist = {{"na", "NA1"}, {"br", "BR1"}, {"lan", "LA1"},
                                        {"las", "LA2"}, {"ru", "RU"}, {"tr", "TR1"},
                                        {"eune", "EUN1"}, {"euw", "EUW1"}, {"kr", "KR"},
                                        {"oce", "OC1"}
                                       };

string curl_data;

int main(){


    //OBTAIN PROGRAM CONFIGURATIONS
    namespace po = boost::program_options;

    po::options_description options("Configuration Options");
    options.add_options()
        ("key", po::value<string>(), "API Key.")
        ("askInfo", po::value<bool>(), "Boolean indicating whether program should ask for name, server and display information.")
        ("name", po::value<string>(), "Name of the summoner to look up.")
        ("server", po::value<string>(), "Server of the summoner to look up.")
        ("showNames", po::value<bool>(), "Boolean indicating whether to display all summoner names (not ideal for small screens).");

    po::variables_map vm;
    try{
        po::store(po::parse_config_file<char>((ExePath() + "/Data_Files/config.txt").c_str(), options), vm);
    }catch(std::exception e){
        std::cout << "Exception caught: " << e.what()
                  << "\nError occurred while parsing \"Data_Files/config.txt\". The file is either missing or improperly configured." << std::endl;
        return 0;
    }
    po::notify(vm);

    if(!vm.count("key")){
        cout << "API Key could not be found within config.txt. Exiting program." << endl;
        return 0;
    }
    if(!vm.count("askInfo")){
        cout << "askInfo choice could not be found within config.txt. Exiting program." << endl;
        return 0;
    }

    string name, server, key, platformid;
    bool askInfoAnswer = 0, displayAnswer = 0;

    key = vm["key"].as<string>();
    askInfoAnswer = vm["askInfo"].as<bool>();

    if(!askInfoAnswer){
        if(!vm.count("name")){
            cout << "No user input selected, but summoner name could not be found within config.txt. Exiting program." << endl;
            return 0;
        }
        else if(!vm.count("server")){
            cout << "No user input selected, but server could not be found within config.txt. Exiting program." << endl;
            return 0;
        }
        else if(!vm.count("showNames")){
            cout << "No user input selected, but showName decision could not be found within config.txt. Exiting program." << endl;
            return 0;
        }

        name = vm["name"].as<string>();
        server = vm["server"].as<string>();
        displayAnswer = vm["showNames"].as<bool>();

        standardise(name);
        standardise(server);
        auto findIt = serverlist.find(server);
        if(findIt != serverlist.end()) platformid = findIt->second;
        else{
            cout << "Invalid server in config.txt. Exiting program." << endl;
            return 0;
        }
    }





    //INITIALISE CURL
    curl_global_init(CURL_GLOBAL_ALL); //needed, read documentation
    CURL* curl = curl_easy_init(); //our curl object
    if(!curl){
        cout << "Error occurred initialising curl easy handle. Exiting..." << endl;
        return 0;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);





    //INITIAILISE CURSES
    setlocale(LC_ALL, ""); //Needed to display characters 127-255 properly (e.g. accented characters).
    initscr();
    unsigned row, col;
    getmaxyx(stdscr, row, col);
    refresh();





    //OBTAIN ANSWERS IF NOT LOOPING
    if(askInfoAnswer){

        erase();
        string output = "Enter the summoner name: ";
        mvprintw(row/2 - 2, (col-output.size())/2, output.c_str());
        move(row/2, (col-output.size())/2);
        refresh();
        char nameInput[100];
        getstr(nameInput);
        name = nameInput;
        standardise(name);

        erase();
        while(true){
            output = "Enter server: (e.g. euw, las, na, br) ";
            mvprintw(row/2 - 2, (col-output.size())/2, output.c_str());
            move(row/2, (col-output.size())/2);
            refresh();
            char serverinput[100];
            getstr(serverinput);
            server = serverinput;
            standardise(server);
            auto findIt = serverlist.find(server);
            if(findIt != serverlist.end()) platformid = findIt->second;
            else{
                erase();
                output = "Server entered not valid.";
                mvprintw(row/2 - 2, (col-output.size())/2, output.c_str());
                continue;
            }
            break;
        }

        erase();
        output = "Show summoner names? (not for small screens)";
        mvprintw(row/2 - 2, (col-output.size())/2, output.c_str());
        move(row/2, (col-output.size())/2);
        refresh();
        char displayInput[100];
        getstr(displayInput);
        displayAnswer = tolower(displayInput[0]) == 'y';

    }

    curs_set(0); //no blinking cursor
    long long prevgameid = -1;


    erase(); //Only want "acquiring game info" to appear the first time we obtain data so we take it outside the loop. Also put it after the post-game information screen.
    string output = "Acquiring game information."; //output holds most of what we print to curses screen
    mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
    refresh();


    //MAIN LOOP

    while(true){

        beginning:

        string gametype = "UNRECORDED GAMETYPE";
        string url;

        //BELOW: OBTAIN SUMMONER ID

        url = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v1.4/summoner/by-name/" + name + key;
        curl_data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) == CURLE_OK){
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                if(response_code == 404){
                    erase();
                    string error = "No such player exists on this server.";
                    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                    refresh();
                    sleepMilli(5000);
                    endwin();
                    return 0;
                }
                else{
                    erase();
                    string error = "Summoner information inaccessible.";
                    mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                    error = "Error code: " + std::to_string(response_code);
                    mvprintw(row/2, (col-error.size())/2, error.c_str());
                    refresh();
                    sleepMilli(60000);
                    continue;
                }
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleepMilli(60000);
            continue;
        }

        Json::Value sumname;
        Json::Reader reader;
        reader.parse(curl_data, sumname);







        //BELOW: OBTAIN CURRENT GAME INFO

        url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                         + platformid + "/" + std::to_string(sumname[name]["id"].asInt64()) + key;
        curl_data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) == CURLE_OK){
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                if(response_code == 404){
                    erase();
                    string error = sumname[name]["name"].asString() + " (" + capitalise_copy(server) + ") " + "is not in a game.";
                    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                    refresh();
                    sleepMilli(30000);
                    continue;
                }
                else{
                    erase();
                    string error = "Game information inaccessible.";
                    mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                    error = "Error code: " + std::to_string(response_code);
                    mvprintw(row/2, (col-error.size())/2, error.c_str());
                    refresh();
                    sleepMilli(60000);
                    continue;
                }
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleepMilli(60000);
            continue;
        }

        Json::Value gameinfo;
        reader.parse(curl_data, gameinfo);

        if(gameinfo["gameId"].asInt64() == prevgameid){
            erase();
            string error = sumname[name]["name"].asString() + " (" + capitalise_copy(server) + ") " + "is not in a game.";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleepMilli(30000);
            continue;
        }



        //BELOW: WORK OUT GAME TYPE

        auto gametypeIt = queues.find(std::to_string(gameinfo["gameQueueConfigId"].asInt()));
        if(gametypeIt != queues.end()) gametype = gametypeIt->second; //so gametype remains UNRECORDED if it doesn't exist

        unsigned participantNo = 0;

        for(auto c : gameinfo["participants"]){
            ++participantNo;
            if(c["summonerId"] == sumname[name]["id"]) sumname[name]["participantId"] = participantNo;
        }




        //BELOW: OBTAIN CHAMPIONID INFO

        url = "https://global.api.pvp.net/api/lol/static-data/" + server + "/v1.2/champion" + key;
        curl_data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) == CURLE_OK){
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                erase();
                string error = "Champion information inaccessible.";
                mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                error = "Error code: " + std::to_string(response_code);
                mvprintw(row/2, (col-error.size())/2, error.c_str());
                refresh();
                sleepMilli(60000);
                continue;
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleepMilli(60000);
            continue;
        }

        Json::Value championinfo;
        reader.parse(curl_data, championinfo);

        //BELOW: OBTAIN LEAGUE INFO



        Json::Value leagueinfo;

        string urlStart = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v2.5/league/by-summoner/";
        string urlAppend;
        unsigned playerCount = 0;


        for(auto c : gameinfo["participants"]){
            urlAppend += std::to_string(c["summonerId"].asInt64()) + ",";
            ++playerCount;
            if(playerCount%10 == 0 || playerCount == participantNo){ //this is needed since Riot can't send more than 10 player's leagueinfo at once

                url = urlStart + urlAppend.substr(0, urlAppend.size()-1) + "/entry" + key; //substr to remove the last comma
                curl_data = "";
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                if(curl_easy_perform(curl) == CURLE_OK){
                    long response_code;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    if(response_code != 200){
                        erase();
                        string error = "League information inaccessible.";
                        mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                        error = "Error code: " + std::to_string(response_code);
                        mvprintw(row/2, (col-error.size())/2, error.c_str());
                        refresh();
                        sleepMilli(60000);
                        goto beginning; //annoyingly continue cannot be used since we're in a for loop
                    }
                }
                else{
                    erase();
                    string error = "Connection error. Waiting a minute...";
                    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                    refresh();
                    sleepMilli(60000);
                    goto beginning;
                }

                urlAppend = "";
                Json::Value tempLeague;
                reader.parse(curl_data, tempLeague);
                leagueinfo.append(tempLeague);
            }
        }


        string nameOutput; //for later

        for(auto& c : gameinfo["participants"]){
            c["champname"] = "UNKNOWN";
            c["position"] = "(UNRANKED)";
            for(auto k : leagueinfo){
                for(auto d : k[std::to_string(c["summonerId"].asInt64())])
                    if(d["queue"].asString() == "RANKED_SOLO_5x5"){
                        c["tier"] = d["tier"];
                        c["division"] = d["entries"][0]["division"];
                        c["position"] = "(" + d["tier"].asString() + " " + d["entries"][0]["division"].asString() + ")";
                    }
                for(auto e : championinfo["data"])
                    if(c["championId"] == e["id"])
                        c["champname"] = e["name"];
                if(c["summonerId"] == sumname[name]["id"]){
                    nameOutput = "\"" + c["summonerName"].asString() + "\"" + " as " + c["champname"].asString()
                                  + " (" + (c["teamId"].asInt() == 100? "Blue Team" : "Red Team")
                                  + ")";
                    sumname[name]["fullname"] = c["summonerName"];
                    sumname[name]["champion"] = c["champname"];
                    sumname[name]["teamId"] = c["teamId"];
                }
            }
        }

        prevgameid = gameinfo["gameId"].asInt64();


        //OUTPUT FORMATTING NEXT

        unsigned maxlengthA = 0, maxlengthB = 0, maxlength = 0;
        unsigned bluesize = 0, redsize = 0;

        for(auto c : gameinfo["participants"]){
            unsigned length = c["champname"].asString().size() + c["position"].asString().size() + 1
                         + (displayAnswer ? c["summonerName"].asString().size() + 3 : 0);
            if(c["teamId"].asInt() == 100){
                ++bluesize;
                if(maxlengthA < length)
                    maxlengthA = length;
            }
            if(c["teamId"].asInt() == 200){
                ++redsize;
                if(maxlengthB < length)
                    maxlengthB = length;
            }
        }

        if(maxlengthA == 0) maxlengthA = 15;
        if(maxlengthB == 0) maxlengthB = 15;
        maxlength = maxlengthA + 5 + maxlengthB;

        if(maxlength > col){
            maxlengthA = 0, maxlengthB = 0;
            for(auto& c : gameinfo["participants"]){
                if(c["position"].asString() == "(UNRANKED)")
                    c["position"] = "(UNRANK)";
                else{
                    string temp = "(";
                    temp.push_back((c["tier"].asString())[0]);
                    temp.push_back((c["tier"].asString())[1]);
                    temp.push_back((c["tier"].asString())[2]);
                    temp += " " + c["division"].asString() + ")";
                    c["position"] = temp;
                }
                unsigned length = c["champname"].asString().size() + c["position"].asString().size() + 1
                             + (displayAnswer ? c["summonerName"].asString().size() + 3 : 0);
                if(c["teamId"].asInt() == 100){
                    if(maxlengthA < length)
                        maxlengthA = length;
                }
                if(c["teamId"].asInt() == 200){
                    if(maxlengthB < length)
                        maxlengthB = length;
                }

            }
            if(maxlengthA == 0) maxlengthA = 15;
            if(maxlengthB == 0) maxlengthB = 15;
            maxlength = maxlengthA + 5 + maxlengthB;
        }

        int printlines = 4 + (4) + (bluesize > redsize ? bluesize : redsize); //change parentheses value to move display up or down
        unsigned long long gametime = gameinfo["gameStartTime"].asInt64() <= 0 ? 0 : gameinfo["gameStartTime"].asInt64()/1000;




        //MAIN GAME OUTPUTTING

        bool errorprint = false;
        string erroroutput;

        while(true){

            erase();

            mvprintw((row/2)-(printlines/2), (col-strlen(gametype.c_str()))/2, gametype.c_str());
            mvprintw((row/2)-(printlines/2)+2, (col-nameOutput.size())/2, nameOutput.c_str());
            mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2, "Blue Team");
            mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2 + maxlength - strlen("Red Team"), "Red Team");

            for(unsigned i = 0; i < bluesize|| i < redsize; ++i){

                if(i < bluesize){
                    auto f = gameinfo["participants"][i];
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2), f["champname"].asString().c_str());
                    if(displayAnswer) printw((" \"" + f["summonerName"].asString() + "\"").c_str());
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA - f["position"].asString().size(), f["position"].asString().c_str());
                }
                if(i < redsize){
                    auto g = gameinfo["participants"][i+bluesize];
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5, g["champname"].asString().c_str());
                    if(displayAnswer) mvprintw((row/2) - (printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5 + g["champname"].asString().size() + 1,
                                         ("\"" + g["summonerName"].asString() + "\"").c_str());
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlength - g["position"].asString().size(), g["position"].asString().c_str());
                }
            }


            if(errorprint){
                mvprintw((row/2)-(printlines/2)+8+(bluesize > redsize ? bluesize : redsize), (col-erroroutput.size())/2, erroroutput.c_str());
                errorprint = false;
            }


            for(unsigned i = 0; i < 30; ++i){ //GAMETIME PRINTING
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                unsigned long long secsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

                unsigned tempgametime = (gametime > 0) ? secsSinceEpoch - gametime : 0;
                unsigned gameminutes = tempgametime/60;
                unsigned gameseconds = tempgametime - gameminutes*60;

                mvprintw((row/2)-(printlines/2)+4, (col-5)/2, ((gameminutes < 10 ? "0" : "") + std::to_string(gameminutes) + ":" +
                                                               (gameseconds < 10 ? "0" : "") + std::to_string(gameseconds)).c_str());
                refresh();
                sleepMilli(1000);
            }

            url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                             + platformid + "/" + std::to_string(sumname[name]["id"].asInt64()) + key;
            curl_data = "";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if(curl_easy_perform(curl) != CURLE_OK){
                erroroutput = "Connection error...";
                errorprint = true;
                continue;
            }

            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                if(response_code == 404) break; //no game was found and we can break
                else{
                    erroroutput = "Error code: " + std::to_string(response_code);
                    errorprint = true;
                    continue; //there was an error
                }
            }

            Json::Value temp;
            reader.parse(curl_data, temp);

            if(temp["gameId"] != gameinfo["gameId"])
                break; //found a game but it differs from the current one.
            if(gametime == 0 && temp["gameStartTime"].asInt64() > 0)
                gametime = temp["gameStartTime"].asInt64()/1000;

        }





        //POST-GAME INFORMATION OUTPUT

        for(unsigned tries = 0; ; ++tries){ //postgame information start

            if(tries >= 6){
                erase();
                output = "Could not find post-game information.";
                mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
                refresh();
                sleepMilli(10000);
                break;
            }

            erase();
            output = "Acquiring post-game information.";
            mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
            refresh();
            sleepMilli(20000);

            url = "https://" + server + ".api.pvp.net/api/lol/" + server
                             + "/v2.2/match/" + std::to_string(gameinfo["gameId"].asInt64()) + key;

            curl_data = "";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            while(curl_easy_perform(curl) != CURLE_OK){
                erase();
                string error = "Connection error. Waiting a minute...";
                mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                refresh();
                curl_data = "";
                sleepMilli(60000);
                continue;
            }
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                continue;
            }

            Json::Value postgame;
            reader.parse(curl_data, postgame);

            erase();
            int gameminutes = postgame["matchDuration"].asInt()/60;
            int gameseconds = postgame["matchDuration"].asInt() - (60*gameminutes);

            int tempwidth = (25 > (11 + gametype.size()) ? 25 : 11 + gametype.size());
            int widthsize = (tempwidth > 0.75*col ? tempwidth : 0.75*col);
            int printlines = 16; // number of stats to print, change parenthesis for height

            int outputint;
            double outputdouble;

            output = "Name:";
            mvprintw((row/2)-(printlines/2), (col-widthsize)/2, output.c_str());
            output = "\"" + sumname[name]["fullname"].asString() + "\"";
            mvprintw((row/2)-(printlines/2), (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            output = "Champion:";
            mvprintw((row/2)-(printlines/2) + 1, (col-widthsize)/2, output.c_str());
            output = sumname[name]["champion"].asString();
            mvprintw((row/2)-(printlines/2) + 1, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            output = "Game Mode:";
            mvprintw((row/2)-(printlines/2) + 2, (col-widthsize)/2, output.c_str());
            output = gametype;
            mvprintw((row/2)-(printlines/2) + 2, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            output = "Game Length:";
            mvprintw((row/2)-(printlines/2) + 3, (col-widthsize)/2, output.c_str());
            output = (gameminutes  < 10 ? "0" : "") + std::to_string(gameminutes) + ":" + (gameseconds < 10 ? "0" : "") + std::to_string(gameseconds);
            mvprintw((row/2)-(printlines/2) + 3, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            for(auto c : postgame["participants"]){
                if(c["participantId"].asInt() == sumname[name]["participantId"].asInt()){ //compares unsigned and signed and so asInt() is needed

                    output = "Status:";
                    mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2, output.c_str());
                    output = (c["stats"]["winner"].asInt() == 1 ? "VICTORY" : "DEFEAT");
                    mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                    output = "KDA:";
                    mvprintw((row/2)-(printlines/2) + 5, (col-widthsize)/2, output.c_str());
                    output = std::to_string(c["stats"]["kills"].asInt()) + "/" + std::to_string(c["stats"]["deaths"].asInt()) + "/" + std::to_string(c["stats"]["assists"].asInt());
                    mvprintw((row/2)-(printlines/2) + 5, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                    output = "Gold Earned:";
                    mvprintw((row/2)-(printlines/2) + 6, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["goldEarned"].asInt();
                    mvprintw((row/2)-(printlines/2) + 6, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "Minions Killed:";
                    mvprintw((row/2)-(printlines/2) + 7, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["minionsKilled"].asInt() + c["stats"]["neutralMinionsKilled"].asInt();
                    mvprintw((row/2)-(printlines/2) + 7, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "CS per 10:";
                    mvprintw((row/2)-(printlines/2) + 8, (col-widthsize)/2, output.c_str());
                    outputdouble = 600*(static_cast<double>(c["stats"]["minionsKilled"].asInt() + c["stats"]["neutralMinionsKilled"].asInt())/(postgame["matchDuration"].asInt()));
                    mvprintw((row/2)-(printlines/2) + 8, (col-widthsize)/2 + widthsize - std::to_string(int(outputdouble)).size() - 3, "%.2f", outputdouble);

                    output = "Wards Placed:";
                    mvprintw((row/2)-(printlines/2) + 9, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["wardsPlaced"].asInt();
                    mvprintw((row/2)-(printlines/2) + 9, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "Largest Killing Spree:";
                    mvprintw((row/2)-(printlines/2) + 10, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["largestKillingSpree"].asInt();
                    mvprintw((row/2)-(printlines/2) + 10, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "Largest Multi Kill:";
                    mvprintw((row/2)-(printlines/2) + 11, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["largestMultiKill"].asInt();
                    mvprintw((row/2)-(printlines/2) + 11, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "Damage Dealt:";
                    mvprintw((row/2)-(printlines/2) + 12, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["totalDamageDealtToChampions"].asInt();
                    mvprintw((row/2)-(printlines/2) + 12, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);

                    output = "Damage Taken:";
                    mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["totalDamageTaken"].asInt();
                    mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2 + widthsize - std::to_string(outputint).size(), "%d", outputint);
                }
            }

            int enemydrake, enemybaron;
            int allydrake, allybaron;

            for(auto c : postgame["teams"]){
                if(c["teamId"] == sumname[name]["teamId"]){
                    allydrake = c["dragonKills"].asInt();
                    allybaron = c["baronKills"].asInt();
                }
                else{
                    enemydrake = c["dragonKills"].asInt();
                    enemybaron = c["baronKills"].asInt();
                }
            }

            output = "Ally/Enemy Drakes:";
            mvprintw((row/2)-(printlines/2) + 14, (col-widthsize)/2, output.c_str());
            output = std::to_string(allydrake) + "/" + std::to_string(enemydrake);
            mvprintw((row/2)-(printlines/2) + 14, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            output = "Ally/Enemy Barons:";
            mvprintw((row/2)-(printlines/2) + 15, (col-widthsize)/2, output.c_str());
            output = std::to_string(allybaron) + "/" + std::to_string(enemybaron);
            mvprintw((row/2)-(printlines/2) + 15, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            refresh();
            sleepMilli(180000);
            break;

        }

        erase();
        output = "Acquiring game information.";
        mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
        refresh();

    }


    curl_easy_cleanup(curl);
    curl_global_cleanup();
    endwin();
    return 0;

}
