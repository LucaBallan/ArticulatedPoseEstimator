//
// GammaLib: Computer Vision library
//
//    Copyright (C) 1998-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
//
//    Third party copyrights are property of their respective owners.
//
//
//    The MIT License(MIT)
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
//
//
//





#include "lib.h"
using namespace std;


//
//
// TaskManager
//
//
TaskManager::TaskManager(TASK_FUNCTION default_F,void *global_data) {
	this->default_F=default_F;
	this->global_data=global_data;
	TaskList=new Array<Task>(20);
	running_task=false;


	CriticalSection=CreateMutex(NULL,FALSE,NULL);
	NonEmptyBuffer=CreateEvent(NULL,TRUE,FALSE,NULL);
	ExitTaskManager=CreateEvent(NULL,TRUE,FALSE,NULL);
	hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TaskManagerThread,this,0,&dwThreadId);
}

TaskManager::~TaskManager() {
	// Terminate Thread
	SetEvent(ExitTaskManager);
	cout<<"TaskManager: Flushing all tasks...\n";
	WAIT_EVENT(hThread);
	cout<<"TaskManager: Flushing completed.\n";

	// Destroy all
	delete TaskList;
	CloseHandle(hThread);
	CloseHandle(NonEmptyBuffer);
	CloseHandle(CriticalSection);
}

void TaskManager::Add(HANDLE *Done,void *data,TASK_FUNCTION F) {
	if (EVENT_SIGNALED(ExitTaskManager)) {Warning("TaskManager: The manager is closing, cannot add a new task.");return;}
	if (!F) F=default_F;
	if (!F) ErrorExit("TaskManager: the task doesn't have a valid function to perform.");

	HANDLE done=NULL;
	if (Done) {					// se  Done == NULL non lo usa
		done=*Done;				// se *Done != NULL assicura che sia signaled e lo resetta
		if (done) {				// se *Done == NULL crea un Handle, aggiorna (*Done)
			if (!EVENT_SIGNALED(done)) ErrorExit("TaskManager: Some tasks are still performing on this event.");
			ResetEvent(done);
		} else (*Done)=done=CreateEvent(NULL,TRUE,FALSE,NULL);
	} 

	Task x;
	x.F=F;
	x.data=data;
	x.done=done;				// done e' un HANDLE!=NULL iff Done!=NULL

	ENTER_C_(CriticalSection);
		TaskList->append(x);
		SetEvent(NonEmptyBuffer);		// signed
	EXIT_C_(CriticalSection);
}

int TaskManager::PendingTasks() {
	int pending_tasks;

	ENTER_C_(CriticalSection);
		pending_tasks=TaskList->numElements();
		if (running_task) pending_tasks++;
	EXIT_C_(CriticalSection);
	
	return pending_tasks;
}

DWORD TaskManager::TaskManagerThread(TaskManager *src) {
	int state;
	TASK_FUNCTION F;
	void *data;
	HANDLE done;

wait_next:	
	WAIT_2EVENT(src->NonEmptyBuffer,src->ExitTaskManager);
	TEST_EVENT(src->NonEmptyBuffer,state);					  // Anche se ha ricevuto il segnale di spegnimento
	if (state!=1) goto exit_from_loop;						  // esegui tutti i processi in coda finche' non sono finiti, 
process_next:												  // dopodiche' termina.
	ENTER_C_(src->CriticalSection);
		F=(*(src->TaskList))[0].F;
		data=(*(src->TaskList))[0].data;
		done=(*(src->TaskList))[0].done;
		src->TaskList->del(0);
		src->running_task=true;
	EXIT_C_(src->CriticalSection);
	
	F(data,src->global_data);
	if (done) SetEvent(done);
	
	ENTER_C_(src->CriticalSection);
		src->running_task=false;
		if (data) delete []data;
		if (src->TaskList->numElements()!=0) {
			EXIT_C_(src->CriticalSection);
			goto process_next;
		}
		ResetEvent(src->NonEmptyBuffer);		// not signed
	EXIT_C_(src->CriticalSection);
	goto wait_next;

exit_from_loop:
	cout<<"TaskManagerTask: Terminated.\n";
	return 0;
}


















