- I am a novice programmer. The code used is inefficient, cuts corners and has potential security issues 
(for instance, SSL verification is turned off), but it does get the job done. I was also too lazy to read 
up on JSON and the code instead interprets the data sent back as one long string and picks out the relevant 
info with pattern-matching functions. As a result the program may become unusable if Riot changes the 
formatting of the data sent back. I will rewrite the code to use JSON at some point, or you can feel tree 
to rewrite as you like. 


- This system is designed for UNIX. It will not port (correctly) to Windows. 








TO GET IT WORKING:



- Obtain your own API key from Riot's website (Google this).



- The program may not work with non-EUW keys, although I doubt this.



- Append your obtained API key to the end of "?api_key=" found in keyinput.txt.



- Make sure the executable and keyinput.txt are in the same folder.



- Run the executable.



- Once the loop starts, the only way to exit is Cntrl+Z.



- If you keep getting the error "No data, connection error?" make sure you entered the API Key correctly.












ADDITIONAL:



- The League Divisions shown (e.g. Challenger I) automatically shorten if the data would go off the end of 
the screen (likely to happen if you play, for example, with Challenger I Heimerdingers!).



- automated.cpp contains source code to create an automated display program (i.e. no user input required). This needs 
to be personalised and compiled yourself.



- readinput.cpp is the source code for a display which requires user input (e.g. name, server).



- CurrentGameFunctions.cpp is a source file needed to be included in compiling automated.cpp or readinput.cpp



- If you want to compile the source code yourself, make sure to include CurrentGameFunctions.cpp and install 
the curl and ncurses libraries, making sure to link to the correct headers and libraries, 

e.g:

g++ -std=c++0x -o main -I/usr/local/include -I/usr/include -L/usr/local/lib -L/usr/lib/ar-linux-gnueabihf 
-lncurses -lcurl automated.cpp CurrentGameFunctions.cpp
