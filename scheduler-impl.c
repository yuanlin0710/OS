// CSCI3150 Asg3
// Name: YUAN Lin 
// SID: 1155141399


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);

// Implement by students

Process MinProcByPID(Process x, Process y){
    Process min_proc;
    if(x.process_id<y.process_id){
        min_proc = x;
    }
    else{
        min_proc = y;
    }
    return min_proc;
}


void SortProcessByPID(Process* p, int num){
    Process* a = p;
    Process* b = (Process*)malloc(num * sizeof(Process));
    int seg, start;
    for (seg = 1; seg < num; seg += seg) {
        for (start = 0; start < num; start += seg + seg) {
            int low = start, mid = min(start + seg, num), high = min(start + seg + seg, num);
            int k = low;
            int start1 = low, end1 = mid;
            int start2 = mid, end2 = high;
            while (start1 < end1 && start2 < end2){
                Process minproc = MinProcByPID(a[start1], a[start2]);
                if (minproc.process_id == a[start1].process_id)
                    b[k++] = a[start1++];
                else 
                    b[k++] = a[start2++];
            }
            
            while (start1 < end1)
                b[k++] = a[start1++];
            while (start2 < end2)
                b[k++] = a[start2++];
        }
        Process* tmp = a;
        a = b;
        b = tmp;
    }
    if (a != p) {
        int i;
        for (i = 0; i < num; i++)
            b[i] = a[i];
        b = a;
    }
    free(b);
}






