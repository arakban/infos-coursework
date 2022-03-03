/*
 * The Priority Task Scheduler
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 1
 */

#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>

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
        const int realtime = 1;
        const int interactive = 2;
        const int normal = 3;
        const int daemon = 4;
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
		runqueue.enqueue(&entity);
    }

    /**
     * Called when a scheduling entity is no longer eligible for running.
     * @param entity
     */
    void remove_from_runqueue(SchedulingEntity& entity) override
    {   
        // implement lock first
        UniqueIRQLock l;
        //task no longer eligible 
		runqueue.remove(&entity);
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
        if (runqueue.count()==0) {
            return NULL;
        }

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
    List<SchedulingEntity *> runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(MultipleQueuePriorityScheduler);