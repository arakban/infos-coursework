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

	/** Given an order, returns the number pages in block for that given order
	 * @param order The order in which the page descriptor lives.
	 * @return Returns the integer rep of number of blocks that order can hold
	 */
	static inline constexpr uint64_t pages_in_block(int order) {
		//perform a shift left of 1 by by that order, so in block 0 its 1 pages per block, in block 1 is 2 and so on
		uint64_t pages = (1 << order); 
		return pages; 
	}

	/** Given a page descriptor, and an order, returns if the page is correctly aligned with that order  
	 * @param pgd The page descriptor to find the buddy for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns TRUE if aligned for that order, false otherwise 
	 */
	static inline constexpr uint64_t is_aligned(const PageDescriptor *pgd, int order) {
		//perform a shift left by by that order
		uint64_t pages_per_block = pages_in_block(order); 

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
		uint64_t pages_per_block = pages_in_block(order); 

		//check alignment of PGD, if it not correctly alligned, we can't find a buddy 
		assert(!is_aligned(pgd,order)); 
		 
		// calc the PFN of buddy - virtual memory address 
		// if PFN is aligned to the next order, then the buddy is the next block in this order (i.e. front of this page),
		// otherwise its in the previous block (so behind this page)
		
		uint64_t buddy_pfn;
		if (is_aligned(pgd,(order+1))) {
			syslog.messagef(LogLevel::DEBUG,"Page is aligned to next order, so buddy is in next block");
			//add to virtual address of page 
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) + pages_per_block;
			syslog.messagef(LogLevel::DEBUG,"Buddy pfn calc: %s", buddy_pfn);
		} else {
			syslog.messagef(LogLevel::DEBUG,"Page is aligned with order, so buddy is in previous block");
			//subtract from virtual address of page 
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) - pages_per_block;
			syslog.messagef(LogLevel::DEBUG,"Buddy pfn calc: %s", buddy_pfn);
		}

		PageDescriptor *buddy_pgd = sys.mm().pgalloc().pfn_to_pgd(buddy_pfn);
		return buddy_pgd;
	}

		/**
	 * Given a pointer to a block of memory to be inserted into an order this function will
	 * insert the block into a free list
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param next_order The order in which the insert the block to
	 */
	void insert_block(PageDescriptor *block_pgd, int next_order) {
		syslog.messagef(LogLevel::DEBUG,"buddy algo called to insert block");
		//the buddy allocator maintains a list of free areas for each order, get that array for this particular order we are inserting into
		PageDescriptor **area = &_free_areas[next_order];

		//iterate till you find the pgd=free-area pointer and there is still free area left to search
		while ((block_pgd > *area) && *area) {
			//we cannot dynamically allocate memory, we need to update the pointer to free-area we are looking at
			area = &(*area)->next_free;
		}

		//move the current pointer of free-area where we are gonna insert the incoming block to next block
		block_pgd->next_free = *area;
		*area =block_pgd;
		syslog.messagef(LogLevel::DEBUG,"inserted block");
	}

	/**
	 * Given a pointer to a block of memory to be removed from an order this function will
	 * remove the block from that area
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param next_order The order in which the remove the block from
	 */
	void remove_block(PageDescriptor *block_pgd, int next_order) {
		syslog.messagef(LogLevel::DEBUG,"buddy algo called to remove block");
		//the buddy allocator maintains a list of free areas for each order, get that array for this particular order we are inserting into
		PageDescriptor **area = &_free_areas[next_order];

		//iterate till you find the pgd=free-area pointer and there is still free area left to search
		while ((block_pgd > *area) && *area) {
			//we cannot dynamically allocate memory, we need to update the pointer to free-area we are looking at
			area = &(*area)->next_free;
			//make sure block exists
			assert(*area == block_pgd);
		}

		//remove the current page residing in that part of free-list, by pointing to the block_pointer->next_free that NULL/empty, so inverse insert_block
		*area = block_pgd->next_free;
		block_pgd->next_free = NULL;
		syslog.messagef(LogLevel::DEBUG,"removed block");
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
        //check alignment of PGD, if it not correctly alligned, we can't split the block
		assert(is_aligned(*block_pointer, source_order)); 

		//now split block, as long as the source order is above 0 -> there is no order belwow zero!
		if (source_order == 0) return *block_pointer;
		
		//the two blocks, one will point to current block pointer address and next block will point to end of that block (in lower order)
		int lower_order = source_order - 1;
		PageDescriptor *block_one = *block_pointer;
		uint64_t pages_lower_block = pages_in_block(lower_order); 
		PageDescriptor *block_two = block_one + pages_lower_block;

		//now remove the block in that order starting at pgd of block one, and add the two new blocks in the order below 
		remove_block(block_one,source_order);
		insert_block(block_one,lower_order);
		insert_block(block_two,lower_order);

		//return the left-hand side of the new block, i.e. the start of the first block
		return block_one;
	}

	/**
	 * Takes a block in the given source order, and merges it (and its buddy) into the next order.
	 * @param block_pointer A pointer to a pointer containing a block in the pair to merge.
	 * @param source_order The order in which the pair of blocks live.
	 * @return Returns the new slot that points to the merged block.
	 */
	PageDescriptor **merge_block(PageDescriptor **block_pointer, int source_order)
	{
        //check alignment of PGD, if it not correctly alligned, we can't split the block
		assert(is_aligned(*block_pointer, source_order)); 

		//now merge blocks, as long as the source order is not in the top most otder
		if (source_order == (MAX_ORDER - 1)) return block_pointer;
		
		//the two blocks, one will point to current block pointer address and next block will point to end of that block (in lower order)
		int higher_order = source_order + 1;
		PageDescriptor *block_one = *block_pointer;
		uint64_t pages_lower_block = pages_in_block(higher_order); 
		//use buddy_of to find the buddy of original block
		PageDescriptor *block_two = buddy_of(*block_pointer,source_order);

		//remove the pages from source order
		remove_block(block_one,source_order);
		remove_block(block_two,source_order);

		//add merged blocks into next (higher) order
		if (block_two>block_one) {
			//merged block starts at block one
			insert_block(block_one,higher_order);	
		}
		else {
			//merged block starts at block two
			insert_block(block_two,higher_order);
		}
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