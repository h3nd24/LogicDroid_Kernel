/* 
  This is just a dummy Monitor_module.c
  You should generate Monitor_module.c yourself using Monitor Generator  
*/

#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/mutex.h>

#define MAX_APPLICATION 100000

typedef struct tHistory {
  char ***propositions;
  char **atoms;
  int **time_tag;
  long timestamp;
} History;

#define ROOT_UID 0
#define INTERNET_UID 1100
#define SMS_UID 1101
#define LOCATION_UID 1102
#define CONTACT_UID 1103

static char notInitialized = 1;
static int mapping[MAX_APPLICATION];
static char *system;
static char *trusted;
static char** relations;
static int relationSize = 0;
static int app_num;
static int module_policyID = 6;

static int num_temp = 1;
static long metric[1] = {10};

static int currentHist = 0;

int LogicDroid_Module_renewMonitorVariable(int* UID, int varCount, char value, int rel);
int LogicDroid_Module_initializeMonitor(int *UID, int appCount);
int LogicDroid_Module_checkEvent(int rel, int *UID, int varCount, long timestamp);

extern void LogicDroid_registerMonitor(int (*renewMonitorVariable)(int*, int, char, int),
        int (*initializeMonitor) (int*, int),
        int (*checkEvent)(int, int*, int, long),
        char **module_relations, int module_relationSize, int module_policyID);
extern void LogicDroid_unregisterMonitor(void);
extern void LogicDroid_setIDs(int, int);

History* History_Constructor(long timestamp);
void History_Reset(History *h);
void History_insertEvent(History *h, int rel, int idx);
char History_Process(History *next, History *prev);
void History_Dispose(History *h);

DEFINE_MUTEX(lock);

History **hist = NULL;

int LogicDroid_Module_renewMonitorVariable(int *UID, int varCount, char value, int rel) {
  int varIdx = 0;
  int mul = 1;
  int i = 0;


  if (notInitialized)
  {
    return 0;
  }

  mutex_lock(&lock);
  for (i = varCount - 1; i >= 0; mul *= app_num, i--)
  {
    varIdx += mapping[UID[i]] * mul;
  }
  hist[currentHist]->atoms[rel][varIdx] = value;
  mutex_unlock(&lock);
  return 0;
}

int LogicDroid_Module_initializeMonitor(int *UID, int appCount) {
  int appIdx = 5;
  int i;
  mutex_lock(&lock);

  printk("initializing Monitor for %d applications\n", appCount);

  mapping[0] = 0;
  mapping[INTERNET_UID] = 1;
  mapping[SMS_UID] = 2;
  mapping[LOCATION_UID] = 3;
  mapping[CONTACT_UID] = 4;
  app_num = appCount + 5;

  kfree(system);
  kfree(trusted);

  system = (char*) kmalloc(sizeof(char) * app_num, GFP_KERNEL);
  memset(system, 0, sizeof(char) * app_num);
  trusted = (char*) kmalloc(sizeof(char) * app_num, GFP_KERNEL);
  memset(trusted, 0, sizeof(char) * app_num);

  for (i = 0; i < appCount; i++)
  {
    mapping[UID[i]] = appIdx++;
  }

  if (hist == NULL)
  {
    hist = (History**) kmalloc(sizeof(History*) * 2, GFP_KERNEL);
    hist[0] = NULL;
    hist[1] = NULL;
  }

  History_Dispose(hist[0]);
  History_Dispose(hist[1]);
  hist[0] = History_Constructor(0);
  hist[1] = History_Constructor(0);
  History_Reset(hist[0]);
  History_Reset(hist[1]);

  LogicDroid_setIDs(3, INTERNET_UID);
  currentHist = 0;
  notInitialized = 0;
  mutex_unlock(&lock);
  return module_policyID;
}

int LogicDroid_Module_checkEvent(int rel, int *UID, int varCount, long timestamp) {
  int varIdx = 0;
  int mul = 1;
  int i = 0;
  char result;
  if (notInitialized)
  {
    return 0;
  }

  mutex_lock(&lock);
  History_Reset(hist[!currentHist]);
  currentHist = !currentHist;
  hist[currentHist]->timestamp = timestamp;

  for (i = varCount - 1; i >= 0; mul *= app_num, i--)
  {
    varIdx += mapping[UID[i]] * mul;
  }
  History_insertEvent(hist[currentHist], rel, varIdx);
  result = History_Process(hist[currentHist], hist[!currentHist]);
  if (result)
  {
    currentHist = !currentHist;
  }
  mutex_unlock(&lock);

  return result;
}

