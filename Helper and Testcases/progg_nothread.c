#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>

#define TABLE_SIZE 10000003
#define N_MAX 50
#define K_MAX 500
#define T_MAX 100
#define ELV_MAX_CAP 6
#define MAX_NEW_REQ 30
#define MAX_RIDERS 1000

int num_of_reqfulfilled = 0;
int curr_pending_req = 0;

typedef struct SolverRequest{
    long mtype;
    int  elevatorNumber;
    char authStringGuess[21];
} SolverRequest;

typedef struct SolverResponse{
    long mtype;
    int guessIsCorrect;
} SolverResponse;

typedef struct PassengerRequest{
    int requestId;
    int startFloor;
    int requestedFloor;
} PassengerRequest;

typedef struct MainSharedMemory{
    char authString[100][21];
    char elevatorMovementInstructions[100];
    PassengerRequest newPassengerRequests[30];
    int elevatorFloors[100];
    int droppedPassengers[1000];
    int pickedUpPassengers[1000][2];
} MainSharedMemory;

typedef struct TurnChangeResponse{
    long mtype;
    int turnNumber;
    int newPassengerRequestCount;
    int errorOccured;
    int finished;
} TurnChangeResponse;

typedef struct TurnChangeRequest{
    long mtype;
    int droppedPassengersCount;
    int pickedUpPassengersCount;
} TurnChangeRequest;

typedef struct Rider {
    int requestId;
    int startFloor;
    int requestedFloor;
    int elv;
    int loc;
    int iswaiting;
} Rider;

typedef struct Elevator{
    int id;
    int current_floor;
    char direction;
    int load;
    Rider curr_riders[MAX_RIDERS];
    int num_riders_inq;
    int curr_dest;
    int path[K_MAX];
    int drop_path[K_MAX];
    int pick_path[K_MAX];
    Rider riders_inq[MAX_RIDERS];      
} Elevator;

typedef struct thread_args_t{
    key_t key;
    Elevator elv;
    int elv_no;
    int* isfree;
    int kid;
    MainSharedMemory* shmPtr;
} thread_args_t;


pthread_mutex_t shm_mutex;
sem_t solver_semaphore;

void initialize_elevators(Elevator elevators[], int num_elevators) {
    for (int i = 0; i < num_elevators; i++) {
        elevators[i].id = i ;
        elevators[i].current_floor = 0;
        elevators[i].direction = 's';
        elevators[i].load = 0;
        elevators[i].num_riders_inq = 0;
        elevators[i].curr_dest = -1;
        for(int j= 0; j<K_MAX ;j++){
            elevators[i].path[j] = 0;
        }
    }
}

int receiveSolver(int key,int t){
    int msgqid;
    if ((msgqid = msgget(key, 0644)) == -1){
        perror("Error in msgget()");
        exit(1);
    }
    struct SolverResponse buf;
    if((msgrcv(msgqid, &buf, sizeof(buf) - sizeof(buf.mtype), t, 0)) == -1){
        perror("Error in msgrcv()");
        exit(1);
    }
    int response = buf.guessIsCorrect;
    return response;
}

TurnChangeResponse receiveMain(int key,int t){
    int msgqid;
    if ((msgqid = msgget(key, 0644)) == -1){
        perror("Error in msgget()");
        exit(1);
    }
    struct TurnChangeResponse buf;
    if((msgrcv(msgqid, &buf, sizeof(buf) - sizeof(buf.mtype), t, 0)) == -1){
        perror("Error in msgrcv()");
        exit(1);
    }
    return buf;
}

void sendSolver(char* str,int elv_no, int key,int t){
    int msgqid;
    if((msgqid = msgget(key, 0644)) == -1){
        perror("Error in msgget()");
        exit(1);
    }

    struct SolverRequest buf;
    buf.mtype = t;
    buf.elevatorNumber = elv_no;

    strncpy(buf.authStringGuess, str, 21);
    if(msgsnd(msgqid, &buf, sizeof(buf) - sizeof(buf.mtype), 0) == -1){
        perror("Error in msgsnd()");
        exit(1);
    }
}

