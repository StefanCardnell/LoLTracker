//g++ -std=c++0x -o main -I/usr/local/include -I/usr/include -L/usr/local/lib -L/usr/lib/ar-linux-gnueabihf -lncurses -lcurl CurrentGameLoopPi.cpp CurrentGameFunctions.cpp

#define CURL_STATICLIB
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <iomanip>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <fstream>

using namespace std;

struct summoner{
    string sumid, sumname;
    string champid, champ;
    string league, division, position;
};

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
bool patternandrecord(string pattern, unsigned& position, char exitchar, string& recorder);
bool pattern(string pattern, unsigned& position);
string removewhitespace(string name);

int main(){

    initscr();
    int row, col;
    getmaxyx(stdscr,row,col);

    string key;

    ifstream keyinput("keyinput.txt");
    if(!keyinput.is_open()){
	erase();
	string output = "keyinput.txt could not be found. Exiting...";
	mvprintw(row/2 - 2, (col-output.size())/2, output.c_str());
	refresh();
	sleep(5);
	endwin();
	return 0;
    }
    keyinput >> key;
    keyinput.close();

    CURL* curl; //our curl object
    curl_global_init(CURL_GLOBAL_ALL); //pretty obvious

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    string server, platformid, name, nameid;

    erase();
    string output = "Enter the summoner name: ";
    mvprintw(row/2 -1, (col-output.size())/2, output.c_str());
    move(row/2, (col-output.size())/2);
    refresh();
    char nameinput[100];
    getstr(nameinput);
    name = nameinput;

    erase();
    while(true){
    	output = "Enter server: (e.g. euw, las, na, br) ";
    	mvprintw(row/2 -1, (col-output.size())/2, output.c_str());
    	move(row/2, (col-output.size())/2);
    	refresh();
    	char serverinput[100];
	getstr(serverinput);
    	server = serverinput;
        for(auto& c : server)
            c = tolower(c);
        for(auto c : serverlist)
            if(server == c.servername)
                    platformid = c.platformid;
        if(platformid == ""){
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
    string displayanswer = displayinput;

    curs_set(0);


    while(true){
        vector<summoner> teamA, teamB;


        string gameid, gamequeueid, gametype = "UNRECORDED GAMETYPE";
        string url;



        if(nameid == ""){

            url = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v1.4/summoner/by-name/" + removewhitespace(name) + key;
            data = "";

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_perform(curl);


            if(!data.empty()){
                unsigned position = 0;
                if(!patternandrecord("\"id\":", position, ',', nameid)){
		    erase();
		    string error = "No such player exists on this server. Exiting...";
		    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
		    refresh();
		    sleep(5);
		    endwin();
		    return 0;
		}
            }
            else{
                erase();
                string error = "No data, connection error? Waiting 2 mins.";
                mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
                refresh();
                sleep(120);
                continue;
            }
        }


        url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/" + platformid + "/" + nameid + key;
        data = "";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_perform(curl);

        if(!data.empty()){
            unsigned position = 0;
            if(patternandrecord("\"gameId\":", position, ',', gameid)){
                patternandrecord("\"gameQueueConfigId\":", position, ',', gamequeueid);
                position = 0;
                while(pattern("{\"teamId\":", position)){
                    vector<summoner>& current = (data[position] == '1' ? teamA : teamB);
                    current.push_back(summoner());
                    patternandrecord("\"championId\":", position, ',', current[current.size()-1].champid);
                    patternandrecord("\"summonerName\":", position, ',', current[current.size()-1].sumname);
                    patternandrecord("\"summonerId\":", position, ',', current[current.size()-1].sumid);
                }
            }
            else{
                erase();
		string error = name + " (" + server + ") " + "is not in a game.";
		mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
		refresh();
                sleep(60);
                continue;
            }
        }
        else{
	    erase();
	    string error = "No data, connection error? Waiting 2mins.";
	    mvprintw(row/2 - 1, (col-error.size())/2, error.c_str());
	    refresh();
            sleep(120);
            continue;
        }

        for(auto c : queues)
            if(gamequeueid == c.configid) gametype = c.description;


        url = "https://global.api.pvp.net/api/lol/static-data/" + server + "/v1.2/champion" + key;

        data = "";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_perform(curl);


        for(auto& c : teamA){
            unsigned position = 0;
            while(!pattern("\"id\":" + c.champid + ",", position)){}
            patternandrecord("\"name\":\"", position, '"', c.champ);
        }

        for(auto& c : teamB){
            unsigned position = 0;
            while(!pattern("\"id\":" + c.champid + ",", position)){}
            patternandrecord("\"name\":\"", position, '"', c.champ);
        }

        url = "https://" + server + ".api.pvp.net/api/lol/" + server + "/v2.5/league/by-summoner/";

        for(auto c = teamA.begin(); c != teamA.end(); ++c){
            url = url + c->sumid;
            if(c != teamA.end()-1) url += ",";
        }

        for(auto c = teamB.begin(); c != teamB.end(); ++c){
            url = url + "," + c->sumid;
        }

        url += "/entry" + key;
        data = "";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_perform(curl);


        for(auto& c : teamA){
            unsigned position = 0;
            unsigned temp;
            if(pattern(c.sumid, position) && (temp = position, pattern("RANKED_SOLO_5x5\"", temp))){
                patternandrecord("\"tier\":\"", position, '"', c.league);
                patternandrecord("\"division\":\"", position, '"', c.division);
                c.position = "(" + c.league + " " + c.division + ")";
            }
            else{
                c.league = "Unranked";
                c.position = "(Unranked)";
            }
        }

        for(auto& c : teamB){
            unsigned position = 0;
            decltype(data.size()) temp;
            if(pattern("\"" + c.sumid + "\"", position) && (temp = position, pattern("RANKED_SOLO_5x5\"", temp))){
                patternandrecord("\"tier\":\"", position, '"', c.league);
                patternandrecord("\"division\":\"", position, '"', c.division);
                c.position = "(" + c.league + " " + c.division + ")";
            }
            else{
                c.league = "Unranked";
                c.position = "(Unranked)";
            }
        }

        //output formatting next

        bool display = (tolower(displayanswer[0]) == 'y' ? true : false);
        unsigned maxlengthA = 0, maxlengthB = 0;

        for(auto c : teamA){
            if(maxlengthA < c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0))
                maxlengthA = c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0);
        }

        for(auto c : teamB){
            if(maxlengthB < c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0))
                maxlengthB = c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0);
        }

        if(maxlengthA == 0) maxlengthA = 15;
        if(maxlengthB == 0) maxlengthB = 15;

        if(maxlengthA + 5 + maxlengthB > col){
            for(auto& c : teamA)
                if(c.position == "(Unranked)")
                    c.position = "(Unrank)";
                else{
                    c.position = "(";
                    c.position.push_back(c.league[0]);
                    c.position.push_back(c.league[1]);
                    c.position.push_back(c.league[2]);
                    c.position += " " + c.division + ")";
                }

            for(auto& c : teamB)
                if(c.position == "(Unranked)")
                    c.position = "(Unrank)";
                else{
                    c.position = "(";
                    c.position.push_back(c.league[0]);
                    c.position.push_back(c.league[1]);
                    c.position.push_back(c.league[2]);
                    c.position += " " + c.division + ")";
                }

            maxlengthA = 0, maxlengthB = 0;

            for(auto c : teamA){
                if(maxlengthA < c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0))
                    maxlengthA = c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0);
            }

            for(auto c : teamB){
                if(maxlengthB < c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0))
                    maxlengthB = c.champ.size() + c.position.size() + 1 + (display ? c.sumname.size()+1 : 0);
            }

            if(maxlengthA == 0) maxlengthA = 15;
            if(maxlengthB == 0) maxlengthB = 15;

        }


	int maxlength = maxlengthA + 5 + maxlengthB;
	int printlines = 4 + (4) + (teamA.size() > teamB.size() ? teamA.size() : teamB.size()); //change parenthesized number for height

	erase();

        mvprintw((row/2)-(printlines/2), (col-strlen(gametype.c_str()))/2, gametype.c_str());

        mvprintw((row/2)-(printlines/2)+2, (col-2-name.size())/2, ("\"" + name + "\"").c_str());

	mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2, "Blue Team");

	mvprintw((row/2)-(printlines/2)+4, (col-maxlength)/2 + maxlength - strlen("Purple Team"), "Purple Team");

        for(unsigned i = 0; i < teamA.size() || i < teamB.size(); ++i){
            if(i < teamA.size()){
                mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2), teamA[i].champ.c_str());
		if(display) mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + teamA[i].champ.size() + 1, teamA[i].sumname.c_str());
		mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA - teamA[i].position.size(), teamA[i].position.c_str());
	    }
            if(i < teamB.size()){
                mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5, teamB[i].champ.c_str());
		if(display) mvprintw((row/2) - (printlines/2)+6+i, ((col-maxlength)/2) + maxlengthA + 5 + teamB[i].champ.size() + 1, teamB[i].sumname.c_str());
		mvprintw((row/2)-(printlines/2)+6+i, ((col-maxlength)/2) + maxlength - teamB[i].position.size(), teamB[i].position.c_str());
            }
        }

	refresh();

        url = "https://" + server + ".api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/" + platformid + "/" + nameid + key;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        while(true){
            sleep(30);

            data = "";
            curl_easy_perform(curl);

            unsigned position = 0;
            string temp;
            if(!patternandrecord("\"gameId\":", position, ',', temp) || temp != gameid){
                sleep(30);
		break;
	    }
        }

        continue;

        curl_easy_cleanup(curl);
        curl_global_cleanup();
	endwin();
        return 0;

    }
}
