README:

- The exectuable is compiled for UNIX. It will not work on Windows. 
- I AM A PROGRAMMING NOOB! The source code is probably inefficient and is largely uncommented, while the program itself could be riddled with bugs. 



[TO USE THE PROGRAM] [IMPORTANT!!]:

DOWNLOAD THE APPROPRIATE LIBRARIES!

Unfortunately a few external libraries are needed to get the executable to run: libcurl, JSON, ncursesw and boost-program-options. 
JSON and boost-program-options are linked statically in to the program so you may only need to install libcurl and ncurses. 
Make sure to update your cache and run these commands:


sudo apt-get install libjsoncpp-dev
sudo apt-get install libncursesw5-dev
sudo apt-get libboost-program-options1.50-dev

Libcurl needs to be downloaded from the top of this link:

http://curl.haxx.se/download.html

And extracted/install in the normal way:

tar -zxvf curl-7.51.0.tar.gz 
cd curl-7.51.0 
make 
make install


After that:

- Obtain a Riot Games API key (google this)
- Append your key to "key=?api_key=" in "Data_Files/config.txt"
- The program may not work with non-EUW keys, although I doubt this.
- Keep "Data_Files/config.txt" in the same folder as leagueTracker
- Run the program (leagueTracker)!
- To exit, Press Cntrl+C. 




[CONFIGURATION FILE OPTIONS]

key: This is where the API key is stored.
askInfo: If set to 1 the program asks for user input before finding game information. If set to 0 the program will use the name, server and display answer below provided below. 
name: Name of the summoner (used if askInfo is 0).
server: Server of the summoner (used if askInfo is 0).
showNames: If set to 1 the program shows summoner names when displaying game information. If set to 0 no summoner names are shown, which is ideal for smaller width screens (used if askInfo is 0).




[NORMAL OPERATION]

1) If askInfo is set to 1, the program will ask for a name, server, and whether to display summoner names. Otherwise the program tries to find game information on startup.
2) If the player on the server is not found, the program will exit.
3) If the player on the server is found, it will search for their current game. If a game is found it will display current game data. If a game is not found, it will the say that the player is not in a game until a game is found.
4) When the game ends, the program attempts to receive post-game data for two minutes. If it is found it will display it for three minutes.
5) Once the three minutes is up or no post-game data was found, it will go back to searching for a current game of the selected player.



[ERROR-HANDLING]

- If you run in to issues with the program, make sure you entered the API KEY in to config.txt correctly. 
- Unlikely but the program may not work after game patches. This may fix itself after a day or two once riot updates their stuff.


[BUG FIXES/UPDATES]:

[11TH MARCH 2015]:
- If connection errors occured then the program would exit, this has now hopefully been fixed.
- Sometimes after a game ended and the program went to search for a new game, it would re-find the game just exited, this has hopefully been fixed.
[12TH MARCH 2015]:
- Fixed errors where the program would crash if a champion wasn't recognised (e.g. after a new patch).  
[26TH MAY 2015]:
- System now deals with status errors sent back by Riot, which would cause anomalous behaviour before. 
[30TH MAY 2015]:
- Fixed an issue where league/division wasn't shown properly in Hexakill game-mode. 
[21ST JUNE 2015]:
- Fixed a bug where program would crash when searching for post-game stats. 
[4TH AUGUST 2015]:
- Improved source code somewhat.
- Updated to include Butcher's Bridge gamemodes.
[22ND APRIL 2016 - gee long time]:
- Improved response to error codes. Fixed errors caused by Riot changing how response body shows errors.
- Updated game types to include dynamic queue.
[22ND NOVEMBER 2016]
- Improved source code slightly and consolidated all user information in to one config file.
- The config file now allows the user to specify whether the program should ask for user input and, if not, uses the provided summoner information.
- The source code now should compile fine on both Windows and UNIX operating systems. 
- Updated game types to include flex queue. 



[FUTURE IMPROVEMENTS]

- Hope to add position of Damage Taken/Damage Dealt in relation to the team. This relies on riot providing such data though.


[USING THE SOURCE CODE!]

- I have added the source code for compiling your own version of the program. The source code should compile fine on both Windows and UNIX operating systems.
- Before compiling you will need the libraries jsoncpp, ncursesw, libcurl and boost-program-options downloaded (see [To use the program] above to understand how to get these). 
- You must compile main.cpp with functions.cpp while also linking the appropriate libraries. e.g. (on UNIX)

g++ -std=c++11 -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/ main.cpp functions.cpp -lcurl -lncursesw -ljsoncpp -lboost_program_options -o leagueTracker

- The header file functions.h must be in the same directory as the source code main.cpp. 