History* History_Constructor(long timestamp) {
  History *retVal = (History*) kmalloc(sizeof(History), GFP_KERNEL);

  retVal->atoms = (char**) kmalloc(sizeof(char*) * 4, GFP_KERNEL);
  retVal->atoms[0] = (char*) kmalloc(sizeof(char) * app_num * app_num, GFP_KERNEL); // trans
  retVal->atoms[1] = system; // system
  retVal->atoms[2] = trusted; // trusted
  retVal->atoms[3] = (char*) kmalloc(sizeof(char) * app_num * app_num, GFP_KERNEL); // call
  retVal->propositions = (char***) kmalloc(sizeof(char**) * 2, GFP_KERNEL);
  retVal->propositions[0] = (char**) kmalloc(sizeof(char*) * 7, GFP_KERNEL);
  retVal->propositions[1] = (char**) kmalloc(sizeof(char*) * 7, GFP_KERNEL);
  retVal->propositions[1][0] = retVal->atoms[0];

  retVal->propositions[0][0] = (char*) kmalloc(sizeof(char), GFP_KERNEL);
  retVal->propositions[0][1] = (char*) kmalloc(sizeof(char) * app_num, GFP_KERNEL);
  retVal->propositions[0][2] = retVal->atoms[0];
  retVal->propositions[0][3] = (char*) kmalloc(sizeof(char) * app_num, GFP_KERNEL);
  retVal->propositions[0][4] = retVal->atoms[1];
  retVal->propositions[0][5] = (char*) kmalloc(sizeof(char) * app_num, GFP_KERNEL);
  retVal->propositions[0][6] = retVal->atoms[2];
  retVal->propositions[1][1] = retVal->atoms[3];
  retVal->propositions[1][2] = (char*) kmalloc(sizeof(char) * app_num * app_num, GFP_KERNEL);
  retVal->propositions[1][3] = (char*) kmalloc(sizeof(char) * app_num * app_num * app_num, GFP_KERNEL);
  retVal->propositions[1][4] = (char*) kmalloc(sizeof(char) * app_num * app_num, GFP_KERNEL);
  retVal->propositions[1][5] = retVal->atoms[0];
  retVal->propositions[1][6] = retVal->atoms[3];

  retVal->time_tag = (int**) kmalloc(sizeof(int*) * num_temp, GFP_KERNEL);
  retVal->time_tag[0] = (int*) kmalloc(sizeof(int) * app_num * app_num, GFP_KERNEL) ;

  retVal->timestamp = timestamp;

  return retVal;
}
void History_Reset(History *h)
{
  memset(h->atoms[0], 0, sizeof(char) * app_num * app_num);
  memset(h->atoms[3], 0, sizeof(char) * app_num * app_num);

  memset(h->propositions[0][0], 0, 1);
  memset(h->propositions[0][1], 0, app_num);
  memset(h->propositions[0][3], 0, app_num);
  memset(h->propositions[0][5], 0, app_num);
  memset(h->propositions[1][2], 0, app_num * app_num);
  memset(h->propositions[1][3], 0, app_num * app_num * app_num);
  memset(h->propositions[1][4], 0, app_num * app_num);

  memset(h->time_tag[0], 0, sizeof(int) * app_num * app_num);
}

void History_insertEvent(History *h, int rel, int idx) {
  h->atoms[rel][idx] = 1;
}

