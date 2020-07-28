uint64_t perft(Board b, const int depth, int divide);
uint64_t perftRecursive(Board b, const int depth);
uint64_t perftMovegen(Board b, const int depth, const int divide);
int hashPerft(Board b, const int depth, const uint64_t prevHash);