//
//
// MultiProcessing
//
//
MultiProcessing::MultiProcessing(int n_processors,SPLIT_FUNCTION Split,TASK_FUNCTION default_F,void *global_data) {
	this->n_processors=n_processors;
	this->Split=Split;

	Processor=new TaskManager*[n_processors+1];
	Processor[0]=new TaskManager((TASK_FUNCTION)ManagerFunction,this);
	for(int i=1;i<=n_processors;i++) Processor[i]=new TaskManager(default_F,global_data);
}

MultiProcessing::~MultiProcessing() {
	// Flush all the processes
	for(int i=1;i<=n_processors;i++) delete Processor[i];
	// They will send the signal to the manager
	// Now I can quit it
	delete Processor[0];
	delete []Processor;
}

void MultiProcessing::ManagerFunction(HANDLE *data,MultiProcessing *src) {
	WaitForMultipleObjects(src->n_processors,data,TRUE,INFINITE);
	for(int i=0;i<(src->n_processors);i++) CloseHandle(data[i]);
}

void MultiProcessing::Add(HANDLE *Done,void *data,TASK_FUNCTION F) {
	HANDLE *DoneA=NULL;
	void **dataA=NULL;

	// Create the handles -> DoneA
	SNEWA_P(DoneA,HANDLE,n_processors);

	// Split the data -> dataA
	SNEWA(dataA,void*,n_processors);
	Split(data,n_processors,dataA);
	SDELETEA(data);

	for(int i=1;i<=n_processors;i++) 
		Processor[i]->Add(&(DoneA[i-1]),dataA[i-1],F);		// Processor[i] will delete dataA[i-1]
															//				will create DoneA[i-1]
	SDELETEA(dataA);

	Processor[0]->Add(Done,DoneA,NULL);						// Done will be managed by TaskManager and the caller either (allocation and delete) or (nothing and (allocation/delete))
															// DoneA[i] will be closed by ManagerFunction
															// DoneA    will be deleted by TaskManager
}

void MultiProcessing::Add(int n,HANDLE *Done,void *data,TASK_FUNCTION F) {
	if ((n<0) || (n>=n_processors)) ErrorExit("MultiProcessing: Processor doesn't exist");
	Processor[n+1]->Add(Done,data,F);
}
			
void MultiProcessing::Interval_Split(void *data,int n_elements_to_split,void **splitted_data) {
	int first,last;
	bscanf((BYTE*)data,"ii",&first,&last);

	double n_ele=(last-first+1)*1.0/n_elements_to_split;

	for(int i=0;i<n_elements_to_split;i++) {
		int f,l;

		f=Approx(first+(n_ele*i));
		
		if (i==n_elements_to_split-1) l=last;
		else l=Approx(first+(n_ele*(i+1)))-1;

		splitted_data[i]=bsprintf("ii",f,l);
	}
}




















//
//
// RepetitiveTaskManager
//
//
RepetitiveTaskManager::RepetitiveTaskManager(void *global_data) {
	this->global_data=global_data;
	this->next_valid_task_id=0;

	TaskList=new Array<RepetitiveTask>(20);
	ClassInfo=new Array<RepetitiveTaskClassInfo>(20);
	CriticalSection=CreateMutex(NULL,FALSE,NULL);
}

RepetitiveTaskManager::~RepetitiveTaskManager() {
	ENTER_C_(CriticalSection);
	int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if ((*TaskList)[i].data) delete []((*TaskList)[i].data);
		}
	EXIT_C_(CriticalSection);

	CloseHandle(CriticalSection);
	delete ClassInfo;
	delete TaskList;
}

void RepetitiveTaskManager::DeclareTaskClass(int class_id,int priority,bool exclusive_execution) {
	if (priority<0) ErrorExit("Task class priority must be greater than or equal to zero.");

	RepetitiveTaskClassInfo x;
	x.class_id=class_id;
	x.priority=priority;
	x.exclusive_execution=exclusive_execution;

	ENTER_C_(CriticalSection);
	if (ClassInfo->search(x)!=-1) {
		EXIT_C_(CriticalSection);
		ErrorExit("Task class already exists.");
	}

	ClassInfo->append(x);
	EXIT_C_(CriticalSection);
}

