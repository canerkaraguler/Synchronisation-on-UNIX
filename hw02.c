


// >>gcc -o hw02.exe hw02.c -lpthread
// >>./hw02.exe




/*

		The execution is like that:

		1) 4 type A thread, 1 type B thread and 1 type C thread is created and joined.
		2) All threads are working parallel and while a type A thread is in its critical section, other type A threads have to wait it.
		3) while type A thread is in its critical section, type B or type C thread can get in their critical section for only one of the un allocated resource type 1 files.
		4) while type B or type C threads can work on resource type 2 files if it is created and while they are working on that file, type A threads are waiting.
		5) Type B and type C can not get its critical section for the same resource file.
		6) If Type B or type C threads can not enter their critical section while resource type 1 files are not empty, they can only enter  their critical section with resource type 2 files.

		If you want to see which thread is working, u can comment out the printf lines in functions delete_prime_line_r2() , delete_negative_line_r2(), delete_prime_line_r1(int semaphoreValue), delete_negative_line_r1(int semaphoreValue), cut_paste(int semaphoreValue,char *message ).Because the output is too long u can only see the last part of the output in terminal so I commended these print lines to avoid confusion. but it works correctly.

*/



#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#define NO_OF_THREADS_TYPE_A 4



void function_for_typeA (void *ptr);
void function_for_typeB (void *ptr);
void function_for_typeC (void *ptr);
void cut_paste(int semaphoreValue,char *message );
int check_prime_number(int number);
void delete_prime_line_r2();
void delete_negative_line_r2();
void delete_prime_line_r1(int semaphoreValue);
void delete_negative_line_r1(int semaphoreValue);

sem_t mutexForBandC;
sem_t mutex;

sem_t resourceType1;

int resourceType2Count=0;

int counter[]={0,0};
char *txtNames[]={"numbers1.txt","numbers2.txt"};
char *tmpTxtNames[]={"tmpnumbers1.txt","tmpnumbers2.txt"};
int emptyTxt[]={0,0};

int main(int argc, char *argv[]){
	//defining threads
	pthread_t threadsTypeA[NO_OF_THREADS_TYPE_A];
	pthread_t threadTypeB;
	pthread_t threadTypeC;
	//creating messages for thread identification.
	char *messagesForThreadsTypeA[]={"ThreadA1","ThreadA2","ThreadA3","ThreadA4"};
	char *messageForThreadTypeB="ThreadB";
	char *messageForThreadTypeC="ThreadC";
	//time declaring for random number generation	
	time_t t;
	srand((unsigned) time(&t));
	//initializing semaphores
	sem_init(&mutexForBandC,0,1);
	sem_init(&mutex,0,1);
	
	sem_init(&resourceType1,0,2);
	
	printf("Creating Threads.\n");
	int index = 0;
		//creating B and C threads.
	pthread_create(&threadTypeB,NULL,(void*)&function_for_typeB,(void*)messageForThreadTypeB);//creating thread type B
	pthread_create(&threadTypeC,NULL,(void*)&function_for_typeC,(void*)messageForThreadTypeC);//creating thread type C
	
	
	for(index=0;index<NO_OF_THREADS_TYPE_A;index++){//creating typeA threads
		pthread_create(&threadsTypeA[index],NULL,(void *) &function_for_typeA, (void *)messagesForThreadsTypeA[index]);
	}
	printf("Done.\n");
	
	printf("Started Working.\n");
	for(index=0;index<NO_OF_THREADS_TYPE_A;index++){//joining typeA threads
		pthread_join(threadsTypeA[index],NULL);
	}
	pthread_join(threadTypeC,NULL);
	pthread_join(threadTypeB,NULL);
	//joining threads
	

	//destroying semaphores.
	sem_destroy(&mutex);
	sem_destroy(&resourceType1);
	sem_destroy(&mutexForBandC);
	printf("Done.\n");
	
	
	return 0;
}



void function_for_typeA (void *ptr){

	
	while(emptyTxt[0]==0 || emptyTxt[1]==0){//loops until all the resource type 1 files are empty.
	
		char *message = (char *)ptr;
	
		int randomNumber=rand()%10+1;//creating a random number n range 1 to 10
	
		
		int semaphoreValue;
		int index = 0;
	
		for(index=0;index<randomNumber;index++){//thread enteres its critical section randomNumber times and each critical section it only cut and past a single line.
	
		
			sem_wait(&resourceType1);//to decide which resource type 1 is avaiable. 
			sem_wait(&mutex);//locks resources.
			//start critical section for only one line
			
			sem_getvalue(&resourceType1,&semaphoreValue);//gets the semaphore value to understand which of the resource type 1 is selected.
			if(emptyTxt[0]==1){//becuase most of times semaphoreValue ==0 , we need to make the thread to work on the other resource file if the current resource 1 file is empty .
				
				cut_paste(1,message);
				
			}else if(emptyTxt[1]==1){
				
				cut_paste(0,message);
				
			}else{
			cut_paste(semaphoreValue,message );//cuts and pastes function that I described in detail later.
			}

			
			

			
			

			//end of critical section for only one line
	
			
			sem_post(&resourceType1);//relaise the resource.
			sem_post(&mutex);//relaise the mutex
			usleep(10);//sleep 10 us for avoiding starvation for other threads.
			
		}
	}
	
	pthread_exit(0);
}

