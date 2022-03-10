# Webscoket Streamed Doom

Note: For this to work you need to obtain a WAD file (which is shareware),
just google for (doom1.wad)

## Build the Game

`cd doomgeneric && make -f Makefile.ws`

## Run

`./doomgeneric`

## Run the simple client

`cd doomgeneric/websocket_client && yarn && yarn start`

Visit [http://localhost:1234](http://localhost:1234)

## TODO

- [ ] Controls over the wire
- [ ] Compression