int RepetitiveTaskManager::Add(void *data,REPETITIVE_TASK_FUNCTION F,int task_class) {

	RepetitiveTask x;
	RepetitiveTaskClassInfo cx;
	x.data=data;
	x.F=F;
	x.task_class=task_class;
	cx.class_id=task_class;

	ENTER_C_(CriticalSection);
		int class_array_index=ClassInfo->search(cx);
		if (class_array_index==-1) {
			EXIT_C_(CriticalSection);
			ErrorExit("Task class does not exists.");
		}
		x.task_class_array_index=class_array_index;
		x.task_priority=(*ClassInfo)[class_array_index].priority;
		x.task_id=next_valid_task_id++;
		
		int i=0,num_tasks=TaskList->numElements();
		if (x.task_priority!=TASK_MIN_PRIORITY) {
			// insert mantaining the priority order and for the task with the same priority we adopt the FIFO order 
			// -> this ensure that tasks of the same class (i.e. of the same priority) are exectuted in FIFO order
			for(;i<num_tasks;i++) {
				if ((*TaskList)[i].task_priority<x.task_priority) {
					TaskList->insertBefore(i,x);
					break;
				}
			}
			if (i==num_tasks) TaskList->append(x);
		} else TaskList->append(x);	  // ensure FIFO order
	EXIT_C_(CriticalSection);

	return x.task_id;
}

void RepetitiveTaskManager::Delete(int task_id) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if ((*TaskList)[i].task_id==task_id) {
				if ((*TaskList)[i].data) delete []((*TaskList)[i].data);
				TaskList->del(i);
				break;
			}
		}
	EXIT_C_(CriticalSection);
}

void RepetitiveTaskManager::DeleteClass(int task_class) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if ((*TaskList)[i].task_class==task_class) {
				if ((*TaskList)[i].data) delete []((*TaskList)[i].data);
				TaskList->del(i);
				num_tasks--;
				i--;
			}
		}
	EXIT_C_(CriticalSection);
}

void RepetitiveTaskManager::DeleteClassRange(int start_class_range,int end_class_range) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if (((*TaskList)[i].task_class>=start_class_range) && ((*TaskList)[i].task_class<=end_class_range)) {
				if ((*TaskList)[i].data) delete []((*TaskList)[i].data);
				TaskList->del(i);
				num_tasks--;
				i--;
			}
		}
	EXIT_C_(CriticalSection);
}

bool RepetitiveTaskManager::IsExecutingClass(int task_class) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if ((*TaskList)[i].task_class==task_class) {
				EXIT_C_(CriticalSection);
				return true;
			}
		}
	EXIT_C_(CriticalSection);
	return false;
}

bool RepetitiveTaskManager::IsExecutingClassRange(int start_class_range,int end_class_range) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if (((*TaskList)[i].task_class>=start_class_range) && ((*TaskList)[i].task_class<=end_class_range)) {
				EXIT_C_(CriticalSection);
				return true;
			}
		}
	EXIT_C_(CriticalSection);
	return false;
}

bool RepetitiveTaskManager::IsStillAlive(int task_id) {
	ENTER_C_(CriticalSection);
		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			if ((*TaskList)[i].task_id==task_id) {
				EXIT_C_(CriticalSection);
				return true;
			}
		}
	EXIT_C_(CriticalSection);
	return false;
}

void RepetitiveTaskManager::print() {
	ENTER_C_(CriticalSection);
	cout<<"Task List:\n";
	int num_tasks=TaskList->numElements();
	for(int i=0;i<num_tasks;i++) {
		cout<<"   "<<(*TaskList)[i]<<"\n";;
	}
	EXIT_C_(CriticalSection);
}

int RepetitiveTaskManager::Execute() {
	int executed_tasks=0;

	ENTER_C_(CriticalSection);
		int num_class=ClassInfo->numElements();
		for(int i=0;i<num_class;i++) (*ClassInfo)[i].avoid_execution=false;

		int num_tasks=TaskList->numElements();
		for(int i=0;i<num_tasks;i++) {
			RepetitiveTask *task_ref=&((*TaskList)[i]);
			RepetitiveTaskClassInfo *class_ref=&((*ClassInfo)[task_ref->task_class_array_index]);

			// Ordinati in ordine di esecuzione (il primo ha la priorita' massima)
			if (!class_ref->avoid_execution) {
				if (task_ref->F(task_ref->data,global_data)==false) {
					if (task_ref->data) delete [](task_ref->data);
					TaskList->del(i);
					num_tasks--;
					i--;
					// TODO: si puo' inserire l'opzione: se lo cancello, vado ad eseguire quello dopo?
				}
				if (class_ref->exclusive_execution) class_ref->avoid_execution=true;
				executed_tasks++;
			}
		}
	EXIT_C_(CriticalSection);
	return executed_tasks;
}