void sendMain(int drpcount,int pickcount, int key,int t){
    int msgqid;
    if((msgqid = msgget(key, 0644)) == -1){
        perror("Error in msgget()");
        exit(1);
    }
    struct TurnChangeRequest buf;
    buf.mtype = t;
    buf.droppedPassengersCount = drpcount;
    buf.pickedUpPassengersCount = pickcount;
    if(msgsnd(msgqid, &buf, sizeof(buf) - sizeof(buf.mtype), 0) == -1){
        perror("Error in msgsnd()");
        exit(1);
    }
}

void gen_string(int N,char *str){
    int i;
    for(i=N-1 ; i>=0 && str[i]=='f';i--){
        str[i] = 'a';
    }
    if(i<0) {
        return;
    }
    str[i]++;
}


void* thread_func(void* args) {
    printf("Thread func working\n");
    thread_args_t* actual_args = (thread_args_t*)args;
   
     
         
            printf("before gen_string\n");
            printf("%d\n",actual_args->elv.load);
           
            char str[actual_args->elv.load + 1];
            str[actual_args->elv.load] = '\0';
           
            for(int i=0 ;i<actual_args->elv.load ; i++){
                str[i] = 'a';
            }
           
            sendSolver(str,actual_args->elv_no,actual_args->key,2);
            sendSolver(str,actual_args->elv_no,actual_args->key,3);
           
            printf("%s\n",str);
            int done = receiveSolver(actual_args->key,4);
            printf("%d",done);
            printf("send to solver\n");
            while(!done){
                gen_string(actual_args->elv.load,str);
                sendSolver(str,actual_args->elv_no,actual_args->key,3);
                done = receiveSolver(actual_args->key,4);
            }
           
            sem_post(&solver_semaphore);
            printf("%s\n",str);
           
            pthread_mutex_lock(&shm_mutex);
                MainSharedMemory* ptr = actual_args->shmPtr;
                strncpy(ptr->authString[actual_args->elv_no], str, 21);
                //actual_args->isfree[actual_args->kid] = 1;
            pthread_mutex_unlock(&shm_mutex);
           
            printf("Thread Func done for %d \n",actual_args->elv.id);
     
       
    actual_args->isfree[actual_args->kid] = 1;
    pthread_exit(NULL);
}

void SetAuthString(int keys[],int N,int M,Elevator elevators[],MainSharedMemory* shmPtr,int isfree[]){

    int x = 0;
    for (int i = 0; i < N; i++) {
       
        if(elevators[i].load == 0){
            continue;
        }
       
        char str[elevators[i].load + 1];
        str[elevators[i].load] = '\0';
       
        for(int j=0 ;j<elevators[i].load; j++){
                str[j] = 'a';
        }
       
        sendSolver(str,i,keys[x%M],2);
        sendSolver(str,i,keys[x%M],3);
           
            printf("%s\n",str);
            int done = receiveSolver(keys[x%M],4);
            printf("%d",done);
            printf("send to solver\n");
            while(!done){
                gen_string(elevators[i].load,str);
                sendSolver(str,i,keys[x%M],3);
                done = receiveSolver(keys[x%M],4);
            }
           
            printf("%s\n",str);
           
                MainSharedMemory* ptr = shmPtr;
                strncpy(ptr->authString[i], str, 21);
            x++;
           
            printf("done for %d\n",i);
    }

}

void delete_rider(Rider arr[],int j,int size){
    for(int i=j ;i<size ; i++){
        arr[i] = arr[i+1];
    }
}

void delete_req(PassengerRequest arr[],int j,int size){
    for(int i=j ;i<size ; i++){
        arr[i] = arr[i+1];
    }
}

