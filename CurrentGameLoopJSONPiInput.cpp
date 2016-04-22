//REQUIRES INSTALLATION OF CURL LIBRARY. THIS VERSION IS FOR UNIX.

//g++ -std=c++11 -o input -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/  CurrentGameLoopJSONPiInput.cpp CurrentGameFunctionsJSON.cpp -lcurl -lncursesw -ljsoncpp

//-l will automatically use lib<name>.so files if it finds them.
//to statically link jsoncpp, add "-Wl,-static -ljsoncpp -Wl,-Bdynamic -lcurl -lncursesw" to the end (while removing earlier links)

#define _XOPEN_SOURCE_EXTENDED //needed to get ncurses to display wchar, see http://www.roguebasin.com/index.php?title=Ncursesw
#define CURL_STATICLIB

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip> //for setw
#include <cstring> //needed for strlen
#include <unistd.h> //for sleep()
#include <sys/time.h>
#include <curl/curl.h>
#include <json/json.h>
#include <ncursesw/curses.h> //extended ncurses library to display wchar
#include "Functions.h"

using namespace std;

const map<string, string> queues = {{"0", "Custom Game"}, {"2", "Normal 5v5 Blind Pick"}, {"7", "Coop vs AI"},
                                    {"31", "Coop vs AI Intro"}, {"32", "Coop vs AI Beginner"}, {"33", "Coop vs AI Intermediate"},
                                    {"8", "Normal 3v3"}, {"14", "Normal 5v5 Draft Pick"}, {"16", "Dominion 5v5 Blind Pick"},
                                    {"17", "Dominion 5v5 Draft Pick"}, {"25", "Dominion Coop vs AI"}, {"4", "Ranked Solo 5v5"},
                                    {"9", "Ranked Premade 3v3"}, {"6", "Ranked Premade 5v5"}, {"41", "Ranked Team 3v3"},
                                    {"42", "Ranked Team 5v5"}, {"52", "Twisted Treeline Coop vs AI"}, {"61", "Teambuilder"},
                                    {"65", "ARAM"}, {"70", "One for all"}, {"72", "Snowdown Showdown 1v1"},
                                    {"73", "Snowdown Showdown 2v2"}, {"75", "SR 6x6 Hexakill"}, {"76", "URF"},
                                    {"83", "URF against AI"}, {"91", "Doom Bots Rank 1"}, {"92", "Doom Bots Rank 2"},
                                    {"93", "Doom Bots Rank 5"}, {"96", "Ascension"}, {"98", "Twisted Treeline 6x6 Hexakill"},
                                    {"300", "King Poro"}, {"310", "Nemesis Pick"}, {"100", "Butcher's Bridge ARAM"},
                                    {"313", "Black Market Brawlers"}, {"400", "Normal 5v5 Draft Pick"}, {"410", "Ranked 5v5 Draft Pick"}
                                   };



const map<string, string> serverlist = {{"na", "NA1"}, {"br", "BR1"}, {"lan", "LA1"},
                                        {"las", "LA2"}, {"ru", "RU"}, {"tr", "TR1"},
                                        {"eune", "EUN1"}, {"euw", "EUW1"}, {"kr", "KR"},
                                        {"oce", "OC1"}
                                       };
string data;




