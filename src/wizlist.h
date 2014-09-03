struct wiznest {
  char *name;
  char *title; 
};

struct wiznode {
  struct wiznest stuff[150];
};

struct wizlistgen {
   int number[10];
   struct wiznode lookup[10];
};

