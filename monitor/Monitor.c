#include "Monitor.h"
#include<linux/linkage.h> 	// to handle system call interface
#include<linux/kernel.h>
#include<linux/slab.h>		// for memory allocation in kernel
#include<linux/uaccess.h>
#include<linux/time.h> 		// for do_gettimeofday

static char noModule = 1;
static int LogicDroid_CallRelationID = -1;
static int LogicDroid_INTERNET_UID = -1;
static int* localUID = NULL;
static int localAppCount = 0;
static char** relations;
static int relationSize = 0;
static int kernel_policyID = 0;

int (*ptr2renewMonitorVariable)(int*, int, char, int) = NULL;
int (*ptr2initializeMonitor) (int*, int) = NULL;
int (*ptr2checkEvent)(int, int*, int, long) = NULL;

int LogicDroid_renewMonitorVariable(int* UID, int varCount, char value, int rel);
int LogicDroid_initializeMonitor(int *UID, int appCount);
int LogicDroid_getRelationName(int ID, char __user *relationName);
int LogicDroid_getCallRelationID(void) {return LogicDroid_CallRelationID;}
int LogicDroid_getInternetUID(void) {return LogicDroid_INTERNET_UID;}

// #######################################################################################
// #                            Interface to System Calls                                #
// #######################################################################################
asmlinkage int sys_LogicDroid_checkChain(int policyID, int caller, int target)
{
	int UID[2] = {caller, target};
	struct timeval tv;
	do_gettimeofday(&tv);
	if (noModule) {return NO_MONITOR;}
	if (kernel_policyID != policyID) return POLICY_MISMATCH;
	int result = LogicDroid_checkEvent(LogicDroid_CallRelationID, UID, 2, (tv.tv_sec));
	printk("checkChain from %d to %d: %d %d (%d)\n", caller, target, kernel_policyID, policyID, result);
	return result;
}

asmlinkage int sys_LogicDroid_initializeMonitor(int __user *UID, int count)
{
	return LogicDroid_initializeMonitor(UID, count);
}

asmlinkage int sys_LogicDroid_modifyStaticVariable(int policyID, int rel, int UID, char value)
{
	if (noModule) return NO_MONITOR;
	if (kernel_policyID != policyID) return POLICY_MISMATCH;
	return LogicDroid_renewMonitorVariable(&UID, 1, value, rel);
}

asmlinkage int sys_LogicDroid_getRelationName(int ID, char __user *relationName)
{
	
	return LogicDroid_getRelationName(ID, relationName);
}

asmlinkage int sys_LogicDroid_isMonitorPresent(void)
{
	
	return !noModule;
}
// #######################################################################################

void LogicDroid_registerMonitor(int (*renewMonitorVariable)(int*, int, char, int), 
				int (*initializeMonitor) (int*, int),
				int (*checkEvent)(int, int*, int, long),
				char **module_relations, int module_relationSize, int module_policyID)
{
  ptr2renewMonitorVariable = renewMonitorVariable;
	ptr2initializeMonitor = initializeMonitor;
	ptr2checkEvent = checkEvent;
	noModule = 0;
	relationSize = module_relationSize;
	relations = module_relations;
	kernel_policyID = module_policyID;
	printk("Registered pointer to functions\n");
	if (localAppCount > 0) ptr2initializeMonitor(localUID, localAppCount);
}

void LogicDroid_unregisterMonitor()
{
	noModule = 1;
	LogicDroid_CallRelationID = -1;
	LogicDroid_INTERNET_UID = -1;
	relations = NULL;
	relationSize = 0;
}

void LogicDroid_setIDs(int CallRelationID, int InternetUID)
{
  LogicDroid_CallRelationID = CallRelationID;
  LogicDroid_INTERNET_UID = InternetUID;
}

int LogicDroid_renewMonitorVariable(int *UID, int varCount, char value, int rel) {
  return ptr2renewMonitorVariable(UID, varCount, value, rel);
} 

int LogicDroid_initializeMonitor(int __user *UID, int appCount) {
  // copy the UID's
  kfree(localUID);
  localAppCount = appCount;
  localUID = (int*) kmalloc(sizeof(int) * localAppCount, GFP_KERNEL);
  copy_from_user(localUID, UID, sizeof(int) * localAppCount);
  if (noModule) return NO_MONITOR;
  return ptr2initializeMonitor(UID, appCount);
}

int LogicDroid_checkEvent(int rel, int *UID, int varCount, long timestamp) {
  if (noModule) return NO_MONITOR;
  return ptr2checkEvent(rel, UID, varCount, timestamp);
}

int LogicDroid_getRelationName(int ID, char __user *relationName)
{
  if (ID < 0 || ID >= relationSize) return OUT_OF_BOUND;
  if (noModule) return NO_MONITOR;
  strcpy(relationName, relations[ID]);
  return 1;
}
