#include <stdio.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>    
#include <unistd.h>   

#define BILLION 1000000000L

//void function for formatting and printing the <state> message on stderr
void state(float time, uint16_t slots, uint32_t temperature){
	//char array containing the elements which represent the slots
	//each slot having X-for taken and " " for not taken
	char slots_t[]="1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]";
	uint8_t counter = 2;

	//check for every bit in the parameter - slots(number) if it is zero or one
	//zero stands for not taken slot
	//one stands for taken slot
	for(int i = 0; i < 16; i++){
		uint8_t bit = (slots >> i) & 1;
		if(i>8)
			++counter;
		if(bit==1)
			slots_t[counter] = 'X';
		
		counter+=5;
	}

	//temperature is equal to temperature/100
	//and because it is in Kelvin, we subtract 273.13 to make it in celsius
	fprintf(stderr,"[%.3f] <state> temp: %.2f°C, slots: %s\n", time, temperature/100 -273.15,slots_t);

}

//void function for formatting and printing the <slot_text> message on stderr
void slot_text(float time, uint8_t identif, char text[]){
	fprintf(stderr,"[%.3f] <slot text> slot %u: %s\n",time, identif+1, text);
}

int main(int argc, char* argv[]) {
	//Structure containing the time of the start of the program 
	//and the time when the first element from the Toaster-message is read by the read function 
	struct timespec start,end;
	clock_gettime(CLOCK_MONOTONIC,&start);

	if (argc != 3)
		errx(1, "Should have exatly tree arguments - <record> or <replay> and <file>\n");

	//parameters for the <state> type of message
	uint16_t message;
	uint16_t slots;
	uint32_t temperature;

	//parameters for the <slot tex> type of message
       	uint8_t identif;    
       	char text[13];

	//read size	
    	ssize_t read_s;

	//parameter for the elapsed time since the start of the program
	float time=0.0;

      	if(strcmp(argv[1], "record") == 0){
		//parameter for file descriptor
	    	int fd1;
	       	//If opening file with file descriptor - fd1 fails, the program sends error message to stderr
	       	if((fd1 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	    		err(2,"%s\n", argv[2]);
	       	
		//read two bytes for the type of messege - <state> or <slot text>
		while ((read_s = read(0,&message,sizeof(message))) > 0){
			
			//getting the time after the first read	
		    	clock_gettime(CLOCK_MONOTONIC,&end);
			//making the time-parameter in seconds
			time = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
		
			//writes the time in the file given as argument of the program
			if(write(fd1,&time,sizeof(time)) != sizeof(time)){
			    	close(fd1);
			    	err(3,"Error while writing in %s\n",argv[2]);	    
			}
		
			//writes the message in the file given as argument of the program
			if(write(fd1,&message,sizeof(message)) != sizeof(message)){
                                close(fd1);
                                err(3,"Error while writing in %s\n", argv[2]);
                        }

		       	if(message == 0x0001){
			//reads from input and writes the slots in the file given as argument of the program	
				if((read_s = read(0,&slots,sizeof(slots))) > 0){
					if(write(fd1,&slots,sizeof(slots)) != sizeof(slots)){
						close(fd1);
						err(3,"Error while writing in %s\n", argv[2]);
					}
				}
				if(read_s == -1){
					close(fd1);
					err(4,"Error while reading toaster messages\n");
				}
				
				//reads from input and writes the temperature in the file
				//given as argument of the program
				if((read_s = read(0,&temperature,sizeof(temperature))) > 0){
					if(write(fd1,&temperature,sizeof(temperature)) != sizeof(temperature)){
						close(fd1);
						err(3,"Error while writing in %s", argv[2]);
					}
				}
				if(read_s == -1){
                                        close(fd1);
                                        err(4,"Error while reading toaster messages\n");
                                }

				//call of the function for printing on stderr
				state(time,slots,temperature);
			}
			else if(message == 0x0002){
				//reads from input and writes the identificator of the slot in the file
				//given as argument of the program 
				if((read_s = read(0,&identif,sizeof(identif))) > 0){
					if(write(fd1,&identif,sizeof(identif)) != sizeof(identif)){
						close(fd1);
						err(3,"Error while writing");
					}
				}
				if(read_s == -1){
                                        close(fd1);
                                        err(4,"Error while reading toaster messages\n");
                                }
			
				//reads from input and writes the text about the toaster
				//in the file given as argument of the program 
				if((read_s = read(0,&text,sizeof(text)) > 0)){
				      if(write(fd1,&text,sizeof(text)) != sizeof(text)){
					      close(fd1);
					      err(3,"Error while writing");
				      }	      
				}
				if(read_s == -1){
                                        close(fd1);
                                        err(4,"Error while reading toaster messages\n");
                                }

				//call of the function for printing on stderr
				slot_text(time, identif, text);
			}
			else {
				err(8,"Wrong format of the input!\n");
			}
		}
		if(read_s == -1){
		  	close(fd1);
  			err(4,"Error while reading toaster messages\n");  
		}

       	}
	else if(strcmp(argv[1], "replay") == 0){
		//parameter for the file descriptor of the file given as argument
		int fd2;    
	       	//If opening file with file descriptor - fd2 fails, the program sends error message to stderr
		if((fd2=open(argv[2], O_RDONLY)) == -1){
			err(4,"Could not open file %s\n",argv[2]);
		}

		//parameter used later to calculate the time for printing
		//the messages on stderr
		float prevTime = 0.0;

		//reading the time written on the file by record command
		while((read_s = read(fd2,&time,sizeof(time))) > 0){
			//turn seconds into microseconds
			//and sleep 
			usleep((time - prevTime)*1000000);
		
			prevTime=time;

			//writes on stdout
			if(write(1,&time,sizeof(time)) != sizeof(time)){
			   	close(fd2);
				err(6,"Error while writing on stdout\n");           
			}
			if((read_s = read(fd2,&message,sizeof(message))) > 0){
	       			if(write(1,&message,sizeof(message)) != sizeof(message)){
		       			close(fd2);
			       		err(6,"Error while writing on stdout\n");
	 			 }

				if(message == 0x0001){
					if((read_s = read(fd2,&slots,sizeof(slots))) > 0){
						 if(write(1,&slots,sizeof(slots)) != sizeof(slots)){
						      	 close(fd2);
						 	 err(6,"Error while writing on stdout\n");
						 }
					}		
			       		if(read_s == -1){
						close(fd2);
					       	err(5,"Error while reading from %s\n",argv[2]);
					}
					if((read_s = read(fd2,&temperature,sizeof(temperature))) > 0){
						if(write(1,&temperature,sizeof(temperature)) != sizeof(temperature)){
                                                         close(fd2);
                                                         err(6,"Error while writing on stdout\n");
                                                 }
					}
					if(read_s == -1){
                                                close(fd2);
                                                err(5,"Error while reading from %s\n",argv[2]);
                                        }
					//call of the function for printing on stderr
					state(time,slots,temperature);
				}
				else if(message == 0x0002){
					if(write(1,&message,sizeof(message)) != sizeof(message)){
						close(fd2);
						err(6,"Error while writing on stdout\n");
       					}
					if((read_s = read(fd2,&identif,sizeof(identif))) > 0){
						if(write(1,&identif,sizeof(identif)) != sizeof(identif)){
							close(fd2);
							err(6,"Error while writing on stdout\n");
				       		}
                                        }
					if(read_s == -1){
				       		close(fd2);
                                                err(5,"Error while reading from %s\n",argv[2]);
					}
                                        if((read_s = read(fd2,&text,sizeof(text))) > 0){
						if(write(1,&text,sizeof(text)) != sizeof(text)){
							close(fd2);
							err(6,"Error while writing on stdout\n");
						}
                                        }
					if(read_s == -1){
						close(fd2);
						err(5,"Error while reading from %s\n",argv[2]);
					}
					
					//call of the function for printing on stderr
					slot_text(time, identif, text);
				}
				else
					err(7,"Wrong format of the file!\n");		
			}
			if(read_s == -1){
				close(fd2);                
			       	err(5,"Error while reading from %s\n",argv[2]);
			}
		}
		if(read_s == -1){
			close(fd2);
			err(5,"Error while reading from %s\n", argv[2]);
		}
	}
	else
		err(9,"Commands are record or replay");
	
}
