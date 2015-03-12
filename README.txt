README:

Updated from the previous version severly.

- This system is designed for UNIX. It will not port (correctly) to Windows. 

[DESIGNER-LEVEL CHANGES]:

JSON is now used rather than those laborious pattern-matching functions I had before, making the program more effective and
less prone to error. 


[USER-LEVEL CHANGES]:

- The name, champion and team of the person searched for are displayed at the top when displaying the current game stats. 
- The game mode is shown at the top
- A running clock displaying the current game time has been added
- Non-ASCII characters should now be displayed normally

- Post game stats have been added. After a game ends the program attempts to retrieve post-game stats for 2 minutes. If it finds them
it shows these stats for the next 3 minutes, otherwise is exits back to the original loop (searches for a game).
- Among these post game stats are KDA, Wards Placed, Damage Taken, CS per 10, Ally/Enemy Drakes, Ally/Enemy Barons et al.


[BUG FIXES]:

[11TH MARCH 2015]:
- If connection errors occured then the program would exit, this has now hopefully been fixed.
- Sometimes after a game ended and the program went to search for a new game, it would re-find the game just exited, this has hopefully been fixed.
[12TH MARCH 2015]:
- Fixed errors where the program would crash if a champion wasn't recognised (e.g. after a new patch).  




[TO USE]:

- Obtain a Riot Games API key (google this)

- Append your key to ?api_key= in "key.txt"

- The program may not work with non-EUW keys, although I doubt this.

- Keep "key.txt" in the same folder as input.exe.

- Run the program (input.exe)!

- To exit, Press Cntrl+C. 


[ERROR-HANDLING]

- If you run in to issues with the program, make sure you entered the API KEY in to key.txt correctly. 

- Unlikely but the program may not work after game patches. This may fix itself after a day or two once riot updates their stuff.


[TO RE-BUILD]:


- I have included the source codes. You can compile your own version that does not take any input (name, server, display) by adding the desired
values to CurrentGameLoopJSONPi.cpp (as indicated by the comments inside) and compiling this with CurrentGameFunctionsJSON.cpp. The libraries
JSONCPP, ncurses and libcurl will need to be manually downloaded, installed and linked in the compilation. e.g.

g++ -std=c++0x -o main -I/usr/local/include -I/usr/include -L/usr/local/lib -L/usr/lib/ar-linux-gnueabihf 
-lncurses -lcurl CurrentGameLoopJSONPi.cp CurrentGameFunctions.cpp

- Compile CurrentGameLoopJSONPiInput.cpp and CurrentGameFunctionsJSON.cpp together to obtain the already provided input.exe



[FUTURE IMPROVEMENTS]

- Hope to add position of Damage Taken/Damage Dealt in relation to the team. This relies on riot providing such data though.

- Hope to find a way so that name/server/display can be added to a text file and there will be no need for input (stdio and ncurses don't go well together).
