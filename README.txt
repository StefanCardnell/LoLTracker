README:

- This system is designed for UNIX. It will not port (correctly) to Windows. 
- I AM A PROGRAMMING NOOB! The source code is probably inefficient and is largely uncommented, while the program itself could be riddled with bugs. 


[TO USE THE PROGRAM] [IMPORTANT!!]:

DOWNLOAD THE APPROPRIATE LIBRARIES!

Unfortunately a few external libraries are needed to get this to run: libcurl, JSON and ncursesw. Make sure to update your cache and run these two commands:

sudo apt-get install libjsoncpp-dev
sudo apt-get install libncursesw5-dev

The last one, libcurl, needs to be downloaded from the top of this link:

http://curl.haxx.se/download.html

And extracted/install in the normal way:

tar -zxvf curl-7.42.1.tar.gz 
cd curl-7.42.1 
make 
make install


After that:

- Obtain a Riot Games API key (google this)

- Append your key to ?api_key= in "key.txt"

- The program may not work with non-EUW keys, although I doubt this.

- Keep "key.txt" in the same folder as input.exe.

- Run the program (input.exe)!

- To exit, Press Cntrl+C. 




[NORMAL OPERATION]

1) The program will ask for a name, server, and whether to display summoner names

2) If the player on the server is not found, the program will exit.

3) If the player on the server is found, it will search for their current game. If a game is found it will display current game data. If a game is not found, it will the say that the player is not in a game until a game is found.

4) When the game ends, the program attempts to receive post-game data for two minutes. If it is found it will display it for three minutes.

5) Once the three minutes is up or no post-game data was found, it will go back to searching for a current game of the selected player.



[ERROR-HANDLING]

- If you run in to issues with the program, make sure you entered the API KEY in to key.txt correctly. 

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



[FUTURE IMPROVEMENTS]

- Hope to add position of Damage Taken/Damage Dealt in relation to the team. This relies on riot providing such data though.

- Hope to find a way so that name/server/display can be added to a text file and there will be no need for input (stdio and ncurses don't go well together).


[USING THE SOURCE CODE!]

- I have added the source code for compiling your own version of the program. 

- Before compiling you will need JSONCPP, ncursesw and libcurl manually downloaded, and then linked in the compilation (see [To use the program] above to understand how to get these). 

- You must compile CurrentGameLoopJSONPiInput.cpp with CurentGameFunctionsJSON.cpp while also linking the appropriate libraries. e.g. 

g++ -std=c++0x -o input -I/usr/local/include -I/usr/include -I/usr/include/jsoncpp/  CurrentGameLoopJSONPiInput.cpp CurrentGameFunctionsJSON.cpp -lcurl -ljsoncpp -lncursesw

- (replace -std=c++0x with -std=c++11 if your compiler can support it)

- The header file Functions.h must be in the same directory as the source code. 

