# HW 5
Good luck and don't cheat!

For my program, the client starts with "./client" and the server will start with "./server 'N'" with N being the number of record entries you want your server to hold in that session.

For each command please be sure to include a space after the word instead of immediately pressing Enter, to ensure peak performance.


# ./Client

### start 
When inputting start you will be greeted with a second message asking you to input a number between 1 and 4 (inclusive). 
This represents which client pipe you will be opening for the program.
As the client program can be run at up to 4 times simultaneously, it is crucial that you run each client program with a different client pipe.
You must start a client session in order to perform any of the other commands, except exiting.

### alloc
Inputting "alloc " will reserve a record in the server for your client, if there are any available. 
The server will write back which record has been reserved, and the client will store that value in its data table. To access which record is stored where use...


### infotab
"infotab " is an interactive command that will list out a first level of tables for the user to navigate through. 
The user will then input the specific first level table that they want to delve deeper into by entering the table number that is listed.
If you have not yet ran 'alloc' there will be no tables for the user to view so I suggest running 'alloc' first.
After selecting which first level table, the program will numerically list the ID's of the server's indexes.
At first, each ID will have an index of -1. This means that the ID is empty and there isn't a record reserved at that data entry.
These ID's are necessary for a few other commands and while it would be aesthetically pleasing to save them in binary, I made it simple for the user (and myself) by keeping them base 10. 


### dealloc 'ID'
Dealloc requires the ID of the record index you wish to remove. 
The ID should be inputted as a base 10 value.
Dealloc will then remove the entry of the record from the local table as well as free the record on the client side.


### read 'ID'
Read will take the ID inputted by the user, translate it to the desired index in the server, and return the record name stored there.
If an index has only been alloc'd, there will be no name for server to return so instead a message will be sent stating so.
To store a name in an ID'd index use....


### store 'ID' 'Z'
store will take a valid ID that has a record index stored within it and save the string 'Z' in the server.
If the string you want to save must be multiple words, please put them within quotes, or better yet, eliminate the spaces and use dashes or underscores instead (hehe).
String 'Z' can only go be up to 255 characters in size so choose your characters wisely as it cannot be renamed unless you 'dealloc' and then 'alloc' that ID.
## Note:
An ID of '0' is valid in this program but only '0'. '0' is not equal to '00' or any other form of '0', so please don't try or you will break my precious program. Thank you.


### close
When the user finally gets tired of this program, they must use the 'close ' command to offically let the server know that they can clean up all traces of this client from the server. 
This command will free up all memory used in this client session but not actually exit it, incase the user decides to start a new client session for whatever reason.


### exit
Exits the program. Can only call exit if there is no client session running. 



# ./Server
The server must be running in a separate terminal before starting a client to serve what the client programs input to it. While the server does not seem to have any userface, users can access its hidden shell by entering CTRL+C on the keyboard. This shell has a few debugging commands for serverside users:

### list
This command lists all current clients operating on the server along with their unique ID's


### list 'X'
After viewing all clients, users can view all entries per client by inputting this command and replacing 'X' with the clientNo. seen in 'list '.


### dump
Dump will print out all allocated records in the server's database and provide info as to what the names are and which client it is reserved to.

### exit
Allows the user to terminate the server properly as to free up all memory. Preferably this command is called after already closing all other clients associated to it.

