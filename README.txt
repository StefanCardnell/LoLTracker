README:

- This system is designed for UNIX. It will not port (correctly) to Windows. 



[BUG FIXES]:

[11TH MARCH 2015]:
- If connection errors occured then the program would exit, this has now hopefully been fixed.
- Sometimes after a game ended and the program went to search for a new game, it would re-find the game just exited, this has hopefully been fixed.
[12TH MARCH 2015]:
- Fixed errors where the program would crash if a champion wasn't recognised (e.g. after a new patch).  
[26TH MAY 2015]:
- System now deals with status errors sent back by Riot, which would cause anomalous behaviour before. 



IMPORTANT!!!!!!!!: [TO USE]:

DOWNLOAD THE APPROPRIATE LIBRARIES!

Unfortunately a few external libraries are needed to get this to run: libcurl, JSON and ncursesw. Make sure to update your cache and
run these two commands:

sudo apt-get install libjsoncpp-dev
sudo apt-get install libncursesw5-dev

The last one, libcurl, needs to be downloaded from the top of this link:

http://curl.haxx.se/download.html

And extracted/installed in the normal way:

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


[ERROR-HANDLING]

- If you run in to issues with the program, make sure you entered the API KEY in to key.txt correctly. 

- Unlikely but the program may not work after game patches. This may fix itself after a day or two once riot updates their stuff.



[FUTURE IMPROVEMENTS]

- Hope to add position of Damage Taken/Damage Dealt in relation to the team. This relies on riot providing such data though.

- Hope to find a way so that name/server/display can be added to a text file and there will be no need for input (stdio and ncurses don't go well together).

- Hope to add separate error messages for when there are issues on Riot's end. 
