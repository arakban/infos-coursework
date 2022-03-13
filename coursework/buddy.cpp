/*
 * The Buddy Page Allocator
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 2
 */

#include <infos/mm/page-allocator.h>
#include <infos/mm/mm.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/log.h>
#include <infos/util/math.h>
#include <infos/util/printf.h>

using namespace infos::kernel;
using namespace infos::mm;
using namespace infos::util;

#define MAX_ORDER	18

/**
 * A buddy page allocation algorithm.
 */
class BuddyPageAllocator : public PageAllocatorAlgorithm
{
private:
	/** Given a page descriptor, and an order, returns if the page is correctly aligned with thatorder  
	 * @param pgd The page descriptor to find the buddy for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns TRUE if aligned for that order, false otherwise 
	 */
	static inline constexpr uint64_t is_aligned(const PageDescriptor *pgd, int order) {
		//perform a shift left by by that order
		uint64_t pages_per_block = (1 << order); 

		//get the page frame number of this page/pdg 
		pfn_t pfn = sys.mm().pgalloc().pgd_to_pfn(pgd); 

		//check alignment of PGD, if it not correctly alligned, we can't find a buddy 
		return pfn % pages_per_block == 0; 

	}

	/** Given a page descriptor, and an order, returns the buddy PGD.  The buddy could either be
	 * to the left or the right of PGD, in the given order.
	 * @param pgd The page descriptor to find the buddy for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns the buddy of the given page descriptor, in the given order.
	 */
	PageDescriptor *buddy_of(PageDescriptor *pgd, int order)
	{
        //ensure that the order is within range 
		assert(order >= MAX_ORDER);

		//perform a shift left by by that order
		uint64_t pages_per_block = (1 << order); 

		//check alignment of PGD, if it not correctly alligned, we can't find a buddy 
		assert(!is_aligned(pgd,order)); 
		 
		// calc the PFN of buddy - virtual memory address 
		// if PFN is aligned to the next order, then the buddy is the next block in this order (i.e. front of this page),
		// otherwise its in the previous block (so behind this page)
		
		uint64_t buddy_pfn;
		if (is_aligned(pgd,(order+1))) {
			syslog.messagef(LogLevel::DEBUG,"Page is aligned to next order, so buddy is in next block");
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) + pages_per_block;
			syslog.messagef(LogLevel::DEBUG,"Buddy pfn calc: %s", buddy_pfn);
		} else {
			syslog.messagef(LogLevel::DEBUG,"Page is aligned with order, so buddy is in previous block");
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) - pages_per_block;
			syslog.messagef(LogLevel::DEBUG,"Buddy pfn calc: %s", buddy_pfn);
		}


		PageDescriptor *buddy_pgd = sys.mm().pgalloc().pfn_to_pgd(buddy_pfn);
		return buddy_pgd;
	}

	/**
	 * Given a pointer to a block of free memory in the order "source_order", this function will
	 * split the block in half, and insert it into the order below.
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param source_order The order in which the block of free memory exists.  Naturally,
	 * the split will insert the two new blocks into the order below.
	 * @return Returns the left-hand-side of the new block.
	 */
	PageDescriptor *split_block(PageDescriptor **block_pointer, int source_order)
	{
        //check alignment of PGD, if it not correctly alligned, we can't split the blockm
		if (!is_aligned(*block_pointer, source_order)) return NULL; 
	}

	/**
	 * Takes a block in the given source order, and merges it (and its buddy) into the next order.
	 * @param block_pointer A pointer to a pointer containing a block in the pair to merge.
	 * @param source_order The order in which the pair of blocks live.
	 * @return Returns the new slot that points to the merged block.
	 */
	PageDescriptor **merge_block(PageDescriptor **block_pointer, int source_order)
	{
        // TODO: Implement me!
	}

public:
	/**
	 * Allocates 2^order number of contiguous pages
	 * @param order The power of two, of the number of contiguous pages to allocate.
	 * @return Returns a pointer to the first page descriptor for the newly allocated page range, or NULL if
	 * allocation failed.
	 */
	PageDescriptor *allocate_pages(int order) override
	{
        // TODO: Implement me!
	}

    /**
	 * Frees 2^order contiguous pages.
	 * @param pgd A pointer to an array of page descriptors to be freed.
	 * @param order The power of two number of contiguous pages to free.
	 */
    void free_pages(PageDescriptor *pgd, int order) override
    {
        // TODO: Implement me!
    }

    /**
     * Marks a range of pages as available for allocation.
     * @param start A pointer to the first page descriptors to be made available.
     * @param count The number of page descriptors to make available.
     */
    virtual void insert_page_range(PageDescriptor *start, uint64_t count) override
    {
        // TODO: Implement me!
    }

    /**
     * Marks a range of pages as unavailable for allocation.
     * @param start A pointer to the first page descriptors to be made unavailable.
     * @param count The number of page descriptors to make unavailable.
     */
    virtual void remove_page_range(PageDescriptor *start, uint64_t count) override
    {
        // TODO: Implement me!
    }

	/**
	 * Initialises the allocation algorithm.
	 * @return Returns TRUE if the algorithm was successfully initialised, FALSE otherwise.
	 */
	bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) override
	{
        // TODO: Implement me!
	}

	/**
	 * Returns the friendly name of the allocation algorithm, for debugging and selection purposes.
	 */
	const char* name() const override { return "buddy"; }

	/**
	 * Dumps out the current state of the buddy system
	 */
	void dump_state() const override
	{
		// Print out a header, so we can find the output in the logs.
		mm_log.messagef(LogLevel::DEBUG, "BUDDY STATE:");

		// Iterate over each free area.
		for (unsigned int i = 0; i < ARRAY_SIZE(_free_areas); i++) {
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "[%d] ", i);

			// Iterate over each block in the free area.
			PageDescriptor *pg = _free_areas[i];
			while (pg) {
				// Append the PFN of the free block to the output buffer.
				snprintf(buffer, sizeof(buffer), "%s%lx ", buffer, sys.mm().pgalloc().pgd_to_pfn(pg));
				pg = pg->next_free;
			}

			mm_log.messagef(LogLevel::DEBUG, "%s", buffer);
		}
	}


private:
	PageDescriptor *_free_areas[MAX_ORDER+1];
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

/*
 * Allocation algorithm registration framework
 */
RegisterPageAllocator(BuddyPageAllocator);