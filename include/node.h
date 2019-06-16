
typedef struct
{
    Board board;
    int value;
    struct Node* parent;
    struct Node* children;
}Node;