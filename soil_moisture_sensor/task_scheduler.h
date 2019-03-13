typedef void (*callbackFunction) ();

struct Task{
  unsigned long time;
  unsigned long duration;
  callbackFunction callback;
  boolean isDone;
  boolean isRepeating;
};

Task * scheduleTask(unsigned long inTime, callbackFunction func, boolean isRepeating);
Task * resetTask(Task *task);

void executeTasks();
