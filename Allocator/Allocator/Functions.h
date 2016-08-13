
struct Block
{
	size_t size, size_prev,	addr, occupied;
	bool state;
};

struct Settings
{
	size_t memory_size = 512; // 1024 * 1024;
	size_t min_size = 4;
};

enum State { FREE, BUSY };

void * mem_alloc(size_t size);
void * mem_realloc(void * addr, size_t size);
void mem_free(void * addr);
void mem_dump();
void mem_defrag();

void previous_load();