int rider_drop(int N,MainSharedMemory* mainshmptr , Elevator elevators[]){
    int t_cnt = 0;

    for(int i=0 ;i<N ;i++){
       // printf("Elevator %d has %d load \n",i,elevators[i].load);
        for(int j=0 ; j<elevators[i].load ; j++){
            printf("current riders of Elevator %d requested floor %d\n",i,elevators[i].curr_riders[j].requestedFloor);
            if(elevators[i].current_floor == elevators[i].curr_riders[j].requestedFloor){
               
                printf("in if\n");
                mainshmptr->droppedPassengers[t_cnt] = elevators[i].curr_riders[j].requestId;
                t_cnt++;
               
                elevators[i].curr_riders[j].elv = -1;
                elevators[i].curr_riders[j].loc = elevators[i].current_floor;
                elevators[i].path[elevators[i].curr_riders[j].requestedFloor]--;
                elevators[i].drop_path[elevators[i].curr_riders[j].requestedFloor]--;
               
                elevators[i].load--;
               
                printf("before delete\n");
                delete_rider(elevators[i].curr_riders,j,elevators[i].load);
                printf("after delete\n");
               
                num_of_reqfulfilled++;
            }
        }
    }

    return t_cnt;
}


int rider_pick(int N,MainSharedMemory* mainshmptr , Elevator elevators[]){
    //printf("int pick\n");
    int t_cnt = 0;

    for(int i=0 ; i<N;i++){
        for(int j=0 ;j< elevators[i].num_riders_inq;j++){
            //printf("%d\n",elevators[i].riders_inq[j].startFloor);
            //printf("%d\n",elevators[i].current_floor);
            if(elevators[i].riders_inq[j].startFloor == elevators[i].current_floor ){
               
                printf("before shm writing pick");

                mainshmptr->pickedUpPassengers[t_cnt][0] = elevators[i].riders_inq[j].requestId;
                mainshmptr->pickedUpPassengers[t_cnt][1] = i;
                t_cnt++;
               
                printf("before shm writing pick");
               
                elevators[i].path[elevators[i].riders_inq[j].startFloor]--;
                elevators[i].pick_path[elevators[i].riders_inq[j].startFloor]--;
                elevators[i].path[elevators[i].riders_inq[j].requestedFloor]++;
                elevators[i].drop_path[elevators[i].riders_inq[j].requestedFloor]++;
                elevators[i].num_riders_inq--;
                elevators[i].load++;
                elevators[i].curr_riders[elevators[i].load-1] = elevators[i].riders_inq[j];
               
                printf("before delete\n");
                delete_rider(elevators[i].riders_inq,j,elevators[i].num_riders_inq);
                printf("after delete\n");
            }
        }
    }
    printf("done pick\n");
    return t_cnt;
}

int cost(Rider R, Elevator elevator,int M){
    int cnt;

    cnt = abs(R.startFloor - elevator.current_floor);

    if((R.startFloor < elevator.current_floor && elevator.direction=='u') || (R.startFloor > elevator.current_floor && elevator.direction=='d') ){
        cnt += K_MAX ;
    }
   
    cnt += elevator.load*M;
   
    if((elevator.load+elevator.num_riders_inq) > 4){
        cnt += K_MAX*(elevator.load+elevator.num_riders_inq);
    }
   
    for(int i=0 ; i<K_MAX ; i++){
        if(elevator.drop_path[i]>0){
            cnt -= K_MAX;
        }
    }

    if (elevator.load +elevator.num_riders_inq== ELV_MAX_CAP){
        cnt = INT_MAX;
    }

    return cnt;
   
}

