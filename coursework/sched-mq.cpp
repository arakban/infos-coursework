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
    const int realtime = 0;
    const int interactive = 1;
    const int normal = 2;
    const int daemon = 3;
    const int idle = 4;

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
        // implement lock first
        UniqueIRQLock l;
        //task eligible to run
		
        // runqueue.enqueue(&entity);
    }

    /**
     * Called when a scheduling entity is no longer eligible for running.
     * @param entity
     */
    void remove_from_runqueue(SchedulingEntity& entity) override
    {   
        // implement lock first
        UniqueIRQLock l;

        int entity_priority = entity.priority();
        String entity_name = entity.name().c_str();
        
        //task no longer eligible, 
        if (entity_priority == realtime) {
            realtime_runqueue.remove(&entity);
        }
		
        if (entity_priority == INTERACTIVE) {
            interactive_runqueue.remove(&entity);
        }

        if (entity_priority == NORMAL) {
            normal_runqueue.remove(&entity);
        }

        if (entity.priority() == DAEMON) {
            daemon_runqueue.remove(&entity);
        }
        
        // runqueue.remove(&entity);
    }

    /**
     * Called every time a scheduling event occurs, to cause the next eligible entity
     * to be chosen.  The next eligible entity might actually be the same entity, if
     * e.g. its timeslice has not expired.
     */
    SchedulingEntity *pick_next_entity() override
    {   
        
        //look at the different ques of priorities of the thread
        UniqueIRQLock l;
        int entity_priority = entity.priority()
        int entity_name = entity.name().c_str()

        if (realtime_runqueue.count() == 0) return NULL;
        if (runqueue.count() > 0) return realtime_runqueue.first();
        

        else {
            //get the first process
            SchedulingEntity *entity = runqueue.pop(); 
            //put at the front of queue and return 
            runqueue.enqueue(entity);
            return entity;
        }

        //round-robin fashion to finish highest priority queue 
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