void function_for_typeB (void *ptr){

/*
	type a threads can chage their resource 1 files if their current resource 1 file is empty so I made a control mechanisim to check that condition in the while loop to avoid deadlock.  

*/

		char *message = (char *)ptr;
		int semaphoreValue;
				
		while(emptyTxt[0]==0 || emptyTxt[1]==0){
			
			sem_wait(&resourceType1);
			if(resourceType2Count>0 && sem_trywait(&mutex)==0){//checks that resource type 2 file is created or not and if it is created,try to block it.
					sem_wait(&mutexForBandC);//blocks type c threads
		
		
					delete_prime_line_r2();
		
					sem_post(&mutexForBandC);

					sem_post(&mutex);
					usleep(10);//sleep 10 us for avoiding starvation for other threads.
			}

			sem_getvalue(&resourceType1,&semaphoreValue);
			if(emptyTxt[0]==1 &&semaphoreValue==0){//control mechanisim that I described above
				goto flag;
			}else if(emptyTxt[1]==1 && semaphoreValue==1){
				goto flag;
			}else if(semaphoreValue==0 ||  (emptyTxt[0]==1 && semaphoreValue==1) ||(emptyTxt[1]==1 && semaphoreValue==0)){
				sem_wait(&mutex);
			delete_prime_line_r1(semaphoreValue);
				sem_post(&mutex);
				
			}else{

			delete_prime_line_r1(semaphoreValue);
			}

			flag:
			
			sem_post(&resourceType1);
			usleep(10);//sleep 10 us for avoiding starvation for other threads.
		}
	
		sem_wait(&mutexForBandC);// blocking thread c to enter its critical section for the scenerio that is resource type 2 is not awaiable for thread b and all resource type 1 files are empty.
		
		
		delete_prime_line_r2();
		
		sem_post(&mutexForBandC);
		usleep(10);//sleep 10 us for avoiding starvation for other threads.

		pthread_exit(0);
}



void function_for_typeC (void *ptr){

/*
	type a threads can chage their resource 1 files if their current resource 1 file is empty so I made a control mechanisim to check that condition in the while loop to avoid deadlock.  

*/
	char *message = (char *)ptr;
	sem_wait(&resourceType1);
			if(resourceType2Count>0 && sem_trywait(&mutex)==0){//checks that resource type 2 file is created or not and if it is created,try to block it.
					sem_wait(&mutexForBandC);//blocks type b threads
		
		
					delete_negative_line_r2();
		
					sem_post(&mutexForBandC);

					sem_post(&mutex);
					usleep(10);//sleep 10 us for avoiding starvation for other threads.
			}
	
	int semaphoreValue;
	while(emptyTxt[0]==0 || emptyTxt[1]==0){
		sem_wait(&resourceType1);
		
		sem_getvalue(&resourceType1,&semaphoreValue);
		if(emptyTxt[0]==1 &&semaphoreValue==0){//control mechanisim that I described above
				goto flag;
			}else if(emptyTxt[1]==1 && semaphoreValue==1){
				goto flag;
			}else if(semaphoreValue==0 || (emptyTxt[0]==1 && semaphoreValue==1)||(emptyTxt[1]==1 && semaphoreValue==0)){
				sem_wait(&mutex);
			delete_negative_line_r1(semaphoreValue);
				sem_post(&mutex);
			}else{
			delete_negative_line_r1(semaphoreValue);
			}	
		flag:
		
		sem_post(&resourceType1);
		usleep(10);//sleep 10 us for avoiding starvation for other threads.
	}
	
	sem_wait(&mutexForBandC);// blocking thread b to enter its critical section for the scenerio that is resource type 2 is not awaiable for thread b and all resource type 1 files are empty.
	
	
	delete_negative_line_r2();
	
	sem_post(&mutexForBandC);
	usleep(10);//sleep 10 us for avoiding starvation for other threads.

	pthread_exit(0);
}
void cut_paste(int semaphoreValue,char *message ){
			
			char line[50];
			int lineCount=0;
			resourceType2Count++;
			//printf("%s is in cs\n", message);
			FILE* oldFile = fopen(txtNames[semaphoreValue], "r");//original file 
			FILE* newFile = fopen(tmpTxtNames[semaphoreValue], "w+");//replica file
			FILE* resource2=fopen("resource2.txt","a");//creating empty resource type 2 file
			if (oldFile != NULL && newFile != NULL){	
				
				if(fgets(line, sizeof(line), oldFile) != NULL){
					fprintf(resource2, "%s", line);//appends the first line to resource type 2 file
					while(fgets(line, sizeof(line), oldFile) != NULL){
									
							fprintf(newFile, "%s", line);//filling the replica file that is not contains the appended line.
									
					}
			    	}else{
					
					emptyTxt[semaphoreValue]=1;//if txt is empty we set it value to 1.
				}
			}

			else
			   	printf("couldn't open files\n");

			fclose(oldFile);
			fclose(newFile);
			fclose(resource2);	

			remove(txtNames[semaphoreValue]);//deleting the old original file.
			rename( tmpTxtNames[semaphoreValue], txtNames[semaphoreValue] );// renaming replica file to original name.

}



