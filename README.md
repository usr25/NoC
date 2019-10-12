# NoC 0.1

An attempt at making a chess engine from scratch. Under active development. I haven't decided on a name, for the time being it is **NoC**.

### Warnings: 

The TT size is currently a compile time constant set to 120MB, to change it go to hash.h and change the first and second defines. I'll change it later

### Use

Download and extract, `$ cd Engine` and then execute `$ make all`, this will generate a binary with the name of the engine.

The engine makes use of the UCI protocol, to list the implemented commands type `help`.

### Tablebases

This engine uses the gaviota tablebases instead of syzygy due to the use of DTM, this is done to ensure mate is given in won situations such as KQvK, and that it is efficient. The tablebases for 3 pieces are already included in gav/, to use them simply execute the engine with `$ pwd == ../Engine` or pass the absolute path as the first argument `$ ./Engine /home/../Engine/gav/`.

To add more tablebases generate them using gaviota or download them from my [mega](https://mega.nz/#F!ieIyCASL!e6TIDQJuQSgjee8VlRwEMQ), they are already compressed and ready to use. Include the files in /gav or pass the directory as an argument as shown before.

You should see a message such as `[+] Using tablebases: 3 4 ~5` where the numbers represent the tablebases available and ~ # means they are incomplete.

The tablebases are currently used to give a fast mate in endgames, and in the search function as an oracle. There is currently no way to not use them besides altering the source code, to do this go to gaviota.c and set `*tbIsAv = 0` in `gavWDL / gavScore` to disable search oracle and DTM queries respectively, or remove the tablebases you don't want to be used.

### About

Chess engine written in C. Currently used in lichess.org as 'usr25'. From the testing I've done it should be around 2100 elo.

No guarantees are made about the engine. All the testing matches were done using cutechess.

### Thanks

Special thanks to [chessprogramming](https://www.chessprogramming.org/Main_Page), [cutechess](https://github.com/cutechess/cutechess), and [lichess-bot](https://github.com/careless25/lichess-bot). Huge thanks to [Miguel balliroca](https://sites.google.com/site/gaviotachessengine/Home/endgame-tablebases-1) for providing the gaviota tablebases and the easy to use API!