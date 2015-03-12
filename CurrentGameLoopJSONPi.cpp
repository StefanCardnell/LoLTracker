//REQUIRES INSTALLATION OF CURL LIBRARY. ALSO IS INTENDED FOR WINDOWS SYSTEMS.
//g++ -std=c++0x -o main -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/ -L/usr/lib/ -L/usr/local/lib -L/usr/lib/ar-linux-gnueabihf -lncurses -lcurl -ljsoncpp CurrentGameLoopJSONPi.cpp CurrentGameLoopFunctionsJSON.cpp

//g++ -std=c++0x -o main -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/ -lcurl -ljsoncpp -lncursesw CurrentGameLoopJSONPi.cpp CurrentGameFunctionsJSON.cpp
//USE THIS FOR WCHAR^^

// TO PERSONALISE FOR SELF-COMPILATION, LOOK FOR CHANGE 1 of 1 BELOW

#define _XOPEN_SOURCE_EXTENDED //needed to get ncurses to display wchar, see http://www.roguebasin.com/index.php?title=Ncursesw
#define CURL_STATICLIB

#include <string>
#include <vector>
#include <curl/curl.h>
#include <iomanip> //for setw
#include <json/json.h>
#include <fstream>
#include <unistd.h> //for sleep()
#include <ncursesw/curses.h> //extended ncurses library to display wchar
#include <string.h>
#include <sys/time.h>

using namespace std;



struct qtype{
    qtype() = default;
    qtype(string a, string b): configid(a), description(b) { }
    string configid;
    string description;
};

const vector<qtype> queues = {qtype("0", "Custom Game"), qtype("2", "Normal 5v5 Blind Pick"), qtype("7", "Coop vs AI"),
                  qtype("31", "Coop vs AI Intro"), qtype("32", "Coop vs AI Beginner"), qtype("33", "Coop vs AI Intermediate"),
                  qtype("8", "Normal 3v3"), qtype("14", "Normal 5v5 Draft Pick"), qtype("16", "Dominion 5v5 Blind Pick"),
                  qtype("17", "Dominion 5v5 Draft Pick"), qtype("25", "Dominion Coop vs AI"), qtype("4", "Ranked Solo 5v5"),
                  qtype("9", "Ranked Duo 3v3"), qtype("6", "Ranked Duo 5v5"), qtype("41", "Ranked Team 3v3"),
                  qtype("42", "Ranked Team 5v5"), qtype("52", "Twisted Treeline Coop vs AI"), qtype("61", "Teambuilder"),
                  qtype("65", "ARAM"), qtype("70", "One for all"), qtype("72", "Snowdown Showdown 1v1"),
                  qtype("73", "Snowdown Showdown 2v2"), qtype("75", "SR 6x6 Hexakill"), qtype("76", "URF"),
                  qtype("83", "URF against AI"), qtype("91", "Doom Bots Rank 1"), qtype("92", "Doom Bots Rank 2"),
                  qtype("93", "Doom Bots Rank 5"), qtype("96", "Ascension"), qtype("98", "Twisted Treeline 6x6 Hexakill"),
                  qtype("300", "King Poro"), qtype("310", "Nemesis Pick")};

struct serverplatform{
    serverplatform() = default;
    serverplatform(string a, string b): servername(a), platformid(b) { }
    string servername;
    string platformid;
};

const vector<serverplatform> serverlist = {serverplatform("na", "NA1"), serverplatform("br", "BR1"), serverplatform("lan", "LA1"),
                                            serverplatform("las", "LA2"), serverplatform("ru", "RU"), serverplatform("tr", "TR1"),
                                            serverplatform("eune", "EUN1"), serverplatform("euw", "EUW1"), serverplatform("kr", "KR"),
                                            serverplatform("oce", "OC1") };


extern string data;
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
string standardise(string name);
string capitalised(string name);