int check_prime_number(int number){//classical prime number algorithm
	int i, flag = 0;
	int prime=1;
	int notPrime=0;
   if (number==0 || number ==1){
	return notPrime;
	}

    for(i=2; i<=number/2; ++i)
    {
        
        if(number%i==0)
        {
            flag=1;
            break;
        }
    }

    if (flag==0){

       return prime;
	}
    else{

        return notPrime;
	}

}

void delete_prime_line_r1(int semaphoreValue){
			int txtElement;
			char line[50];
			int primeCount=0;
			//printf("ThreadB is in cs(resourceType1)\n" );
			FILE* oldFile = fopen(txtNames[semaphoreValue], "r");//original file 
			FILE* newFile = fopen(tmpTxtNames[semaphoreValue], "w+");//replica file
			
		if (oldFile != NULL && newFile != NULL){
				
				
					while(fgets(line, sizeof(line), oldFile) != NULL){
						txtElement=atoi(line);
						if(txtElement>=0){//because we run thread c and thread b at the same time, we can not guarantee that thread c works first and remove all negative numbers so we need to check that statement.
							if(check_prime_number(txtElement)==0){		
								fprintf(newFile, "%s", line);// filling the replica file with non prime numbers.
								
							}else{
								primeCount++;
							}
						}else{
								fprintf(newFile, "%s", line);
						}			
					}
			    	
			}

			else
			   	printf("couldn't open files\n");

			fclose(oldFile);
			fclose(newFile);
				


			remove(txtNames[semaphoreValue]);//deleting the old original file.
			rename( tmpTxtNames[semaphoreValue], txtNames[semaphoreValue] );// renaming replica file to original name.


}




void delete_prime_line_r2(){//creates a replica file without prime numbers and then after deleting original file renames the replica one . 
			int txtElement;
			char line[50];
			int primeCount=0;
			//printf("ThreadB is in cs(resourceType2)\n");
			FILE* oldFile = fopen("resource2.txt", "r");//original file
			FILE* newFile = fopen("tmpresource2.txt", "w+");//replica file
			
			if (oldFile != NULL && newFile != NULL){
				
				
					while(fgets(line, sizeof(line), oldFile) != NULL){
						txtElement=atoi(line);
						if(txtElement>=0){//because we run thread c and thread b at the same time, we can not guarantee that thread c works first and remove all negative numbers so we need to check that statement.
							if(check_prime_number(txtElement)==0){		
								fprintf(newFile, "%s", line);// filling the replica file with non prime numbers.
								
							}else{
								primeCount++;
							}
						}else{
								fprintf(newFile, "%s", line);
						}			
					}
			    	
			}

			else
			   	printf("couldn't open files\n");

			fclose(oldFile);
			fclose(newFile);
				

			remove("resource2.txt");//deleting the old original file.
			rename( "tmpresource2.txt", "resource2.txt" );// renaming replica file to original name.

}
void delete_negative_line_r1(int semaphoreValue){

			int txtElement;
			char line[50];
			//printf("ThreadC is in cs(resourceType1)\n");
			FILE* oldFile = fopen(txtNames[semaphoreValue], "r");//original file 
			FILE* newFile = fopen(tmpTxtNames[semaphoreValue], "w+");//replica file
			
			if (oldFile != NULL && newFile != NULL){
				
				
					while(fgets(line, sizeof(line), oldFile) != NULL){
						txtElement=atoi(line);
						if(txtElement>=0){
									
								fprintf(newFile, "%s", line);//filling the replica file with non negative numbers
								
							
						}
								
									
					}
			    	
			}

			else
			   	printf("couldn't open files\n");

			fclose(oldFile);
			fclose(newFile);
				

			
			remove(txtNames[semaphoreValue]);//deleting the old original file.
			rename( tmpTxtNames[semaphoreValue], txtNames[semaphoreValue] );// renaming replica file to original name.


}
void delete_negative_line_r2(){//creates a replica file without negative numbers and then after deleting original file renames the replica one . 
			int txtElement;
			char line[50];
			//printf("ThreadC is in cs(resourceType2)\n");
			FILE* oldFile = fopen("resource2.txt", "r");//original file
			FILE* newFile = fopen("tmpresource2.txt", "w+");//replica file
			
			if (oldFile != NULL && newFile != NULL){
				
				
					while(fgets(line, sizeof(line), oldFile) != NULL){
						txtElement=atoi(line);
						if(txtElement>=0){
									
								fprintf(newFile, "%s", line);//filling the replica file with non negative numbers
								
							
						}
								
									
					}
			    	
			}

			else
			   	printf("couldn't open files\n");

			fclose(oldFile);
			fclose(newFile);
				

			remove("resource2.txt");//deleting the old original file.
			rename( "tmpresource2.txt", "resource2.txt" );// renaming replica file to original name.


}
