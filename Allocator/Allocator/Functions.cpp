#include "Functions.h"
#include <vector>
#include <iostream>

using namespace std;

vector<Block> Memory;
Settings settings;

void previous_load()
{
	Block block;

	block.addr = 0;
	block.size = settings.memory_size;
	block.size_prev = 0;
	block.state = FREE;

	Memory.push_back(block);
}

size_t memory_align(size_t size)
{
	return size % settings.min_size == 0 ? size : size + settings.min_size - size % settings.min_size;
}

vector<Block>::iterator get_at_pos(int i)
{
	vector<Block>::iterator it = Memory.begin();
	advance(it, i);
	
	return it;
}

bool is_free_space(size_t _size)
{
	size_t n = 0;

	for (auto b : Memory)
		if (b.size > _size) return true;

	return false;
}

void * mem_alloc(size_t _size)
{
	if (!_size) return NULL;

	size_t size = memory_align(_size + 1);	// + 1 byte for header info

	int i = 0;

	// Finding free block
	for (auto b : Memory)
	{
		if (b.size >= size && b.state == FREE)
		{
			// Free block found. Creating two blocks
			Block block1, block2;

			block1.addr = b.addr;
			block1.size = size;
			block1.state = BUSY;
			block1.size_prev = b.size_prev;
			block1.occupied = _size + 1;

			block2.addr = block1.addr + block1.size;
			block2.size = b.size - block1.size;
			block2.size_prev = block1.size_prev;
			block2.state = FREE;
			block2.occupied = 0;

			// Add new blocks, exclude old block
			Memory.insert(get_at_pos(i + 1), block1);
			if (block2.size) Memory.insert(get_at_pos(i + 2), block2);
			Memory.erase(get_at_pos(i));

			return (int *)block1.addr;
		}
		i++;
	}
	
	return NULL;
}

void * mem_realloc(void * addr, size_t _size)
{
	size_t size = memory_align(_size);

	// Find block with current address
	unsigned int i = 0;
	for (auto & b : Memory)
	{
		if (b.addr == (int)addr)
		{
			if (size < b.size)	// Decrease block size
			{
				size_t bytes_replace = b.size - size;
				
				if (is_free_space(bytes_replace))
				{
					if (size == 0) {// Destroy old block
						b.state = FREE;
						return mem_alloc(bytes_replace);

					}
					else           // Just reduce block size
					{
						// Create new free block and reduce old block
						Block block;
						
						block.addr = b.addr + size;
						block.occupied = b.occupied - size;
						block.size = b.size - size;
						block.state = BUSY;
						block.size_prev = size;

						b.occupied = b.size = size;

						Memory.insert(get_at_pos(i + 1), block);

						return (int *)block.addr;
					}

				}
			}
			else if (size > b.size)
			{
				// Try to extend block to neighbour
				size_t free_memory = 0, bytes_to_extend = size - b.size;

				free_memory += (i && (*get_at_pos(i - 1)).state == FREE) ? (*get_at_pos(i - 1)).size : 0;
				free_memory += (i + 1 < Memory.size() && (*get_at_pos(i + 1)).state == FREE ? (*get_at_pos(i + 1)).size : 0);

				if (free_memory >= bytes_to_extend)
				{
					// use neighbour's memory
					if (i && (*get_at_pos(i - 1)).state == FREE)
						if ((*get_at_pos(i - 1)).size <= bytes_to_extend)
						{
							b.size += (*get_at_pos(i - 1)).size;
							b.addr = (*get_at_pos(i - 1)).addr;
							bytes_to_extend -= (*get_at_pos(i - 1)).size;
							(*get_at_pos(i - 1)).size = 0;
						}
						else
						{
							(*get_at_pos(i - 1)).size -= bytes_to_extend;
							b.size += bytes_to_extend;
							b.addr -= bytes_to_extend;
							bytes_to_extend = 0;
						}
					if (bytes_to_extend)
						if ((*get_at_pos(i + 1)).size <= bytes_to_extend)
						{
							b.size += (*get_at_pos(i + 1)).size;
							(*get_at_pos(i + 1)).size -= bytes_to_extend;
						}
						else
						{
							(*get_at_pos(i + 1)).size -= bytes_to_extend;
							(*get_at_pos(i + 1)).addr += bytes_to_extend;
							b.size += bytes_to_extend;
						}

					if (!(*get_at_pos(i - 1)).size) Memory.erase(get_at_pos(i - 1));
					if (!(*get_at_pos(i)).size) Memory.erase(get_at_pos(i));

					return (int *)(*get_at_pos(i - 1)).addr;
				}
				else
				{
					// Neighbours can't help, create new block in memory
					b.state = FREE;
					b.occupied = 0;
					return mem_alloc(size);
				}
			}
		}
		i++;
	}

	return NULL;
}

void mem_free(void * addr)
{
	unsigned int i = 0;

	for (auto & b : Memory)
	{
		if (b.addr == (int)addr)
		{
			b.state = FREE;

			// If there is free neighbour besides
			if (i && (*get_at_pos(i - 1)).state == FREE)
			{
				(*get_at_pos(i - 1)).size += b.size;
				Memory.erase(get_at_pos(i));
			}
			
			if (i + 1 < Memory.size() && (*get_at_pos(i + 1)).state == FREE)
			{
				b.size += (*get_at_pos(i + 1)).size;
				Memory.erase(get_at_pos(i + 1));
			}

			return;
		}
		i++;
	}
}

void defragment(int i)
{
	vector<Block>::iterator & it1 = get_at_pos(i), it2 = get_at_pos(i + 1);

	if (it1 == Memory.end() || it2 == Memory.end()) return;

	if ((*it1).state == FREE && (*it2).state == FREE) 
	{
		// Glue both free blocks
		(*it1).size = (*it1).size + (*it2).size;

		Memory.erase(it2);
		return defragment(i);
	}
	else if ((*it1).state == FREE && (*it2).state == BUSY)
	{
		// Swap free and busy blocks
		Block block;

		block.addr = (*it1).addr + (*it2).size;
		block.occupied = (*it1).addr;
		block.size = (*it1).size;
		block.size_prev = (*it1).size_prev;
		block.state = (*it1).state;

		(*it2).addr -= block.size;

		Memory.erase(it1);
		Memory.insert(get_at_pos(i + 1), block);

		return defragment(i);
	}

	return defragment(i + 1);
}

void mem_defrag()
{
	defragment(0);	// Recursive
}

void mem_dump()
{
	cout << endl << "Memory map:" << endl;
	for (auto b : Memory)
		printf("%6d%4s%6d\t%30s\n", b.addr, "..", b.size-1 + b.addr, (b.state == FREE ? "FREE" : "BUSY"));
		//printf("%6d%4s%6d\t%8d%24s\n", b.addr, "..", b.size - 1 + b.addr, b.occupied, (b.state == FREE ? "FREE" : "BUSY"));
}