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





typedef void(*TASK_FUNCTION)(void *data, void *global_data);
typedef void (*SPLIT_FUNCTION)(void *data,int n_elements_to_split,void **splitted_data);
typedef bool (*REPETITIVE_TASK_FUNCTION)(void *data,void *global_data);						// true		-> The task will stay alive (will repeat next time)
																							// false	-> The task will die		(will not repeat next time)
																							// NOTE: A repetitive task is NOT ALLOWED to Add or Delete Task inside his task!!
					


//
// TaskManager:
//		- Crea un oggetto che performa sequenzialmente (First In First Processed) una sequenza di task
//		- Il processamento di questi task avverra' in parallelo al task chiamante
//		- I task non saranno parallelizzati tra loro, ma eseguiti sequenzialmente
//
//		NB: Classe testata approfonditamente in MultiTheading, Sync Ok, Leakage Ok
//

class TaskManager {
	// Tasks Data
	Array<Task> *TaskList;
	TASK_FUNCTION default_F;
	void *global_data;
	bool running_task;					// informa se sto'eseguendo o meno un task (solo informativo)

	// Thread
	HANDLE hThread;
	DWORD dwThreadId;
	HANDLE CriticalSection,NonEmptyBuffer,ExitTaskManager;
	static DWORD WINAPI TaskManagerThread(TaskManager *src);

public:
	TaskManager(TASK_FUNCTION default_F=NULL,void *global_data=NULL);
	~TaskManager();
			// Call the ~TaskManager to flush out all the tasks
			// deve essere chiamato prima di distruggere gli eventi Done


	void Add(HANDLE *Done,void *data,TASK_FUNCTION F=NULL);
			// data        = array di bytes o di oggetti semplici
			//		         creata dal chiamante
			//				 modificabile dal Task (usabile come spazio temporaneo)
			//		         cancellata da TaskManager con delete []
			//		         puo' essere NULL
		    // global_data = puntatore a qualsiasi cosa
			//				 gestita dal chiamante (TaskManager non la cancellera')
			//		         puo' essere NULL
			// Done        = NULL   niente
			//            != NULL   Puntatore ad un oggetto HANDLE
			//						HANDLE signaled     -> il task e' terminato
			//						HANDLE non-signaled -> il task e' in esecuzione o in coda
			//			
			//							se HANDLE == NULL, TaskManager crea l'evento 
			//							se HANDLE != NULL, TaskManager usera' questo evento e aggiorna il valore puntato
			//											   In tal caso HANDLE deve essere signaled alla chiamata di Add, verra' resettato a non-signaled all'interno di Add.
			//						In ogni caso, il chiamante deve occuparsi della sua distruzione con CloseHandle
			//						ATTENZIONE: distruggere un evento Done solo se si e' sicuri che nessun task lo sta' ancora usando
			//									sicuramente dopo ~TaskManager gli eventi done possono essere cancellati
			//

	int PendingTasks();
			// Ritorna il numero di task in esecuzione o in coda per l'esecuzione
};






//
// MultiProcessing:
//		- Crea un oggetto che performa sequenzialmente (First In First Processed) una sequenza di tasks
//		  suddividendo ogni task in n_processors sottotask che verranno eseguiti in parallelo.
//		- Il processamento di questi tasks avverra' in parallelo al task chiamante
//		- Tra loro i tasks non saranno parallelizzati, ma eseguiti sequenzialmente
//		- I sotto task costituenti uno specifico task saranno eseguiti parallelamente (solo) tra loro
//
//		NB: Classe testata in MultiTheading, Sync Ok	(test non approfonditi dal punto di vista del memory leakage ma dovrebbe essere ok)
//

class MultiProcessing {
	int n_processors;
	TaskManager **Processor;

	SPLIT_FUNCTION Split;
	static void ManagerFunction(HANDLE *data,MultiProcessing *src);

public:
	MultiProcessing(int n_processors,SPLIT_FUNCTION Split,TASK_FUNCTION default_F=NULL,void *global_data=NULL);
	~MultiProcessing();

