typedef void (*callbackFunction) ();

struct Task{
  unsigned long time;
  callbackFunction callback;
  boolean isDone;
};

Task * scheduleTask(unsigned long inTime);
Task * resetTask(Task *task);

void executeTasks();