void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
    printf("Process number: %d\n", proc_num);
    for (int i = 0;i < proc_num; i++)
        printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    for (int i = 0;i < queue_num; i++){
        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
    }

    // my implement
    int time_slice = ProcessQueue[queue_num-1]->time_slice;
    int slice_used = 0;
    int slice_offset = 0;
    int current_time = 0;
    int current_allotment = ProcessQueue[queue_num-1]->allotment_time;
    int current_runningqueue = (queue_num-1);
    Process front_proc;
    Process de_proc;
    int flag_allQueueEmpty = 1;
    int last_print_time = 0;

    for(int i =0; i<proc_num; i++){
        proc[i].service_time = 0;

    }

    // process's running time on the current queue (to check whether exceed allotment) will be stored in service time

    do{
        int flag_printed = 0;

        // check dequeue
        if(IsEmptyQueue(ProcessQueue[current_runningqueue])){
            // check current running queue is empty or not
        }
        else{
            //printf("checkpoint1");

            front_proc = FrontQueue(ProcessQueue[current_runningqueue]);

            if(front_proc.execution_time==0){
                // if finish, dequeue
                de_proc = DeQueue(ProcessQueue[current_runningqueue]);


                //print for finish
                if(current_time % time_slice == slice_offset){
                    // if use up a slice & finish at the same time
                    outprint(current_time-time_slice, current_time, de_proc.process_id, de_proc.arrival_time, de_proc.execution_time);
                    //printf("checkpoint1\n");
                    flag_printed =1;
                    last_print_time = current_time;

                }else{
                    // if finish early
                    slice_used = (current_time - slice_offset) % time_slice;
                    //slice_offset = current_time % time_slice;
                    if(flag_printed == 0){
                        outprint(current_time-slice_used, current_time, de_proc.process_id, de_proc.arrival_time, de_proc.execution_time);
                        //printf("checkpoint2\n");
                        flag_printed =1;
                        last_print_time = current_time;
                    }
                }

            }
            else if((front_proc.service_time % time_slice == 0) && front_proc.service_time){
                
                // if finish one slice
                // print
                if(flag_printed == 0){
                    outprint(current_time-time_slice, current_time, front_proc.process_id, front_proc.arrival_time, front_proc.execution_time);
                    //printf("checkpoint3\n");
                    int con1 = (front_proc.service_time % time_slice == 0);
                    int con2 = front_proc.service_time;
                    //printf("cond 1 %d cond 2 %d", con1, con2 );
                    flag_printed = 1;
                    last_print_time = current_time;
                }
                

                if(front_proc.service_time<current_allotment){
                    // if finish one slice but not exceed allotment
                    // make this process go to tail of CURRENT queue
                    
                    de_proc = DeQueue(ProcessQueue[current_runningqueue]);
                    ProcessQueue[current_runningqueue] = EnQueue(ProcessQueue[current_runningqueue], de_proc);

                }
                else{
                    // if exceed allotment, go to lower queue
                    if(current_runningqueue==0){
                        // if already at lowest level, only set the service time to 0
                        front_proc = DeQueue(ProcessQueue[current_runningqueue]);
                        front_proc.service_time = 0;
                        ProcessQueue[current_runningqueue] = EnQueue(ProcessQueue[current_runningqueue], front_proc);                     
                        //modify
                    }
                    else{
                        // else, lower 1 level, and set service time to 0
                        de_proc = DeQueue(ProcessQueue[current_runningqueue]);
                        de_proc.service_time = 0;
                        ProcessQueue[current_runningqueue-1] = EnQueue(ProcessQueue[current_runningqueue-1], de_proc); 
                    }

                }

            }
        
        }


        // enqueue
        //for(int i = queue_num-1; i >=0; i--){
        //    // check whether all queues are empty
        //    if(!IsEmptyQueue(ProcessQueue[i])){
        //        flag_allQueueEmpty = 0;
        //        break;
        //    }
        //    
        //}
        

        //if(flag_allQueueEmpty==1){
        //    // if all queues are empty, reset offset/running queue/allotment
        //    current_runningqueue = queue_num-1;
        //    current_allotment = ProcessQueue[queue_num-1]->allotment_time;
        //   time_slice = ProcessQueue[queue_num-1]->time_slice;
        //    slice_offset = current_time % time_slice;

        //}

        for(int i = 0; i<proc_num; i++){
            // enqueue process
            if (current_time == proc[i].arrival_time){
                ProcessQueue[queue_num-1] = EnQueue(ProcessQueue[queue_num-1], proc[i]);
                flag_allQueueEmpty = 0;
            }
        }
        //printf("checkpoint2");


        // periodic boost
        if(current_time % period == 0 && current_time != 0){
            if(flag_printed==0){
                // if not printed, print current
                front_proc = FrontQueue(ProcessQueue[current_runningqueue]);
                outprint(last_print_time, current_time, front_proc.process_id, front_proc.arrival_time, front_proc.execution_time);
                //printf("checkpoint4\n");
                last_print_time = current_time;
                flag_printed = 1;
                
            }

            // reset offset, current running queue, allotment, time_slice
            time_slice = ProcessQueue[queue_num-1]->time_slice;
            current_allotment = ProcessQueue[queue_num-1]->allotment_time;
            current_runningqueue = queue_num-1;
            slice_offset = current_time % time_slice;

            

            // queue all unfinished processes to the top most queue by PID order
            // reset service time of each process to 0
            int tmp_process_num = 0;
            for(int j = 0; j<queue_num; j++){
                tmp_process_num+= Length(ProcessQueue[j]);
            }
            if(tmp_process_num != 0){
                // if there is any process in queue, boost
                Process* tmp_process = (Process*)malloc(tmp_process_num * sizeof(Process));

                // dequeue each process and store into tmp_process
                // at the same time set service time to 0
                for(int j = 0; j<tmp_process_num; j++){
                    for(int m = 0; m<queue_num; m++){
                        if(!IsEmptyQueue(ProcessQueue[m])){
                            Process to_tmp_proc = DeQueue(ProcessQueue[m]);
                            tmp_process[j].process_id = to_tmp_proc.process_id;   //double check here
                            tmp_process[j].arrival_time = to_tmp_proc.arrival_time;
                            tmp_process[j].execution_time = to_tmp_proc.execution_time;
                            tmp_process[j].service_time = 0;
                            break;
                        }
                    }
                }
            
                // sort by PID
                SortProcessByPID(tmp_process, tmp_process_num);

                // enqueue back to queue 0
                for(int j = 0; j<tmp_process_num; j++){
                    ProcessQueue[queue_num-1] = EnQueue(ProcessQueue[queue_num-1], tmp_process[j]);
                }

                free(tmp_process);

            }






        }


        // task selection & execution
        // I think the conditions for selecting a new process (rather than executing the old) == need to print
        


        if(flag_printed == 0){
            // no need to select a new process
            // if the list is not empty, just run the old
            

            if(!IsEmptyQueue(ProcessQueue[current_runningqueue])){                
                // modify
                front_proc = DeQueue(ProcessQueue[current_runningqueue]);
                front_proc.execution_time--;
                front_proc.service_time++;
                //printf("%d\n", front_proc.execution_time);
                //printf("a%d\n", front_proc.service_time);
                ProcessQueue[current_runningqueue] = AddTail(ProcessQueue[current_runningqueue], front_proc);                
            }
            

        }
        else{
            // need to select a new process; so need to pick the topmost, queue num-1 by default
            current_runningqueue = queue_num-1;
            for (int i = (queue_num-1); i >= 0; i--){
                if(!IsEmptyQueue(ProcessQueue[i])){
                    current_runningqueue = i;
                    break;
                }
            }
            // update allotment, slice, running queue, offset
            current_allotment = ProcessQueue[current_runningqueue]->allotment_time;
            time_slice = ProcessQueue[current_runningqueue]->time_slice;
            slice_offset = current_time % time_slice;

            // run
            if(!IsEmptyQueue(ProcessQueue[current_runningqueue])){
                // modify
                front_proc = DeQueue(ProcessQueue[current_runningqueue]);
                front_proc.execution_time--;
                front_proc.service_time++;
                //printf("%d\n", front_proc.execution_time);
                //printf("b%d\n", front_proc.service_time);
                ProcessQueue[current_runningqueue] = AddTail(ProcessQueue[current_runningqueue], front_proc);      
            }            

        }

                       

        // stop condition: current time >= last process's arrival time && no more process in any queue
        flag_allQueueEmpty = 1;
        for(int i = 0; i < queue_num; i++){
            // check whether all queues are empty
            if(!IsEmptyQueue(ProcessQueue[i])){
                flag_allQueueEmpty = 0;
                break;
            }
        }        
        
        
        if (current_time >= proc[proc_num-1].arrival_time && flag_allQueueEmpty){
            break;
        }

    }while (++current_time);



}

