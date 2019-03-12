#define MAX_ELEMENTS 32

struct Task * scedueled_tasks[MAX_ELEMENTS];
byte numberOfElements = 0;

Task * scheduleTask(unsigned long inTime, callbackFunction callback){
  struct Task *newTask = (Task *)malloc(sizeof(Task));
  newTask->callback = callback;
  newTask->time = millis() + inTime;
  newTask->isDone = false;

  Serial.print("** Scheduled new task at ");
  Serial.println(newTask->time);
  
  scedueled_tasks[numberOfElements] = newTask;
  numberOfElements+=1;
  if(numberOfElements==MAX_ELEMENTS){
    numberOfElements = 0;
  }

  return newTask;
}

Task * resetTask(Task *task) {
  Serial.print("** Reseting task ");
  Serial.println(task->time);
  task->time = millis() + 5000;
  task->isDone = false;

  return task;
}

void executeTasks() {
   unsigned long currentTime = millis();

   for(int i=0; i< numberOfElements; i++){
    Task * currentTask = scedueled_tasks[i];
    if(!currentTask->isDone && currentTask->time <= millis()) {
      Serial.println("Yelding task");  
      currentTask->isDone = true;
      currentTask->callback();
    }
   }
}
