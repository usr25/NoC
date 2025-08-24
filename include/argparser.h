typedef struct
{
	char** args;
	char* train;
	char* nnue;
	char* gaviota;
	int numArgs;
} Arguments;

Arguments parseArguments(int argc, char** argv);