	void Add(HANDLE *Done,void *data,TASK_FUNCTION F=NULL);
			// Work as in TaskManager
			// data        = array di bytes o di oggetti semplici
			//		         creata dal chiamante
			//				 modificabile dal Task (usabile come spazio temporaneo)
			//		         cancellata da TaskManager con delete []
			//		         puo' essere NULL
		    // global_data = puntatore a qualsiasi cosa
			//				 gestita dal chiamante (TaskManager non la cancellera')
			//		         puo' essere NULL
			// Done        = NULL   niente
			//            != NULL   Puntatore ad un oggetto HANDLE
			//						HANDLE signaled     -> il task e' terminato
			//						HANDLE non-signaled -> il task e' in esecuzione o in coda
			//			
			//							se HANDLE == NULL, TaskManager crea l'evento 
			//							se HANDLE != NULL, TaskManager usera' questo evento e aggiorna il valore puntato
			//											   In tal caso HANDLE deve essere signaled alla chiamata di Add, verra' resettato a non-signaled all'interno di Add.
			//						In ogni caso, il chiamante deve occuparsi della sua distruzione con CloseHandle
			//						ATTENZIONE: distruggere un evento Done solo se si e' sicuri che nessun task lo sta' ancora usando
			//									sicuramente dopo ~TaskManager gli eventi done possono essere cancellati
			//
	
	

	void Add(int n,HANDLE *Done,void *data,TASK_FUNCTION F=NULL);
			// Work as before but it will perform the operation only on the processor n = 0,...,n_processors-1

	
	// Standard split function
	static void Interval_Split(void *,int,void **);
			// Get an interval to work with [f,l]   -> data=bsprintf("ii",f,l);
			// split it equally in n processors
			// DETERMINISTIC assignation (with the same input we get the same association to the same thread) (Necessary for gpu operations)
};

//
// void Split(void *data,int n_elements_to_split,void **splitted_data)
//				Splitta data in n_elements_to_split 
//					data              = [const] vettore in BYTE contenente le informazioni (non cancellare)
//										puo' essere NULL
//					splitted_data     = vettore di vettori BYTE contenente le informazioni splittate 
//										splitted_data e' creato da MultiProcessing e cancellato da esso
//					splitted_data[i]  = vettore in BYTE contenente le informazioni splittate
//										deve essere allocato e riempito da Split
//										cancellato da MultiProcessing con delete []
//										puo' essere NULL
// 

//
// Tipico Utilizzo:
//
//	MultiProcessing X(6,MultiProcessing::Interval_Split,NULL,NULL);
//
//	X.Add(NULL,SOME_DATA,INIT_FUNCTION);			
//	X.Add(Done1,data1,F1);
//	X.Add(NULL,data2,F2);
//	WAIT_EVENT(Done1);
//
//	X.Add(NULL,data3,F3);
//	X.Add(Done2,NULL,END_FUNCTION);
//	WAIT_EVENT(Done2);
//
//  CloseHandle(Done1);
//	CloseHandle(Done2);
//
//
// Test environment
//
//HANDLE CriticalSection;
//void F(void *data,void *global) {
//	int f,l;
//	bscanf((BYTE*)data,"ii",&f,&l);
//
//	ENTER_C;
//	cout<<"["<<f<<","<<l<<"]\n";
//	EXIT_C;
//}
//
//MultiProcessing *X=new MultiProcessing(6,MultiProcessing::Interval_Split,NULL,NULL);
//CriticalSection=CreateMutex(NULL,FALSE,NULL);
//HANDLE Done=NULL;
//X->Add(&Done,bsprintf("ii",1,100),F);
//WAIT_EVENT(Done);
//CloseHandle(Done);Done=NULL;
//X->Add(&Done,bsprintf("ii",1,100),F);
//delete X;
//CloseHandle(Done);Done=NULL;
//CloseHandle(CriticalSection);












