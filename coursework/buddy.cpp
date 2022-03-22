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

	/** Given a page descriptor, and an order, returns if the block is aligned with that order
	 * @param pgd The page descriptor to check alignment for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns TRUE if aligned for that order, false otherwise 
	 */
	bool is_aligned(const PageDescriptor *pgd, int order) {
		//perform a shift left by by that order
		uint64_t pages_per_block = this->pages_in_block(order); 

		//get the page frame number of this page/pdg -> right shift to see pgd no a multiple 2^order
		pfn_t pfn = sys.mm().pgalloc().pgd_to_pfn(pgd); 

		//check alignment of PGD, if it not correctly alligned, we can't find a buddy 
		return (pfn % pages_per_block == 0); 
	}

	/** Given an order, returns the number pages in block for that given order
	 * @param order The order in which the page descriptor lives.
	 * @return Returns the integer rep of number of blocks that order can hold
	 */
	uint64_t pages_in_block(int order) {
		//perform a shift left of 1 by by that order, so in block 0 its 1 pages per block, in block 1 is 2 and so on
		uint64_t pages = (1 << order); 
		return pages; 
	}

	/** Given a starting page descriptor, and an count of pages to free, returns the lowest order that can hold that many pages
	 * @param pgd The starting page descriptor 
	 * @param count The number of page descriptor do allocate/free
	 * @return Returns an integer representing the lowest order that can fit the count of pages needed 
	 */
	int lowest_possible_order(const PageDescriptor *pgd, uint64_t count) {
		//let's first find the highest aligned order to pgd and then count; 
		int highest_aligned_order = MAX_ORDER;

		while (!(is_aligned(pgd, highest_aligned_order))) {
			highest_aligned_order--;
		}

		//and now count to lowest order that can fit count of pages 
		uint64_t pages_can_allocate = this->pages_in_block(highest_aligned_order); 
		int lowest_possible = highest_aligned_order;

		while ((lowest_possible <= MAX_ORDER) && (pages_can_allocate < count)) {
			if (is_aligned(pgd, lowest_possible)) { 
				//mm_log.messagef(LogLevel::INFO,"Got lowest possible aligned order %d to fit 0x%p + 0x%lx", lowest_possible, pgd, count);
				return lowest_possible; 
			}
			lowest_possible++;
			pages_can_allocate = this->pages_in_block(highest_aligned_order);
		}
		
		//mm_log.messagef(LogLevel::INFO,"Got lowest possible order %d to fit 0x%p + 0x%lx", lowest_possible, pgd, count);
		return lowest_possible;
	}

	/** Given a page descriptor, and an order, returns the buddy PGD.  The buddy could either be
	 * to the left or the right of PGD, in the given order.
	 * @param pgd The page descriptor to find the buddy for.
	 * @param order The order in which the page descriptor lives.
	 * @return Returns the buddy of the given page descriptor, in the given order.
	 */
	PageDescriptor *buddy_of(const PageDescriptor *pgd, int order)
	{	
		////mm_log.messagef(LogLevel::INFO,"Find buddy pgd");

        //ensure that the order is within range, if we are top-most order we can't have a buddy - 256 bytes is largest possible block
		assert(order <= MAX_ORDER); 

		//perform a shift left by by that order
		uint64_t pages_per_block = this->pages_in_block(order); 

		//check alignment of PGD, if it not correctly alligned, we can't find a buddy 
		assert(this->is_aligned(pgd,order)); 
		 
		// calc the PFN of buddy - virtual memory address 
		// if PFN is aligned to the next order, then the buddy is the next block in this order (i.e. front of this page),
		// otherwise its in the previous block (so behind this page)
		
		uint64_t buddy_pfn;
		if (this->is_aligned(pgd,(order+1))) {
			////mm_log.messagef(LogLevel::DEBUG,"page is aligned to next order, so buddy is in next block");
			//add to virtual address of page 
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) + pages_per_block;
			////mm_log.messagef(LogLevel::DEBUG,"buddy pfn calc: %lx", buddy_pfn);
		} else {
			////mm_log.messagef(LogLevel::DEBUG,"page is aligned with order, so buddy is in previous block");
			//subtract from virtual address of page 
			buddy_pfn = sys.mm().pgalloc().pgd_to_pfn(pgd) - pages_per_block;
			////mm_log.messagef(LogLevel::DEBUG,"Buddy pfn calc: %lx", buddy_pfn);
		}
		
		////mm_log.messagef(LogLevel::DEBUG,"turning buddy pfn to pgd");
		PageDescriptor *buddy_pgd = sys.mm().pgalloc().pfn_to_pgd(buddy_pfn);
		////mm_log.messagef(LogLevel::INFO,"Returning buddy pgd");
		return buddy_pgd;
	}

	/** Given a page descriptor, and an order, returns if the buddy is in the
	 * @param start The page descriptor to check if in the free areas for the order
	 * @param order The order in which we are searching the page descriptor for
	 * @return Returns the if target is in a free-block is this order
	 */
	bool is_in_free(const PageDescriptor *target, int order) {
		//pointer to the free block we are searching
		PageDescriptor *free_block = this->_free_areas[order];
		uint64_t block_size = this->pages_in_block(order);
		
		while (free_block->next_free) {
			if ((free_block <= target) && (free_block + block_size >= target)) {
				//target is somewhere in the block we are looking at
				//mm_log.messagef(LogLevel::DEBUG,"target found somewhere in the block we are looking at");
				return true;
			};        
			////mm_log.messagef(LogLevel::DEBUG,"keep searching in this order: %d", order);
			free_block = free_block->next_free; 
		}
		return false;
	}

	/**
	 * Given a pointer to a block of memory to be inserted into an order this function will
	 * insert the block into a free list
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param next_order The order in which the insert the block to
	 * @return Returns the area of free-list that is inserted
	 */
	PageDescriptor **insert_block(PageDescriptor *block_pgd, int next_order) {
		//mm_log.messagef(LogLevel::INFO,"Buddy algo called to insert block %p in order %d", block_pgd, next_order);
		//the buddy allocator maintains a list of free areas for each order, get that array for this particular order we are inserting into
		PageDescriptor **area = &_free_areas[next_order];
		//mm_log.messagef(LogLevel::DEBUG,"starting address in free_list: %p", *area);

		//iterate till you find the pgd=free-area pointer and there is still free area left to search
		while ((block_pgd > *area) && *area) {
			//mm_log.messagef(LogLevel::DEBUG,"iterating through-freelist till we find the pgd=%p. Pointer of free-area, at: %p", block_pgd, *area);
			//we cannot dynamically allocate memory, we need to update the pointer to free-area we are looking at
			area = &(*area)->next_free;
		}

		//move the current pointer of free-area where we are gonna insert the incoming block to next block
		block_pgd->next_free = *area;
		*area = block_pgd;
		//mm_log.messagef(LogLevel::INFO,"Inserted block into free_area list");
		return area;
	}

	/**
	 * Given a pointer to a block of free_memory, remove the block from that area
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param next_order The order in which the remove the block from
	 * @return Returns the area of free-list that is removed
	 */
	void remove_block(PageDescriptor *block_pgd, int next_order) {
		////mm_log.messagef(LogLevel::INFO,"Buddy algo called to remove block");
		//the buddy allocator maintains a list of free areas for each order, get that array for this particular order we are inserting into
		PageDescriptor **area = &_free_areas[next_order];

		//iterate till you find the find block to remove in linked-list of _free_area
		while ((block_pgd != *area) && *area) {
			////mm_log.messagef(LogLevel::DEBUG,"iterating till we find the pgd=free-area pointer");
			//we cannot dynamically allocate memory, we need to update the pointer to free-area we are looking at
			area = &(*area)->next_free;
		}

		//make sure block exists
		assert(*area == block_pgd);

		//remove the current page residing in that part of free-list, by pointing to the block_pointer->next_free that NULL/empty, so inverse insert_block
		*area = block_pgd->next_free;
		block_pgd->next_free = NULL;
		////mm_log.messagef(LogLevel::INFO,"Removed block from free_area");
	}

	/**
	 * Given a pointer to a block of free memory in the order "source_order", this function will split the block in half, and insert it into the order below.
	 * @param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
	 * @param source_order The order in which the block of free memory exists.  Naturally,
	 * the split will insert the two new blocks into the order below.
	 * @return Returns the left-hand-side of the new block.
	 */
	PageDescriptor *split_block(PageDescriptor **block_pointer, int source_order)
	{	
		////mm_log.messagef(LogLevel::INFO,"Splitting blocks");
        //check alignment of PGD, if it not correctly alligned, we can't split the block from block pointer - so make sure we are sending in top of the block_pointer
		assert(this->is_aligned(*block_pointer, source_order)); 

		//now split block, as long as the source order is above 0 -> there is no order below zero!
		if (source_order == 0) return *block_pointer;
		
		//the two blocks, one will point to current block pointer address and next block will point to end of that block (in lower order)
		int lower_order = source_order - 1;
		uint64_t pages_lower_block = this->pages_in_block(lower_order);
		PageDescriptor *block_one = *block_pointer;
		PageDescriptor *block_two = this->buddy_of(block_one,lower_order);

		//now remove the block in that order starting at pgd of block one, and add the two new blocks in the order below 
		this->remove_block(block_one, source_order);
		this->insert_block(block_one, lower_order);
		this->insert_block(block_two, lower_order);

		//return the left-hand side of the new block, i.e. the start of the first block
		////mm_log.messagef(LogLevel::INFO,"Finished splitting blocks");
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
		////mm_log.messagef(LogLevel::INFO,"Merging blocks");
        //check alignment of PGD, if it not correctly alligned, we can't split the block
		assert(this->is_aligned(*block_pointer, source_order)); 

		//now merge blocks, as long as the source order is not in the top most order
		if (source_order == MAX_ORDER) return block_pointer;
		
		//the two blocks, one will point to current block pointer address and next block will point to end of that block (in lower order)
		int higher_order = source_order + 1;
		PageDescriptor *block_one = *block_pointer;
		
		//use buddy_of to find the buddy of original block
		PageDescriptor *block_two = this->buddy_of(*block_pointer,source_order);

		//remove the pages from source order
		this->remove_block(block_one,source_order);
		this->remove_block(block_two,source_order);

		//add merged blocks into next (higher) order
		if (block_two>block_one) {
			//merged block starts at block one
			return this->insert_block(block_one,higher_order);	
		}
		else {
			//merged block starts at block two
			return this->insert_block(block_two,higher_order);
		}
		////mm_log.messagef(LogLevel::INFO,"Finished merging blocks");
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
		//mm_log.messagef(LogLevel::INFO,"Called to allocate pages");
        int highest_order = order;

		//iterate over each order above inputed order till you find a highest order that is not allocated (non-empty in free list)
		//Remember: the caller does not care where in memory these pages are, just that the pages returned are
		//contiguous.
		while (highest_order <= MAX_ORDER && this->_free_areas[highest_order] == NULL) {
			highest_order++;
		}

		//allocation failed, we reached the top most order
		if (highest_order > MAX_ORDER) {
			//mm_log.messagef(LogLevel::DEBUG,"allocation failed, we counted to high - there must be no suitable blocks free left!");
			return NULL;
		}

		//we can allocated 2^order of contigous pages, point to that starting block
		PageDescriptor *block = _free_areas[highest_order];

		//iteratively split blocks till target order is reached (binary buddy system - https://www.geeksforgeeks.org/operating-system-allocating-kernel-memory-buddy-system-slab-system/)
		for (int i = highest_order; i > order; i--) {
			block = this->split_block(&block,i);
		}
		////mm_log.messagef(LogLevel::DEBUG,"Finished splitting blocks");

		//remove the block of contigous pages from free-memory of that order as its been allocated (memory management core will update status to ALLOCATED)
		this->remove_block(block, highest_order);
		//mm_log.messagef(LogLevel::INFO,"Removed the block of contigous pages from free-memory");
		//return the first pdg in this linked list of allocated blocks
		return block;
	}

    /**
	 * Frees 2^order contiguous pages.
	 * @param pgd A pointer to an array of page descriptors to be freed.
	 * @param order The power of two number of contiguous pages to free.
	 */
    void free_pages(PageDescriptor *pgd, int order) override
    {
		//mm_log.messagef(LogLevel::INFO,"Called to free pages");;
		//assert that block is aligned with order
		assert(this->is_aligned(pgd,order));

		//Free pages should be put back into the free lists, coalescing buddies back up to the
		//maximum order as per the buddy allocation algorithm.
		
		//point to first pfn - first pdg to be freed
		PageDescriptor **base = insert_block(pgd,order);
		//get pointer to first buddy that could be free, pdg/base should not be in free list 
		PageDescriptor *potential_buddy = this->_free_areas[order];
		//get buddy of base
		PageDescriptor *buddy_of_base = this->buddy_of(*base,order);

		//iterate over potential buddies, start from current order and move up till MAX_ORDER
		for (int curr_order = order; curr_order <= MAX_ORDER && potential_buddy; curr_order++){
			if (buddy_of_base != potential_buddy) {  
				//we can't find block's buddy at this pointer to free_area - area of free_list is not NULL
				////mm_log.messagef(LogLevel::DEBUG,"we can't find block's buddy at this pointer to free_area, so moving onto the next block in current order");
				//move onto the next block in current order
				potential_buddy = potential_buddy->next_free;
				//stay in same order
				curr_order --;
			}
			else { 
				//base's buddy is free, so we can merge to base
				base = this->merge_block(base,curr_order);
				//go to next order
				potential_buddy = this->_free_areas[curr_order];
				//new buddy of new base
				buddy_of_base = this->buddy_of(*base,curr_order);
				////mm_log.messagef(LogLevel::DEBUG,"base's buddy is free, so we can merge to base");
			}
		}
		//mm_log.messagef(LogLevel::INFO,"Finished freeing pages");
    }

    /**
     * Marks a range of pages as available for allocation -> put it back in _free_areas
     * @param start A pointer to the first page descriptors to be made available.
     * @param count The number of page descriptors to make available.
     */
    virtual void insert_page_range(PageDescriptor *start, uint64_t count) override
    {
        //replicate the logic of free_pages, but now you know start and how many pages to make available, not order of block if start is aligned with order
		//mm_log.messagef(LogLevel::INFO, "Called to insert page range, with start pdg=%p and count=%lx", start, count);
		
		//get lowest order that could allocate count
		int lowest_order = lowest_possible_order(start, count); 
		uint64_t remaining_pgs = count;
		PageDescriptor **base = insert_block(start, lowest_order);
 
		for (int curr_order = lowest_order; curr_order <= MAX_ORDER && remaining_pgs>0; curr_order++) {
			PageDescriptor *buddy_of_base = this->buddy_of(*base,lowest_order);
			//mm_log.messagef(LogLevel::DEBUG, "We start with base at=%p in free list and try to merge with base %p", base, buddy_of_base);
			//we check to see if block's buddy is free
			if (is_in_free(buddy_of_base, curr_order)) {
				//base's buddy is free, so we can merge to base
				base = this->merge_block(base,curr_order);
				//decrease amount of pages left to be inserted
				remaining_pgs = remaining_pgs - pages_in_block(curr_order);
				//mm_log.messagef(LogLevel::DEBUG,"base's buddy is free, so we can merge to base");
			} else {
				//mm_log.messagef(LogLevel::DEBUG,"base's buddy is not free, so no more merging");
				break; 
			}
		}
		//mm_log.messagef(LogLevel::INFO,"Finished inserting page range");

    }

    /**
     * Marks a range of pages as unavailable for allocation - take out of free_areas_
     * @param start A pointer to the first page descriptors to be made unavailable.
     * @param count The number of page descriptors to make unavailable.
     */
    virtual void remove_page_range(PageDescriptor *start, uint64_t count) override
    {	
		//mm_log.messagef(LogLevel::INFO,"Called to remove page range, with start pdg=%p and count=%lx", start, count);
		
		//get lowest order that could allocate count - to compensate not knowing order of pages to be allocated
		int lowest_order = lowest_possible_order(start, count); 
		int highest_order = lowest_order; 
		//now count up to the highest order where the start page is available 
		while (highest_order < MAX_ORDER && !is_in_free(start, highest_order)) {
			highest_order++;
		}

		//we can allocate 2^order of contigous pages, point to that starting block in the free area
		uint64_t can_allocate_max = pages_in_block(highest_order); 

		//iterate over potential blocks, start from current higheat order and split down until blocksize goes below count  
		for (int curr_order = highest_order; curr_order >= 0 && can_allocate_max >= count; curr_order--) {
			//check alignment of start, if not aligned to start of its block we need to point to its 
			start = split_block(&start, curr_order);
			can_allocate_max = pages_in_block(curr_order);
			//mm_log.messagef(LogLevel::DEBUG,"split the block starting at pdg: pdg=%p to order %d and we can remove pages=%lx", start, curr_order, pages_in_block(curr_order));
			
			//time to exit loop and remove block if next block in lower order can't allocate 
			if (can_allocate_max <= count) {
				//remove the block of contigous pages from free-memory of that order as its been allocated (memory management core will update status to ALLOCATED)
				remove_block(start, curr_order);
				break; 
			}
		}
		//mm_log.messagef(LogLevel::INFO,"Finished removing page range");
		
    }

	/**
	 * Initialises the allocation algorithm.
	 * @return Returns TRUE if the algorithm was successfully initialised, FALSE otherwise.
	 */
	bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) override
	{
        //mm_log.messagef(LogLevel::INFO, "Buddy Allocator Initialising pgd=%p, nr_pgds=0x%lx", page_descriptors, nr_page_descriptors);
		
		//the nth pgd we have initialised
		uint64_t remaining_pgs = nr_page_descriptors;
		PageDescriptor *pgds = page_descriptors;

		for (int curr_order = MAX_ORDER; curr_order >= 0 && remaining_pgs >= 0; curr_order--) {
			int block_counter = 0;
			//check if the blocksize for current order can hold atleast one block of remaining pgs to initialise:
			uint64_t curr_blocksize = pages_in_block(curr_order);

			if (remaining_pgs >= curr_blocksize) {
				//_free_areas[curr_order] = pgds;

				while (remaining_pgs >= curr_blocksize) {
					//mm_log.messagef(LogLevel::INFO, "We have can fit some remaining pgds: 0x%lx in order %d", remaining_pgs, curr_order);
					block_counter++;
					//move to next block of this order in free list and in pgds 
					pgds->next_free = pgds + curr_blocksize;
					pgds = pgds->next_free;
					//decrease amount of pages left to be initialised by one block at current order
					remaining_pgs = remaining_pgs - curr_blocksize;
				}
			}
			//mm_log.messagef(LogLevel::INFO, "Finished initialising blocks %d for order %d and now have remaining pgds: 0x%lx", block_counter, curr_order, remaining_pgs);
			//end 
			pgds->next_free = NULL;
		}
		
		if (remaining_pgs > 0) {
			////mm_log.messagef(LogLevel::INFO, "We didn't handle 0x%lx pages ", remaining_pgs);
			return false;
		}
		else {
			//mm_log.messagef(LogLevel::INFO, "Succesfully finished allocator->init");
			return true;
		}
		
	}

	/**
	 * Returns the friendly name of the allocation algorithm, for INFOging and selection purposes.
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