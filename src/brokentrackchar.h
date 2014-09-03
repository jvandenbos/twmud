class CharTrackerNode;
class CharTrackerIgnoreNode;
class CharTrackerIgnore;
class CharTrackerGroup;
class CharTracker;

extern CharTracker TrackingSystem;

class CharTrackerNode {
 public:
   char name[255];
   long num_objs;
   long level;
   time_t laston;
   char host[512];
   CharTrackerNode *next;
   
   CharTrackerNode();
   ~CharTrackerNode();
   
   void SetName(char *);
   void SetObjs(long);
   void SetLevel(long);
   void SetLaston(time_t);
   void SetHost(char *);
};

class CharTrackerIgnoreNode {
 public:
   CharTrackerGroup *group;
   CharTrackerIgnoreNode *next;
};

class CharTrackerIgnore {
 public:
   int all;
   CharTrackerIgnoreNode *list;
   
   CharTrackerIgnore();
   ~CharTrackerIgnore();
   
   int IsIgnoring(CharTrackerGroup *);
   void AddIgnore(CharTrackerGroup *);
   void IgnoreAll();
   void RemIgnore(CharTrackerGroup *);
   void CopyList(CharTrackerIgnore *);
};

class CharTrackerGroup {
 public:
   CharTrackerNode *members;
   CharTrackerGroup *next;
   CharTrackerIgnore ignore;
   
   CharTrackerGroup();
   ~CharTrackerGroup();
   
   CharTrackerNode *AddChar(char *);
   CharTrackerNode *GetChar(char *);
   void            AddGroup(CharTrackerGroup *);
   void            Save(FILE *);
   long            Count();
};

class CharTracker {
 public:
   CharTrackerGroup *groups;
   
   CharTracker();
   ~CharTracker();
   CharTrackerGroup *GetGroupChar(char *);
   CharTrackerGroup *NewGroupChar(char *);
   void             DeleteCharConst(const char *);
   void             DeleteChar(char *);
   void             UpdateChar(char *, long, long);
   void             UpdateCharLast(char *, time_t); 
   void             UpdateCharHost(char *, char *);
   void             Save();
   void             Load();
   
   //requires stuff from mud
   void             CheckChar(char_data *);
   void             DoCheck(char *, char *);
   void             Show(char_data *, char *);
   void             ShowSingle(char_data *, CharTrackerGroup *,long);
   int              UpdateCharFull(char_data *);
   int              UpdateCharFull(CharTrackerNode *);
};
