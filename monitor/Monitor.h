#ifndef LOGICDROID_MONITOR_H_INCLUDED
#define LOGICDROID_MONITOR_H_INCLUDED

typedef enum 
{
  OUT_OF_BOUND = -3,
  NO_MONITOR = 2,
  POLICY_MISMATCH = -1,
  OK = 0,
  VIOLATION = 1
} LOGICDROID_RETURN_VALUE;

int LogicDroid_checkEvent(int rel, int *UID, int varCount, long timestamp);
int LogicDroid_getCallRelationID(void);
int LogicDroid_getInternetUID(void);
void LogicDroid_registerMonitor(int (*renewMonitorVariable)(int*, int, char, int), 
				int (*initializeMonitor) (int*, int),
				int (*checkEvent)(int, int*, int, long),
				char **module_relations, int module_relationSize, int module_policyID);
void LogicDroid_unregisterMonitor(void);
void LogicDroid_setIDs(int CallRelationID, int InternetUID);

EXPORT_SYMBOL(LogicDroid_registerMonitor);
EXPORT_SYMBOL(LogicDroid_unregisterMonitor);
EXPORT_SYMBOL(LogicDroid_setIDs);

#endif
