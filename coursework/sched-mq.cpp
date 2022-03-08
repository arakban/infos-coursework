/*
 * The Priority Task Scheduler
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 1
 */

#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>
#include <infos/util/string.h>

using namespace infos::kernel;
using namespace infos::util;

/**
 * A Multiple Queue priority scheduling algorithm
 */
class MultipleQueuePriorityScheduler : public SchedulingAlgorithm
{
public:
    /**
     * Returns the friendly name of the algorithm, for debugging and selection purposes.
     */
    const char* name() const override { return "mq"; }

    /**
     * Called during scheduler initialisation.
     */
    void init()
    {
        syslog.messagef(LogLevel::DEBUG, "Scheduling-MQ algo init\n");
    }

    /**
     * Called when a scheduling entity becomes eligible for running.
     * @param entity
     */
    void add_to_runqueue(SchedulingEntity& entity) override
    {           
        syslog.messagef(LogLevel::DEBUG, "OS has asked to add enitity to runqueue, getting priority and name");
        

        SchedulingEntityPriority entity_priority = entity.priority();
        syslog.messagef(LogLevel::DEBUG, "Got priority!");

        infos::util::String entity_name = entity.name().c_str();
        syslog.messagef(LogLevel::DEBUG, "Got name!");

        //add runnable task to runqueue of the correct priority 
        syslog.messagef(LogLevel::DEBUG, "Adding scheduling entity {%s} to runqueue--%s--",entity_name,entity_priority); 

        //REALTIME
        if (entity_priority == SchedulingEntityPriority::REALTIME) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for {%s} : REALTIME",entity_name);
            
            realtime_runqueue.enqueue(&entity);
            syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} from REALTIME",entity_name);
        }
		
        //INTERACTIVE
        else if (entity_priority == SchedulingEntityPriority::INTERACTIVE) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for {%s} : INTERACTIVE",entity_name);
            
            interactive_runqueue.enqueue(&entity);
            syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} from INTERACTIVE",entity_name);
        }

        //NORMAL
        else if (entity_priority == SchedulingEntityPriority::NORMAL) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for entitity {%s} : NORMAL",entity_name);
            
            normal_runqueue.enqueue(&entity);
            syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} from NORMAL",entity_name);
        }

        //DAEMON
        else  if (entity_priority == SchedulingEntityPriority::DAEMON) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for entitity {%s} : DAEMON",entity_name);
            
            daemon_runqueue.enqueue(&entity);
            syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority DAEMON",entity_name);
        }

        //IDLE - should not happen
        else {

            syslog.messagef(LogLevel::DEBUG, "IDLE process so nothing to add");
        }
    }

    /**
     * Called when a scheduling entity is no longer eligible for running.
     * @param entity
     */
    void remove_from_runqueue(SchedulingEntity& entity) override
    {   
        
        syslog.messagef(LogLevel::DEBUG, "OS has asked to remove enitity to runqueue, getting priority and name");

        int entity_priority = entity.priority();
        syslog.messagef(LogLevel::DEBUG, "Got priority!");

        infos::util::String entity_name = entity.name().c_str();
        syslog.messagef(LogLevel::DEBUG, "Got name!");
        
        //task is no longer eligible, so try to remove from its runqueue 
        syslog.messagef(LogLevel::DEBUG, "Removing scheduling entity {%s} from --%s--runqueue",entity_name,entity_priority); 
        
        //REALTIME
        if (entity_priority == SchedulingEntityPriority::REALTIME) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for {%s} : REALTIME",entity_name);
            
            realtime_runqueue.remove(&entity);
            syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} from REALTIME",entity_name);
        }
		

        //INTERACTIVE
        else if (entity_priority == SchedulingEntityPriority::INTERACTIVE) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for {%s} : INTERACTIVE",entity_name);

            interactive_runqueue.remove(&entity);
            syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} from INTERACTIVE ",entity_name);
        }

        //NORMAL
        else if (entity_priority == SchedulingEntityPriority::NORMAL) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for entitity {%s} : NORMAL",entity_name);

            normal_runqueue.remove(&entity);
            syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} from NORMAL",entity_name);
        }

        //DAEMON
        else if (entity_priority == SchedulingEntityPriority::DAEMON) {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "Lock implemented for entitity {%s} : DAEMON",entity_name);
    
            daemon_runqueue.remove(&entity);
            syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority DAEMON",entity_name);
        }

        //IDLE - should not happen
        else {
            syslog.messagef(LogLevel::DEBUG, "IDLE process to add");
        }
    }

    /**
     * Called every time a scheduling event occurs, to cause the next eligible entity
     * to be chosen.  The next eligible entity might actually be the same entity, if
     * e.g. its timeslice has not expired.
     */
    SchedulingEntity *pick_next_entity() override
    {   
        syslog.messagef(LogLevel::DEBUG, "OS has asked for a scheduling event occurs, to cause the next eligible entity to be chosen");
        
        //look at the different queues of priorities of the thread, return top of queue if last process left
        //round-robin fashion to finish/point-to the current process to non-empty highest priority queue 
        //otherwise go to the next priority queue 
        
        //start with realtime queue 
        if (!realtime_runqueue.empty())  {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "REALTIME process(s) left to execute\n");
            //get the first entity on top of the queue 
            SchedulingEntity *entity = realtime_runqueue.dequeue(); 
            //put at the end of queue and return entity 
            realtime_runqueue.enqueue(entity);
            return realtime_runqueue.first(); 
        } 
        
        //realtime queue is empty so move to interactive 
        else if (!interactive_runqueue.empty())  {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "INTERACTIVE process(s) left to execute\n");
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            interactive_runqueue.enqueue(entity);
            return  interactive_runqueue.first(); 
        } 
        
        //interactive  queue is empty so move to daemon
        else if (!normal_runqueue.empty())  {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "NORMAL process(s) left to execute\n");
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            normal_runqueue.enqueue(entity);
            return normal_runqueue.first(); 
        } 
        
        //daemon queue is empty so return null
        else if (daemon_runqueue.empty()) {
            syslog.messagef(LogLevel::DEBUG, "Nothing left to execute\n");
            return NULL;
        }
        
        //get top of daemon queue
        else  {
            UniqueIRQLock l;
            syslog.messagef(LogLevel::DEBUG, "DAEMON process left to execute\n");
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            daemon_runqueue.enqueue(entity);
            return daemon_runqueue.first();  
        }
    }

private:
    // A list to keep track of the current runqueue 
    List<SchedulingEntity *> realtime_runqueue;
    List<SchedulingEntity *> interactive_runqueue;
    List<SchedulingEntity *> normal_runqueue;
    List<SchedulingEntity *> daemon_runqueue;

};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(MultipleQueuePriorityScheduler);