int main(){

    setlocale(LC_ALL, ""); //for ncurses to display wchar

    CURL* curl; //our curl object
    curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    string name = /*ENTER HERE*/, server = /*ENTER HERE*/, displayanswer = /*ENTER Y or N HERE*/, key; //CHANGE 1 of 1: PERSONALISE BY ENTEREING VALUES HERE, LEAVE KEY ALONE.

    ifstream keyinput("/home/pi/LeagueGame/key.txt");
    if(!keyinput.is_open()){
        cout << "key.txt could not be found. Exiting..." << endl;
        sleep(5);
        return 0;
    }

    keyinput >> key;
    keyinput.close();


    initscr();
    int row, col;
    getmaxyx(stdscr, row, col);
    refresh();

    long long prevgameid = 0;


    while(true){


        string platformid;
        string gametype = "UNRECORDED GAMETYPE";
        string url;

        for(auto c : serverlist)
            if(standardise(server) == c.servername)
                platformid = c.platformid;



        //BELOW: OBTAIN SUMMONER ID



        url = "https://" + standardise(server) + ".api.pvp.net/api/lol/" + standardise(server) + "/v1.4/summoner/by-name/" + standardise(name) + key;

        Json::Value sumname;
        Json::Reader reader;


        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) != CURLE_OK){
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            data = "";
            sleep(60);
            continue;
        }

        reader.parse(data, sumname);

        if(sumname.empty()){
            erase();
            string error = "No such player exists on this server.";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(5);
            endwin();
            return 0;
        }






        //BELOW: OBTAIN GAME INFO


        url = "https://" + standardise(server) + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                         + platformid + "/" + to_string(sumname[standardise(name)]["id"].asInt()) + key;

        Json::Value gameinfo;

        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) != CURLE_OK){
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            data = "";
            sleep(60);
            continue;
        }

        reader.parse(data, gameinfo);

        if(gameinfo.empty() || gameinfo["gameId"].asInt64() == prevgameid){
            erase();
            string error = (sumname[standardise(name)]["name"].asString() + " (" + capitalised(server) + ") " + "is not in a game.");
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            sleep(30);
            continue;
        }

        prevgameid = gameinfo["gameId"].asInt64();



        //BELOW: WORK OUT GAME TYPE


        for(auto c : queues)
            if(to_string(gameinfo["gameQueueConfigId"].asInt()) == c.configid) gametype = c.description;




        //BELOW: OBTAIN CHAMPIONID INFO



        url = "https://global.api.pvp.net/api/lol/static-data/" + standardise(server) + "/v1.2/champion" + key;

        Json::Value championinfo;

        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) != CURLE_OK){
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            data = "";
            sleep(60);
            continue;
        }

        reader.parse(data, championinfo);




        //BELOW: OBTAIN LEAGUE INFO



        url = "https://" + standardise(server) + ".api.pvp.net/api/lol/" + standardise(server) + "/v2.5/league/by-summoner/";

        for(auto c : gameinfo["participants"])
            url += to_string(c["summonerId"].asInt()) + ",";

        url += "/entry" + key;

        Json::Value leagueinfo;

        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(curl_easy_perform(curl) != CURLE_OK){
            erase();
            string error = "Connection error. Waiting a minute...";
            mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
            refresh();
            data = "";
            sleep(60);
            continue;
        }

        reader.parse(data, leagueinfo);

        string nameoutput; //for later
        string output;

        for(auto& c : gameinfo["participants"]){
            c["champname"] = "UNKNOWN";
            c["position"] = "(UNRANKED)";
            for(auto d : leagueinfo[to_string(c["summonerId"].asInt())])
                if(d["queue"].asString() == "RANKED_SOLO_5x5"){
                    c["tier"] = d["tier"];
                    c["division"] = d["entries"][0]["division"];
                    c["position"] = "(" + d["tier"].asString() + " " + d["entries"][0]["division"].asString() + ")";
                }
            for(auto e : championinfo["data"])
                if(c["championId"] == e["id"])
                    c["champname"] = e["name"];
            if(c["summonerId"] == sumname[standardise(name)]["id"]){
                nameoutput = "\"" + c["summonerName"].asString() + "\"" + " as " + c["champname"].asString()
                              + " (" + (c["teamId"].asInt() == 100? "Blue Team" : "Purple Team")
                              + ")";
                sumname[standardise(name)]["champion"] = c["champname"];
                sumname[standardise(name)]["teamId"] = c["teamId"];
            }
        }


        //OUTPUT AND FORMATTING NEXT

        bool display = (tolower(displayanswer[0]) == 'y');
        bool errorprint = false;
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
                if(c["position"].asString() == "(Unranked)")
                    c["position"] = "(Unrank)";
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
                output = "Connection error...";
                mvprintw((row/2)-(printlines/2)+8+(bluesize > purplesize ? bluesize : purplesize), (col-output.size())/2, output.c_str());
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

            url = "https://" + standardise(server) + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/"
                             + platformid + "/" + to_string(sumname[standardise(name)]["id"].asInt()) + key;

            data = "";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            if(curl_easy_perform(curl) != CURLE_OK){
                    errorprint = true;
                    continue;
            }
            else errorprint = false;


            Json::Value temp;
            reader.parse(data, temp);

            if(temp.empty() || temp["gameId"] != gameinfo["gameId"])
                break;
            else if(gametime == 0 && temp["gameStartTime"].asInt64() > 0)
                gametime = temp["gameStartTime"].asInt64()/1000;
        }

        bool found = false;
        int tries = 0;
        curs_set(1);


        do{
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


            url = "https://" + standardise(server) + ".api.pvp.net/api/lol/" + standardise(server)
                             + "/v1.3/game/by-summoner/" + to_string(sumname[standardise(name)]["id"].asInt())
                             + "/recent" + key;


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

            Json::Value postgame;

            reader.parse(data, postgame);



            url = "https://" + standardise(server) + ".api.pvp.net/api/lol/" + standardise(server)
                             + "/v2.2/match/" + to_string(gameinfo["gameId"].asInt()) + key;

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

            Json::Value postgameadditional;

            reader.parse(data, postgameadditional);




            ++tries;

            if(!postgameadditional.empty())
                for(auto c : postgame["games"]){
                    if(c["gameId"] == gameinfo["gameId"]){
                        erase();
                        found = true;
                        int gameminutes = c["stats"]["timePlayed"].asInt()/60;
                        int gameseconds = c["stats"]["timePlayed"].asInt() - (60*gameminutes);

                        int tempwidth = (25 > (11 + gametype.size()) ? 25 : 11 + gametype.size());
                        int widthsize = (tempwidth > 0.75*col ? tempwidth : 0.75*col);
                        int printlines = 16; // number of stats to print, change parenthesis for height


                        int outputint;
                        double outputdouble;

                        output = "Name:";
                        mvprintw((row/2)-(printlines/2), (col-widthsize)/2, output.c_str());

                        output = "\"" + name + "\"";
                        mvprintw((row/2)-(printlines/2), (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                        output = "Champion:";
                        mvprintw((row/2)-(printlines/2) + 1, (col-widthsize)/2, output.c_str());

                        output = sumname[standardise(name)]["champion"].asString();
                        mvprintw((row/2)-(printlines/2) + 1, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                        output = "Game Mode:";
                        mvprintw((row/2)-(printlines/2) + 2, (col-widthsize)/2, output.c_str());

                        output = gametype;
                        mvprintw((row/2)-(printlines/2) + 2, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                        output = "Game Length:";
                        mvprintw((row/2)-(printlines/2) + 3, (col-widthsize)/2, output.c_str());

                        output = (gameminutes  < 10 ? "0" : "") + to_string(gameminutes) + ":" + (gameseconds < 10 ? "0" : "") + to_string(gameseconds);
                        mvprintw((row/2)-(printlines/2) + 3, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                        output = "Status:";
                        mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2, output.c_str());

                        output = (c["stats"]["win"].asInt() == 1 ? "VICTORY" : "DEFEAT");
                        mvprintw((row/2)-(printlines/2) + 4, (col-widthsize)/2 + widthsize - output.size(), output.c_str());

                        output = "KDA:";
                        mvprintw((row/2)-(printlines/2) + 5, (col-widthsize)/2, output.c_str());

                        output = to_string(c["stats"]["championsKilled"].asInt()) + "/" + to_string(c["stats"]["numDeaths"].asInt()) + "/" + to_string(c["stats"]["assists"].asInt());
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

                        outputdouble = 600*(static_cast<double>(c["stats"]["minionsKilled"].asInt() + c["stats"]["neutralMinionsKilled"].asInt())/(c["stats"]["timePlayed"].asInt()));
                        mvprintw((row/2)-(printlines/2) + 8, (col-widthsize)/2 + widthsize - to_string(int(outputdouble)).size() - 3, "%.2f", outputdouble);

                        output = "Wards Placed:";
                        mvprintw((row/2)-(printlines/2) + 9, (col-widthsize)/2, output.c_str());

                        outputint = c["stats"]["wardPlaced"].asInt();
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

                        outputint = c["stats"]["totalDamageDealt"].asInt();
                        mvprintw((row/2)-(printlines/2) + 12, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                        output = "Damage Taken:";
                        mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2, output.c_str());

                        outputint = c["stats"]["totalDamageTaken"].asInt();
                        mvprintw((row/2)-(printlines/2) + 13, (col-widthsize)/2 + widthsize - to_string(outputint).size(), "%d", outputint);

                        int enemydrake, enemybaron;
                        int allydrake, allybaron;

                        for(auto c : postgameadditional["teams"]){
                            if(c["teamId"] == sumname[standardise(name)]["teamId"]){
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

                    }
                }

        }while(found == false);




        continue;

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        endwin();
        return 0;

    }
}