char History_Process(History *next, History *prev) {
  int y, x, z;
  long delta;

  // [4] : DiamondDot_10(trans(x, z))
  for (x = 0; x < app_num; x++) {
    for (z = 0; z < app_num; z++) {
      next->propositions[1][4][x * app_num + z] = 0;
      next->time_tag[0][x * app_num + z] = 0;
      delta = (next->timestamp - prev->timestamp);
      if (delta <= metric[0])
      {
        next->propositions[1][4][x * app_num + z] = prev->propositions[1][5][x * app_num + z];
        next->time_tag[0][x * app_num + z] = (int) delta;
        if (!next->propositions[1][4][x * app_num + z] && prev->time_tag[0][x * app_num + z] + delta <= metric[0])
        {
          next->propositions[1][4][x * app_num + z] = prev->propositions[1][4][x * app_num + z];
          if (next->propositions[1][4][x * app_num + z]) next->time_tag[0][x * app_num + z] += prev->time_tag[0][x * app_num + z];
        }
      }

    }
  }

  // [3] : (DiamondDot_10(trans(x, z)) and call(z, y))
  for (x = 0; x < app_num; x++) {
    for (z = 0; z < app_num; z++) {
      for (y = 0; y < app_num; y++) {
        next->propositions[1][3][x * app_num * app_num + z * app_num + y] = next->propositions[1][4][x * app_num + z] && next->propositions[1][6][z * app_num + y];
      }
    }
  }

  // [2] : exist_z((DiamondDot_10(trans(x, z)) and call(z, y)))
  for (x = 0; x < app_num; x++) {
    for (y = 0; y < app_num; y++) {
      for (z = 0; z < app_num && !next->propositions[1][2][x * app_num + y]; z++) {
        next->propositions[1][2][x * app_num + y] = next->propositions[1][2][x * app_num + y] || next->propositions[1][3][x * app_num * app_num + z * app_num + y];
      }

    }
  }

  // [0] : (call(x, y) or exist_z((DiamondDot_10(trans(x, z)) and call(z, y))))
  for (x = 0; x < app_num; x++) {
    for (y = 0; y < app_num; y++) {
      next->propositions[1][0][x * app_num + y] = next->propositions[1][1][x * app_num + y] || next->propositions[1][2][x * app_num + y];
    }
  }

  // [3] : !(system(x))
  for (x = 0; x < app_num; x++) {
    next->propositions[0][3][x] = !next->propositions[0][4][x];
  }

  // [5] : !(trusted(x))
  for (x = 0; x < app_num; x++) {
    next->propositions[0][5][x] = !next->propositions[0][6][x];
  }

  // [1] : (trans(x, sms) and !(system(x)) and !(trusted(x)))
  for (x = 0; x < app_num; x++) {
    next->propositions[0][1][x] = next->propositions[0][2][x * app_num + 2] && next->propositions[0][3][x] && next->propositions[0][5][x];
  }

  // [0] : exist_x((trans(x, sms) and !(system(x)) and !(trusted(x))))
  for (x = 0; x < app_num && !next->propositions[0][0][0]; x++) {
    next->propositions[0][0][0] = next->propositions[0][0][0] || next->propositions[0][1][x];
  }


  return next->propositions[0][0][0];
}

// Additional function to clean up garbage
void History_Dispose(History *h) {
  int i;

  if (h == NULL) return;

  // Don't remove the actual data aliased by the variables
  h->propositions[0][2] = NULL;
  h->propositions[0][4] = NULL;
  h->propositions[0][6] = NULL;
  h->propositions[1][1] = NULL;
  h->propositions[1][5] = NULL;
  h->propositions[1][6] = NULL;
  h->propositions[1][0] = NULL;
  h->atoms[1] = NULL;
  h->atoms[2] = NULL;

  // clean propositions
  for (i = 0; i < 7; i++)
  {
    kfree(h->propositions[0][i]);
  }
  kfree(h->propositions[0]);
  for (i = 0; i < 7; i++)
  {
    kfree(h->propositions[1][i]);
  }
  kfree(h->propositions[1]);
  kfree(h->propositions);

  // clean atoms
  for (i = 0; i < 4; i++)
  {
    kfree(h->atoms[i]);
  }
  kfree(h->atoms);

  // clean temporal metric
  for (i = 0; i < num_temp; i++)
  {
    kfree(h->time_tag[i]);
  }
  kfree(h->time_tag);

  // finally free the history reference
  kfree(h);
}

int __init init_monitor_module(void)
{
  printk(KERN_INFO "Attaching the policy exist_x((trans(x, sms) and !(system(x)) and !(trusted(x))))\n");
  relations = (char**) kmalloc(sizeof(char*) * 4, GFP_KERNEL);
  relations[0] = "trans";
  relations[1] = "system";
  relations[2] = "trusted";
  relations[3] = "call";
  relationSize = 4;
  LogicDroid_registerMonitor(&LogicDroid_Module_renewMonitorVariable,
    &LogicDroid_Module_initializeMonitor,
    &LogicDroid_Module_checkEvent,
    relations, relationSize, module_policyID);
  return 0;
}

void __exit cleanup_monitor_module(void)
{
  printk(KERN_INFO "Detaching the policy from the monitor stub in kernel\n");
  kfree(relations);
  History_Dispose(hist[0]);
  History_Dispose(hist[1]);
  kfree(system);
  kfree(trusted);
  LogicDroid_unregisterMonitor();
}

module_init(init_monitor_module);
module_exit(cleanup_monitor_module);
