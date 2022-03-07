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

    }

    /**
     * Called when a scheduling entity becomes eligible for running.
     * @param entity
     */
    void add_to_runqueue(SchedulingEntity& entity) override
    {   
        syslog.messagef(LogLevel::DEBUG, "Adding scheduling entity to runqueue\n");
        // implement lock first so we don't run out of time
        UniqueIRQLock l;
        //syslog.messagef(LogLevel::DEBUG, "Lock implemented\n");
        
        //add runnable task to runqueue of the correct priority 
        int entity_priority = entity.priority();
        //String entity_name = entity.name().c_str();
        
        if (entity_priority == SchedulingEntityPriority::REALTIME) {
            //syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} of priority REALTIME/%s to runqueue",entity_name,entity_priority);
            realtime_runqueue.enqueue(&entity);
        }
		
        else if (entity_priority == SchedulingEntityPriority::INTERACTIVE) {
            //syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} of priority INTERACTIVE/%s to runqueue",entity_name,entity_priority);
            interactive_runqueue.enqueue(&entity);
        }

        else if (entity_priority == SchedulingEntityPriority::NORMAL) {
            //syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} of priority NORMAL/%s to runqueue",entity_name,entity_priority);
            normal_runqueue.enqueue(&entity);
        }

        else {
            //syslog.messagef(LogLevel::DEBUG, "Adding entitity {%s} of priority DAEMON/%s to runqueue",entity_name,entity_priority);
            daemon_runqueue.enqueue(&entity);
        }
    }

    /**
     * Called when a scheduling entity is no longer eligible for running.
     * @param entity
     */
    void remove_from_runqueue(SchedulingEntity& entity) override
    {   
        syslog.messagef(LogLevel::DEBUG, "Removing scheduling entity to runqueue\n");
        // implement lock first so we don't run out of time
        UniqueIRQLock l;
        //syslog.messagef(LogLevel::DEBUG, "Lock implemented\n");

        int entity_priority = entity.priority();
        //String entity_name = entity.name().c_str();
        
        //task is no longer eligible, so remove from its runqueue  
        if (entity_priority == SchedulingEntityPriority::REALTIME) {
            //syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority REALTIME/%s to runqueue",entity_name,entity_priority);
            realtime_runqueue.remove(&entity);
        }
		
        else if (entity_priority == SchedulingEntityPriority::INTERACTIVE) {
            //syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority INTERACTIVE/%s to runqueue",entity_name,entity_priority);
            interactive_runqueue.remove(&entity);
        }

        else if (entity_priority == SchedulingEntityPriority::NORMAL) {
            //syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority NORMAL/%s to runqueue",entity_name,entity_priority);
            normal_runqueue.remove(&entity);
        }

        else {
            //syslog.messagef(LogLevel::DEBUG, "Removing entitity {%s} of priority DAEMON/%s to runqueue",entity_name,entity_priority);
            daemon_runqueue.remove(&entity);
        }
        
    }

    /**
     * Called every time a scheduling event occurs, to cause the next eligible entity
     * to be chosen.  The next eligible entity might actually be the same entity, if
     * e.g. its timeslice has not expired.
     */
    SchedulingEntity *pick_next_entity() override
    {   
        //look at the different queues of priorities of the thread, return top of queue if last process left
        //round-robin fashion to finish/point-to the current process to non-empty highest priority queue 
        //otherwise go to the next priority queue 
        
        //start with realtime queue 
        if (!realtime_runqueue.empty())  {
            //syslog.messagef(LogLevel::DEBUG, "REALTIME process left to execute\n");
            UniqueIRQLock l;
            //get the first entity on top of the queue 
            SchedulingEntity *entity = realtime_runqueue.dequeue(); 
            //put at the end of queue and return entity 
            realtime_runqueue.enqueue(entity);
            return entity; 
        } 
        
        //realtime queue is empty so move to interactive 
        else if (!interactive_runqueue.empty())  {
            //syslog.messagef(LogLevel::DEBUG, "INTERACTIVE process left to execute\n");
            UniqueIRQLock l;
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            interactive_runqueue.enqueue(entity);
            return entity; 
        } 
        
        //interactive  queue is empty so move to daemon
        else if (!normal_runqueue.empty())  {
            //syslog.messagef(LogLevel::DEBUG, "NORMAL process left to execute\n");
            UniqueIRQLock l;
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            normal_runqueue.enqueue(entity);
            return entity; 
        } 
        
        //daemon queue is empty so return null
        else if (!daemon_runqueue.empty()) {
            //syslog.messagef(LogLevel::DEBUG, "Nothing left to execute\n");
            return NULL;
        }
        
        //get top of daemon queue
        else {
            //syslog.messagef(LogLevel::DEBUG, "DAEMON process left to execute\n");
            UniqueIRQLock l;
            SchedulingEntity *entity = daemon_runqueue.dequeue(); 
            daemon_runqueue.enqueue(entity);
            return entity;  
        }
        //syslog.messagef(LogLevel::DEBUG, "");
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