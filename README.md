# NoC

An attempt at making a chess engine from scratch. Under active development.

### Warnings: 

The TT size is currently a compile time constant set to 120MB, to change it go to hash.h and change the first define. I'll change that later, maybe.

### Use

Download and extract, `$ cd Engine` and then execute `$ make all`, this will generate a binary with the name of the engine.

There are more options in the makefile, change between yes/no to use them. ` $ make help` for further information.

The engine makes use of the UCI protocol, to list the implemented commands type `help`.

### NNUE

NNUE (Efficiently Updatable Neural Networks) are NNs that take less time to evaluate multiple chess positions, because only a few values change with each move, more info [here](https://www.chessprogramming.org/Stockfish_NNUE).

A good NNUE gives a massive elo boost to pretty much any engine, NoC used to have 2500elo and, after reducing the nps by a factor of 5, it can achieve about 3000elo (this also shows how bad the old evaluation was :/). I'm sure that with the implementation of better vectorization instructions (AVX/SSE) much more elo could be won (sf's new nps are about 60%). I may work on that later.

The engine is programmed so that it can use the same NNUE as stockfish, at least the current 512x32x32 version. This networks can be generated using [nodchip's repo](https://github.com/nodchip/Stockfish), or downloaded from [fishtest](https://tests.stockfishchess.org/nns). I've done it this way so that it requires less effort to keep the evaluation up-to-date.

### Tablebases

This engine uses the gaviota tablebases instead of syzygy due to the use of DTM, this is done to ensure mate is given in won situations such as KQvK, and that it is efficient. The tablebases for 3 pieces are already included in gav/, to use them simply execute the engine with `$ pwd == ../Engine` or pass the absolute path as the first argument `$ ./Engine /home/../Engine/gav/`.

To add more tablebases generate them using gaviota or download them from my [mega](https://mega.nz/#F!ieIyCASL!e6TIDQJuQSgjee8VlRwEMQ), they are already compressed and ready to use. Include the files in /gav or pass the directory as an argument as shown before.

You should see a message such as `[+] Using tablebases: 3 4 ~5` where the numbers represent the tablebases available and ~ # means they are incomplete.

The tablebases are currently used to give a fast mate in endgames, and in the search function as an oracle. There is currently no way to not use them besides altering the source code, to do this go to gaviota.c and set `*tbIsAv = 0` in `gavWDL / gavScore` to disable search oracle and DTM queries respectively, or remove the tablebases you don't want to be used.

They aren't of much use when the match uses tablebases or engine scores to award a premature win.

### About

Chess engine written in C. Currently used in lichess.org as 'usr25', though it is offline most of the time. From the testing I've done it should be around 3000elo.

No guarantees are made about the engine. All the testing matches were done using cutechess.

The version corresponds to the month and year of release. 11.19 == Nov 19, unless there is a big change there will be one version max per month.

The engine is under the GPLv3+ License.

### Thanks

Special thanks to [chessprogramming](https://www.chessprogramming.org/Main_Page), [cutechess](https://github.com/cutechess/cutechess), and [lichess-bot](https://github.com/careless25/lichess-bot). Huge thanks to [Miguel balliroca](https://sites.google.com/site/gaviotachessengine/Home/endgame-tablebases-1) for providing the gaviota tablebases and the easy to use API! Also to Ronald de Man, the author of [Cfish](https://github.com/syzygy1/Cfish), whose source code helped me implement sf's NNUE
