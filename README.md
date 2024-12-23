# Multiplayer Tetris
This project is to look into client/server architecture.
The project consists of 2 parts: the client and server. These parts can be found int their respecitve folders.
The server code is responsible for playing the actual game (don't trust the client). 
The client only displays the information that is send by the server

## Running the code
Each of the folders has a `compile_and_run.sh` script. Simply run these scripts to setup the server and client. 
The client will require a `.env` file with the IP and port of the server. The `.env_example` can be used for this and contains the ip and port to run locally

# client/server communication
Communication between client and server is done by sending any commands in the following format: 
```
Identifier args
```
Note the seperation between the identifier and args is a space.
Whenever a request is made and the receiver needs to send a response back it must use the same identifier along with its response.

Whenever a client requests something from the server, but is not registered it will receive a "NOCONN" message

# Connections
## Connect
Establishes connection from client to server
- Identifier:   CONNECT
- Args:         OK / ERR

Client -> Server: <br />
    "CONNECT <br />"
Server -> Client response: <br />
    "CONNECT OK" <br /> 
    "CONNECT ERR"  <br />

## Ping
The server will periodically ping the client. If the client does not respond the connection will be terminated
- Identifier:   PING 
- Args:         OK / <int:ping>
                where the <int:ping> is the known ping on the server in ms 

Server -> Client: <br />
    "PING <int:ping>"  <br />
Client -> Server: <br />
    "PING OK" <br />

# Game
## Start 
Sends a request to start the game
- Identifier:   START
- Args:         OK / Err

Client -> Server: <br />
    "START" <br />
Server -> Client:  <br />
    "START OK"  <br />
    "START ERR"  <br />

## Controls
Sends an input from the client to the server
- Identifier:   ACTION
- Args:         l / r / d / q / e / s

Client -> Server: <br />
    "ACTION <char:action>"  <br />
Sever response:  <br />
    None <br />

## Game state
After every server update the board is send back to the client
- Identifier:   STATE
- Args:         SP <string:game_state> / MP <string:game_state> <string:game_state>
                The SP/MP identifies the game type and the amount of game states send back is determined by the type.
                A game state is formatted as <board>_<game_over>_<score>_<piece_queue>_<held_piece>
                For the board the rows and columns are seperated by ; and , respectively.
                For the piece queue the items are seperated by a comma.


Server -> Client: <br />
    "STATE MP 0,0, .. ,0; .. ;0,0 .. ,0;_<int:game_over>_<int:score>_0,1,2_<int:held_piece>"  <br />
    "STATE MP 0,0, .. ,0; .. ;0,0 .. ,0;_<int:game_over>_<int:score>_0,1,2_<int:held_piece>"  <br />
Client response: <br />
    None <br />

## Muliplayer Queue
Send a request for a multiplayer match
- Identifier: QUEUE
- Client args: ENTER / CANCEL
- Server args: Ok / Err

Client -> Server: <br />
    "QUEUE ENTER" <br />
    "QUEUE CANCEL" <br />
Server -> Client: <br />
    "QUEUE OK" <br />
    "QUEUE ERR" <br />


Send a confirmation for started multiplayer match <br />
Server -> Client:
- Identifier: MATCH
- args: P1/P2
Server -> Client: <br />
    "MATCH P1" <br />
    "MATCH P2" <br />

