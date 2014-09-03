
#if USE_stdlib
#include <stdlib.h>
#endif

char* getpass(const char*);

main(void)
{
    char salt[3];
    long now;
    char pass1[32];
    char pass2[32];
    
    now = time(0);
    salt[0] = now % 25 + 'a';
    salt[1] = (now /= 25) % 25 + 'a';
    salt[2] = 0;
    
    strcpy(pass1, getpass("Password: "));
    strcpy(pass2, getpass("Again: "));

    if(strcmp(pass1, pass2))
    {
	printf("Password mismatch, try again.\n");
	exit(1);
    }
    
    printf("#define MASTER_PASSWORD \"%s\"\n", crypt(pass1, salt));
    exit(0);
}
