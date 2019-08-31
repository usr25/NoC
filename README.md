# Engine

An attempt at making a chess engine from scratch. Under development.

### Warnings: 

Currently the TTs give some trouble, they are disabled by default.

### Use

Download and extract, `$ cd Engine` and then execute `$ make all`, this will generate a binary named 'optimized'.

### About

Chess engine written in C. Currently used in lichess.org as 'usr25', no ranked matches thus far

This engine uses the gaviota tablebases to play better endgames, by default all 3 pieces tablebases are provided. They are only used to generate the winning / drawing move when possible.

### Thanks

Special thanks to [chessprogramming](https://www.chessprogramming.org/Main_Page), [cutechess](https://github.com/cutechess/cutechess), and [lichess-bot](https://github.com/careless25/lichess-bot). Thanks to [Miguel balliroca](https://sites.google.com/site/gaviotachessengine/Home/endgame-tablebases-1) for providing the gaviota tablebases and the easy to use API!