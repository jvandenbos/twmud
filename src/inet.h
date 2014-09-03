#ifdef __cplusplus
extern "C"
{
#endif
    
unsigned long inet_addr(const char* cp);
int inet_network(const char* cp);
struct in_addr inet_makeaddr(int net, int lna);
int inet_lnaof(struct in_addr in);
int inet_netof(struct in_addr in);
char* inet_ntoa(struct in_addr in);

#ifdef __cplusplus
};
#endif

