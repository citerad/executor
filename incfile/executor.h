#define MSG_SIZE 256

void Result(int);
int mkJail(const char *);
int RunComm(int, char**);
int RunExt(int, char**);
void Shutdown();
void MyAbort(int);
void rmJail(char *);
void Signal_Hand(int);
