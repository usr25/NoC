/* Use the gaviota tablebases to improve play and to give mate */
int canGav(const uint64_t all);
int gavScore(Board b, int* tbIsAv);
void initGav(const char* path);