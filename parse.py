"""
This script turns each position in a list of pgns into it's fen and a tag
with the result of the game, used to tune the engine
"""

import chess.pgn as pgn

def result(s):
    if s[0] == '0':
        return 'b'
    if s[0] == '1' and s[1] == '-':
        return 'w'
    elif s[0] == '1' and s[1] == '/':
        return 'd'
    return None

def ended_in_cm(res, game):
    if res == 'd':
        return False
    try:
        game.headers["Termination"]
    except KeyError:
        return True
    return False

#TODO: Get a percentage of the parsed games (maybe based on number of lines)
def parse(pgns, to):
    print("[+] Parsing games")
    cnt = 0
    with open(pgns, "r") as read, open(to, "w") as write:
        game = 1
        while game:
            game = pgn.read_game(read)
            if game == None:
                continue
            board = game.board()
            res = result(game.headers["Result"])
            if res == None:
                continue
            l = list(game.mainline_moves())

            #Don't read up to checkmate, but don't cut too soon either
            if ended_in_cm(res, game):
                l = l[:-7]
            #If it is a draw with a lot of moves (probably 50fold rep) don't read all moves
            elif res == 'd' and len(l) > 160:
                l = l[:-25]

            #Don't read the first moves, especially if it ended in a win
            #To avoid possible biases
            start = 4 if res == 'd' else 10

            for move in l[:start]:
                board.push(move)

            cnt += len(l[start:])
            buff = []
            for move in l[start:]:
                board.push(move)
                buff.append("{},{}".format(board.fen(), res))
            write.write("\n".join(buff))


    print("[+] Added {} positions".format(cnt))

def main():
    parse("games.pgn", "fen.csv")

if __name__ == '__main__':
    main()