int main(){

    setlocale(LC_ALL, ""); //for ncurses to display wchar

    CURL* curl; //our curl object
    curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
    curl = curl_easy_init();
    if(!curl){
        cout << "Error occurred initialising curl easy handle. Exiting..." << endl;
        return 0;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    string name, server, displayanswer, key, platformid;

    ifstream keyinput("key.txt"); //change this for portability
    if(!keyinput){
        cout << "key.txt could not be found. Exiting..." << endl;
        return 0;
    }

    keyinput >> key;
    keyinput.close();


    initscr();
    int row, col;
    getmaxyx(stdscr, row, col);
    refresh();

    erase();
    string output = "Enter the summoner name: ";
    mvprintw(row/2 -1, (col-output.size())/2, output.c_str());
    move(row/2, (col-output.size())/2);
    refresh();
    char nameinput[100];
    getstr(nameinput);
    name = nameinput;
    makeStandardised(name);

    erase();
    while(true){
    	output = "Enter server: (e.g. euw, las, na, br) ";
    	mvprintw(row/2 -1, (col-output.size())/2, output.c_str());
    	move(row/2, (col-output.size())/2);
    	refresh();
    	char serverinput[100];
        getstr(serverinput);
    	server = serverinput;
    	makeStandardised(server);
    	if(serverlist.find(server) != serverlist.end()) platformid = serverlist.find(server)->second;
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
    mvprintw(row/2 -1, (col-output.size())/2, output.c_str());
    move(row/2, (col-output.size())/2);
    refresh();
    char displayinput[100];
    getstr(displayinput);
    displayanswer = displayinput;

    long long prevgameid = -1;

    while(true){

        beginning:


        string gametype = "UNRECORDED GAMETYPE";
        string url;


        //BELOW: OBTAIN SUMMONER ID



        url = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v1.4/summoner/by-name/" + name + key;
        data = "";
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
                    sleep(5);
                    endwin();
                    return 0;
                }
                else{
                    erase();
                    string error = "Summoner information inaccessible.";
                    mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                    error = "Error code: " + to_string(response_code);
                    mvprintw(row/2, (col-error.size())/2, error.c_str());
                    refresh();
                    sleep(60);
                    continue;
                }
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(60);
            continue;
        }

        Json::Value sumname;
        Json::Reader reader;
        reader.parse(data, sumname);







        //BELOW: OBTAIN GAME INFO


        url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                         + platformid + "/" + to_string(sumname[name]["id"].asInt64()) + key;
        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) == CURLE_OK){
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                if(response_code == 404){
                    erase();
                    string error = sumname[name]["name"].asString() + " (" + returnCapitalised(server) + ") " + "is not in a game.";
                    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                    refresh();
                    sleep(30);
                    continue;
                }
                else{
                    erase();
                    string error = "Game information inaccessible.";
                    mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                    error = "Error code: " + to_string(response_code);
                    mvprintw(row/2, (col-error.size())/2, error.c_str());
                    refresh();
                    sleep(60);
                    continue;
                }
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(60);
            continue;
        }

        Json::Value gameinfo;
        reader.parse(data, gameinfo);

        if(gameinfo["gameId"].asInt64() == prevgameid){
            erase();
            string error = sumname[name]["name"].asString() + " (" + returnCapitalised(server) + ") " + "is not in a game.";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(30);
            continue;
        }



        //BELOW: WORK OUT GAME TYPE


        if(queues.find(to_string(gameinfo["gameQueueConfigId"].asInt())) != queues.end()) gametype = queues.find(to_string(gameinfo["gameQueueConfigId"].asInt()))->second; //so gametype remains UNRECORDED if it doesn't exist

        unsigned participantNo = 0;

        for(auto c : gameinfo["participants"]){
            ++participantNo;
            if(c["summonerId"] == sumname[name]["id"]) sumname[name]["participantId"] = participantNo;
        }




        //BELOW: OBTAIN CHAMPIONID INFO



        url = "https://global.api.pvp.net/api/lol/static-data/" + server + "/v1.2/champion" + key;
        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) == CURLE_OK){
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                erase();
                string error = "Champion information inaccessible.";
                mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                error = "Error code: " + to_string(response_code);
                mvprintw(row/2, (col-error.size())/2, error.c_str());
                refresh();
                sleep(60);
                continue;
            }
        }
        else{
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(60);
            continue;
        }

        Json::Value championinfo;
        reader.parse(data, championinfo);

        //BELOW: OBTAIN LEAGUE INFO



        Json::Value leagueinfo;

        string urlStart = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v2.5/league/by-summoner/";
        string urlAppend;
        unsigned playerCount = 0;


        for(auto c : gameinfo["participants"]){
            urlAppend += to_string(c["summonerId"].asInt64()) + ",";
            ++playerCount;
            if(playerCount%10 == 0 || playerCount == participantNo){ //this is needed since Riot can't send more than 10 player's leagueinfo at once

                url = urlStart + urlAppend + "/entry" + key;
                data = "";
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                if(curl_easy_perform(curl) == CURLE_OK){
                    long response_code;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    if(response_code != 200){
                        erase();
                        string error = "League information inaccessible.";
                        mvprintw(row/2 - 2, (col-error.size())/2, error.c_str());
                        error = "Error code: " + to_string(response_code);
                        mvprintw(row/2, (col-error.size())/2, error.c_str());
                        refresh();
                        sleep(60);
                        goto beginning; //annoyingly continue cannot be used since we're in a for loop
                    }
                }
                else{
                    erase();
                    string error = "Connection error. Waiting a minute...";
                    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                    refresh();
                    sleep(60);
                    goto beginning;
                }

                urlAppend = "";
                Json::Value tempLeague;
                reader.parse(data, tempLeague);
                leagueinfo.append(tempLeague);
            }
        }


        string nameoutput; //for later
        string output;

        for(auto& c : gameinfo["participants"]){
            c["champname"] = "UNKNOWN";
            c["position"] = "(UNRANKED)";
            for(auto k : leagueinfo){
                for(auto d : k[to_string(c["summonerId"].asInt64())])
                    if(d["queue"].asString() == "RANKED_SOLO_5x5"){
                        c["tier"] = d["tier"];
                        c["division"] = d["entries"][0]["division"];
                        c["position"] = "(" + d["tier"].asString() + " " + d["entries"][0]["division"].asString() + ")";
                    }
                for(auto e : championinfo["data"])
                    if(c["championId"] == e["id"])
                        c["champname"] = e["name"];
                if(c["summonerId"] == sumname[name]["id"]){
                    nameoutput = "\"" + c["summonerName"].asString() + "\"" + " as " + c["champname"].asString()
                                  + " (" + (c["teamId"].asInt() == 100? "Blue Team" : "Purple Team")
                                  + ")";
                    sumname[name]["fullname"] = c["summonerName"];
                    sumname[name]["champion"] = c["champname"];
                    sumname[name]["teamId"] = c["teamId"];
                }
            }
        }

        prevgameid = gameinfo["gameId"].asInt64();


        //OUTPUT FORMATTING NEXT

        bool display = (tolower(displayanswer[0]) == 'y');
        unsigned maxlengthA = 0, maxlengthB = 0, maxlength;
        unsigned bluesize = 0, purplesize = 0;

        for(auto c : gameinfo["participants"]){
            int length = c["champname"].asString().size() + c["position"].asString().size() + 1
                         + (display ? c["summonerName"].asString().size() + 3 : 0);
            if(c["teamId"].asInt() == 100){
                ++bluesize;
                if(maxlengthA < length)
                    maxlengthA = length;
            }
            if(c["teamId"].asInt() == 200){
                ++purplesize;
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
                int length = c["champname"].asString().size() + c["position"].asString().size() + 1
                             + (display ? c["summonerName"].asString().size() + 3 : 0);
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

        int printlines = 4 + (4) + (bluesize > purplesize ? bluesize : purplesize); //change parenthes$
        long long gametime = gameinfo["gameStartTime"].asInt64() <= 0 ? 0 : gameinfo["gameStartTime"].asInt64()/1000;

        //OUTPUTTING

        curs_set(0);

        bool errorprint = false;
        string erroroutput;

        while(true){

            erase();

            mvprintw((row/2)-(printlines/2), (col-strlen(gametype.c_str()))/2, gametype.c_str());
            mvprintw((row/2)-(printlines/2)+2, (col-nameoutput.size())/2, nameoutput.c_str());



            for(unsigned i = 0; i < bluesize|| i < purplesize; ++i){

                if(i < bluesize){
                    auto f = gameinfo["participants"][i];
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2), f["champname"].asString().c_str());
                    if(display) printw((" \"" + f["summonerName"].asString() + "\"").c_str());
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA - f["position"].asString().size(), f["position"].asString().c_str());
                }
                if(i < purplesize){
                    auto g = gameinfo["participants"][i+bluesize];
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5, g["champname"].asString().c_str());
                    if(display) mvprintw((row/2) - (printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5 + g["champname"].asString().size() + 1,
                                         ("\"" + g["summonerName"].asString() + "\"").c_str());
                    mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlength - g["position"].asString().size(), g["position"].asString().c_str());
                }
            }


            if(errorprint){
                mvprintw((row/2)-(printlines/2)+8+(bluesize > purplesize ? bluesize : purplesize), (col-erroroutput.size())/2, erroroutput.c_str());
                errorprint = false;
            }


            int i = 0;
            while(i < 30){ //GAMETIME PRINTING
                timeval timestamp;
                gettimeofday(&timestamp, 0);
                long long tempgametime;
                if(gametime <= 0) tempgametime = 0;
                else tempgametime = timestamp.tv_sec - gametime;

                int gameminutes = tempgametime/60;
                int gameseconds = tempgametime - gameminutes*60;

                move((row/2)-(printlines/2)+4, 0);
                clrtoeol();
                mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2, "Blue Team");
                mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2 + maxlength - strlen("Purple Team"), "Purple Team");
                mvprintw((row/2)-(printlines/2)+4, (col-5)/2, ((gameminutes < 10 ? "0" : "") + to_string(gameminutes) + ":" +
                                                               (gameseconds < 10 ? "0" : "") + to_string(gameseconds)).c_str());
                refresh();
                sleep(1);
                ++i;
            }

            url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                             + platformid + "/" + to_string(sumname[name]["id"].asInt64()) + key;
            data = "";
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
                    erroroutput = "Error code: " + to_string(response_code);
                    errorprint = true;
                    continue; //there was an error
                }
            }

            Json::Value temp;
            reader.parse(data, temp);

            if(temp["gameId"] != gameinfo["gameId"])
                break; //found a game but it differs from the current one.
            if(gametime == 0 && temp["gameStartTime"].asInt64() > 0)
                gametime = temp["gameStartTime"].asInt64()/1000;

        }

        curs_set(1);

        for(int tries = 0; ; ++tries){ //postgame information start

            if(tries >= 6){
                erase();
                string output = "Could not find game information...";
                mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
                refresh();
                sleep(10);
                break;
            }

            erase();
            string output = "Obtaining game information...";
            mvprintw(row/2 - 1, (col-output.size())/2, output.c_str());
            refresh();
            sleep(20);

            url = "https://" + server + ".api.pvp.net/api/lol/" + server
                             + "/v2.2/match/" + to_string(gameinfo["gameId"].asInt64()) + key;

            data = "";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            while(curl_easy_perform(curl) != CURLE_OK){
                erase();
                string error = "Connection error. Waiting a minute...";
                mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                refresh();
                data = "";
                sleep(60);
                continue;
            }
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != 200){
                continue;
            }

            Json::Value postgame;
            reader.parse(data, postgame);

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
            output = (gameminutes  < 10 ? "0" : "") + to_string(gameminutes) + ":" + (gameseconds < 10 ? "0" : "") + to_string(gameseconds);
            mvprintw((row/2)-(printlines/2) + 3, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            for(auto c : postgame["participants"]){
                if(c["participantId"].asInt() == sumname[name]["participantId"].asInt()){ //compares unsigned and signed and so asInt() is needed

                    output = "Status:";
                    mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2, output.c_str());
                    output = (c["stats"]["winner"].asInt() == 1 ? "VICTORY" : "DEFEAT");
                    mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                    output = "KDA:";
                    mvprintw((row/2)-(printlines/2) + 5, (col-widthsize)/2, output.c_str());
                    output = to_string(c["stats"]["kills"].asInt()) + "/" + to_string(c["stats"]["deaths"].asInt()) + "/" + to_string(c["stats"]["assists"].asInt());
                    mvprintw((row/2)-(printlines/2) + 5, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                    output = "Gold Earned:";
                    mvprintw((row/2)-(printlines/2) + 6, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["goldEarned"].asInt();
                    mvprintw((row/2)-(printlines/2) + 6, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "Minions Killed:";
                    mvprintw((row/2)-(printlines/2) + 7, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["minionsKilled"].asInt() + c["stats"]["neutralMinionsKilled"].asInt();
                    mvprintw((row/2)-(printlines/2) + 7, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "CS per 10:";
                    mvprintw((row/2)-(printlines/2) + 8, (col-widthsize)/2, output.c_str());
                    outputdouble = 600*(static_cast<double>(c["stats"]["minionsKilled"].asInt() + c["stats"]["neutralMinionsKilled"].asInt())/(postgame["matchDuration"].asInt()));
                    mvprintw((row/2)-(printlines/2) + 8, (col-widthsize)/2 + widthsize - to_string(int(outputdouble)).size() - 3, "%.2f", outputdouble);

                    output = "Wards Placed:";
                    mvprintw((row/2)-(printlines/2) + 9, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["wardsPlaced"].asInt();
                    mvprintw((row/2)-(printlines/2) + 9, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "Largest Killing Spree:";
                    mvprintw((row/2)-(printlines/2) + 10, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["largestKillingSpree"].asInt();
                    mvprintw((row/2)-(printlines/2) + 10, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "Largest Multi Kill:";
                    mvprintw((row/2)-(printlines/2) + 11, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["largestMultiKill"].asInt();
                    mvprintw((row/2)-(printlines/2) + 11, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "Damage Dealt:";
                    mvprintw((row/2)-(printlines/2) + 12, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["totalDamageDealtToChampions"].asInt();
                    mvprintw((row/2)-(printlines/2) + 12, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                    output = "Damage Taken:";
                    mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2, output.c_str());
                    outputint = c["stats"]["totalDamageTaken"].asInt();
                    mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);
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
            output = to_string(allydrake) + "/" + to_string(enemydrake);
            mvprintw((row/2)-(printlines/2) + 14, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            output = "Ally/Enemy Barons:";
            mvprintw((row/2)-(printlines/2) + 15, (col-widthsize)/2, output.c_str());
            output = to_string(allybaron) + "/" + to_string(enemybaron);
            mvprintw((row/2)-(printlines/2) + 15, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

            refresh();
            sleep(180);
            break;

        }

        continue;

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        endwin();
        return 0;

    }
}