int assign_elevator(Rider R , Elevator elevators[], int N,int M){

    int min_cost_elv = N+1;
    int min_cost = INT_MAX;

    for(int i=0 ;i<N ; i++){
        int x = cost(R,elevators[i],M);
        if(x < min_cost){
            min_cost = x;
            min_cost_elv = i;
        }
    }
   
    if(min_cost == INT_MAX){
        return 1;
    }
    R.elv = min_cost_elv;
    elevators[min_cost_elv].path[R.startFloor]++;
    elevators[min_cost_elv].pick_path[R.startFloor]++;
    elevators[min_cost_elv].num_riders_inq++;
    elevators[min_cost_elv].riders_inq[elevators[min_cost_elv].num_riders_inq - 1] = R;
    return 0;
}

void New_request_helper(PassengerRequest new_req[] ,Elevator elevators[] ,int N,int M){

    for(int i=0 ; i<curr_pending_req;i++){

        Rider new_rider ;
        new_rider.requestId = new_req[i].requestId;
        new_rider.startFloor = new_req[i].startFloor;
        new_rider.requestedFloor = new_req[i].requestedFloor;
        new_rider.iswaiting = 1;
        new_rider.elv = -1;
        new_rider.loc = new_req[i].startFloor;

        int state = assign_elevator(new_rider,elevators,N,M);
       
        if(state){
            continue;
        }else{
            delete_req(new_req,i,curr_pending_req);
            curr_pending_req--;
        }
       
    }
}

void Next_Inst(Elevator elevators[] , int N,int K){
    for(int i=0 ;i<N ;i++){

        char curr_direc = elevators[i].direction;
        int curr_floor = elevators[i].current_floor;
        int flag = 1;
        int min_floor = -1;
        int max_floor = K+1;

        for(int j = 0 ; j<K ;j++){
            if(flag && elevators[i].drop_path[j]>0){
                min_floor = j;
                flag = 0;
            }
            if(elevators[i].drop_path[j]>0){
                max_floor = j;
            }
        }

        if(min_floor == -1 && max_floor == K+1){
            for(int j = 0 ; j<K ;j++){
            if(flag && elevators[i].pick_path[j]>0){
                min_floor = j;
                flag = 0;
            }
            if(elevators[i].pick_path[j]>0){
                max_floor = j;
            }
        }
        }
        if(curr_direc == 'u'){
            if(curr_floor < max_floor && max_floor != K+1){
                elevators[i].direction = 'u';
                elevators[i].curr_dest = max_floor;
               
            }else{
                if(max_floor!=K+1){
                    elevators[i].direction = 'd';
                    elevators[i].curr_dest = min_floor;
                }else{
                    elevators[i].direction = 's';
                }
            }
        }

        if(curr_direc == 'd'){
            if(curr_floor > min_floor && min_floor != -1){
                elevators[i].direction = 'd';
                elevators[i].curr_dest = min_floor;
            }else{
                if(min_floor!= -1){
                    elevators[i].direction = 'u';
                    elevators[i].curr_dest = max_floor;
                }else{
                    elevators[i].direction = 's';
                }
            }
        }

        if(curr_direc == 's'){
            if(max_floor != K+1 && min_floor!=-1){
            if(curr_floor > min_floor && curr_floor < max_floor){
                if(curr_floor-min_floor > max_floor - curr_floor ){
                    elevators[i].direction = 'u';
                    elevators[i].curr_dest = max_floor;
                }else{
                    elevators[i].direction = 'd';
                    elevators[i].curr_dest = min_floor;
                }
            }
            if(curr_floor > min_floor && curr_floor > max_floor){
                elevators[i].direction = 'd';
                elevators[i].curr_dest = min_floor;
            }
            if(curr_floor < min_floor && curr_floor < max_floor){
                elevators[i].direction = 'u';
                elevators[i].curr_dest = max_floor;
            }
            }else{
                elevators[i].direction = 's';
            }
        }

    }
}

