//Use the gaviota tablebases to improve play and to ensure mate is given quickly
int canGav(const uint64_t all);
int gavScore(Board b, int* tbIsAv);
int gavWDL(Board b, int* tbIsAv);
int gavWDLSoft(Board b, int* tbIsAv);
void initGav(const char* path);