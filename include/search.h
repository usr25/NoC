typedef struct
{
    Board board;
    int value;
    struct Node* parent;
    struct Node* children;
}Node;

Move bestMoveAB(Board b, int depth, int tree);
Move bestMoveBrute(Board b, int depth, int tree);