//
// RepetitiveTaskManager (SyncTaskManager):
//		- Crea un oggetto che performa, durante ogni Execute(), una serie di Tasks (specificati in una sorta di to-do list)
//		- dopo l'esecuzione di un task, esso potra' scegliere se essere o meno eseguito alla prossima chiamata di Execute() (ovvero, se morire oppure rimanere in vita almeno per la prossima chiamata)
//		- i task non saranno parallelizzati tra loro, ma eseguiti sequenzialmente in un ordine cooerente con la priorita' impostata
//		  task a priorita' alta vengono eseguiti prima di quelli a priorita' bassa
//		- ogni task puo' operare nel suo spazio dati (*data) a suo piacimento (rimanendo nei limiti allocati)
//		  potra' quindi usare questo spazio per comunicare con se stesso in una sua futura esecuzione
//		- NOTE: i task NON SONO AUTORIZZATI A CREARE O CANCELLARE ALTRI TASK nella lista -> risultato imprevedibile o deadlock
//		- se si immagina che ad ogni Execute() ciascun tasks esegue solo una parte del proprio lavoro,
//		  questo oggetto si puo' immaginare come un TaskManager dove piu' tasks vengono eseguiti contemporaneamente
//		  e in modo sincrono (inoltre sempre rispettando le loro priorita').
//		- ogni task appartiene ad una classe a cui e' associata una priorita' di esecuzione
//		  la classe inoltre definisce se i task appartenenti ad essa inseriti nella to-do list possano
//		  o meno essere eseguiti durante la stessa chiamata Execute().
//		  In caso contrario, infatti, solo un task di quella classe potra' essere eseguito da Execute() mentre
//		  gli altri verranno eseguiti quando quest'ultimo muore.
//		  Questo ordine segue la politica FIFO, ovvero, il primo task di quella classe ad entrare e' il primo ad
//		  essere servito.
//		
//		NB: Classe testata approfonditamente in MultiTheading, Sync Ok and Leakage
//
#define TASK_MIN_PRIORITY	0

class RepetitiveTaskManager {
	// Tasks Data
	Array<RepetitiveTask> *TaskList;
	Array<RepetitiveTaskClassInfo> *ClassInfo;
	void *global_data;

	// sync data
	int next_valid_task_id;
	HANDLE CriticalSection;

public:
	RepetitiveTaskManager(void *global_data=NULL);
	~RepetitiveTaskManager();

	void DeclareTaskClass(int class_id,int priority=TASK_MIN_PRIORITY,bool exclusive_execution=true);

	int  Add(void *data,REPETITIVE_TASK_FUNCTION F,int task_class);
			// data				= array di bytes o di oggetti semplici
			//					  creata dal chiamante
			//					  modificabile dal Task (puo' comunicare qualcosa alla sua prossima esecuzione)
			//					  cancellata da RepetitiveTaskManager con delete []
			//					  puo' essere NULL
		    // global_data		= puntatore a qualsiasi cosa
			//					  gestita dal chiamante (RepetitiveTaskManager non la cancellera')
			//					  puo' essere NULL
			// task_class		= classe di appartenenza del task
			// priority			= priorita' di esecuzione del task    [0, +INF] 
			//					  i task a priorita' piu' alta verranno eseguiti prima di quelli a bassa priorita'
			//					  nulla si puo' asserire per quei task con egual priorita'
			//					  priority = TASK_MIN_PRIORITY = 0    -> priorita' minima, i task con questa priorita'
			//															 vengono eseguiti dopo tutti gli altri senza 
			//															 riguardo di ordine tra di loro
			// id				= id univoco del task
			//					  ritornato da Add
	
	void Delete(int task_id);												// Cancella lo specifico task
	void DeleteClass(int task_class);										// Cancella tutti i task appartenenti a questa classe
	void DeleteClassRange(int start_class_range,int end_class_range);		// Cancella tutti i task appartenenti a questo range di classi (estremi compresi)
	
	bool IsStillAlive(int task_id);
	bool IsExecutingClass(int task_class);
	bool IsExecutingClassRange(int start_class_range,int end_class_range);
	void print();

	int Execute();															// Ritorna il numero dei task eseguiti
};


typedef class RepetitiveTaskManager SyncTaskManager;