int main(){

    FILE *inputfptr = fopen("input.txt", "r");
    if(inputfptr==NULL){
        perror("Error on accessing input file");
        exit(1);
    }
   
    int N,K,M,T;
    key_t shm_key;
    key_t main_msgq_key;

    fscanf(inputfptr, "%d", &N);
    fscanf(inputfptr, "%d", &K);
    fscanf(inputfptr, "%d", &M);
    fscanf(inputfptr, "%d", &T);
    fscanf(inputfptr, "%d", &shm_key);
    fscanf(inputfptr, "%d", &main_msgq_key);

    key_t solv_msgq_key[M];

    for(int i=0 ;i<M ;i++){
        fscanf(inputfptr, "%d", &solv_msgq_key[i]);
    }

    fclose(inputfptr);
   
    printf("%d %d %d %d %d %d ",N,K,M,T,shm_key,main_msgq_key);
    for(int i=0;i<M;i++) {
        printf("%d ",solv_msgq_key[i]);
    }
    printf("\n");
   
   
   
    MainSharedMemory* mainshmPtr;
    int shmId = shmget(shm_key,sizeof(MainSharedMemory),0666);
    if(shmId == -1){
        perror("Error on shmget()");
        exit(1);
    }
    mainshmPtr = (MainSharedMemory*)shmat(shmId,NULL,0);

    pthread_mutex_init(&shm_mutex, NULL);

    Elevator elevators[N];

    initialize_elevators(elevators,N);
    int Total_req = 0;
    char inst[100];
    for (int i = 0; i < 100; i++){
        inst[i] = 's';
    }
    int droppedPassengers[1000];
    int pickedUpPassengers[1000][2];
    int isFinished = 0;
   
    PassengerRequest request_pool[1000];
   

    TurnChangeResponse currState = receiveMain(main_msgq_key,2);
    isFinished = currState.finished;

    int no_new_req = currState.newPassengerRequestCount;
    Total_req += no_new_req;
    PassengerRequest* new_req = mainshmPtr->newPassengerRequests;
   
    for(int i=0 ;i<no_new_req;i++){
        request_pool[curr_pending_req++] = new_req[i];
    }
   
    int x = 0;
    while(!isFinished){
       
        int isfree[M];
        for(int i=0 ;i<M ; i++){
            isfree[i] = 1;
        }
        printf("set auth start \n");
       
        SetAuthString(solv_msgq_key,N,M,elevators,mainshmPtr,isfree);
       
        printf("Set auth done\n");

        int drpcnt=0;
        int pickcnt=0;

        drpcnt = rider_drop(N,mainshmPtr,elevators);
       
        printf("drp done\n");
       
        if(T--){
            New_request_helper(request_pool,elevators,N,M);
        }
       
        printf("new req done\n");

        pickcnt = rider_pick(N,mainshmPtr,elevators);
       
        printf("rider pick done\n");

        Next_Inst(elevators,N,K);

        for(int i=0;i<N;i++) {
            inst[i] = elevators[i].direction;
        }
       
        strncpy(mainshmPtr->elevatorMovementInstructions,inst,100);
        sendMain(drpcnt,pickcnt,main_msgq_key,1);

        currState = receiveMain(main_msgq_key,2);

        no_new_req = currState.newPassengerRequestCount;
        PassengerRequest* new_req = mainshmPtr->newPassengerRequests;
        Total_req += no_new_req;
        for(int i=0 ;i<no_new_req;i++){
            request_pool[curr_pending_req++] = new_req[i];
        }
        isFinished = currState.finished;
       
        for(int i=0;i<N;i++) {
            elevators[i].current_floor = mainshmPtr->elevatorFloors[i];
            printf("Elevator %d is on floor %d with current load+riders in queue with load %d %d\n",i,elevators[i].current_floor,elevators[i].load + elevators[i].num_riders_inq,elevators[i].load);
        }
        printf("number of total request fulfilled %d and total request are %d",num_of_reqfulfilled,Total_req);
        printf("----Turn%d finished-----\n",x);
        x++;
    }
   
   

    pthread_mutex_destroy(&shm_mutex);
    return 0;
}