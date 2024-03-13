
#include "array_request_manager.h"
// #include "hash_request_manager.h"

#include "atomic_queue.h"
// #include "atomic_queue_rigtorp.h"
#include "locked_queue.h"

#include "offload.h"

#define QUEUE_SIZE   1024
#define REQUEST_POOL_SIZE 1024

// using OE = mmcso::OffloadEngine<mmcso::AtomicQueueRigtorp<QUEUE_SIZE>, mmcso::ArrayRequestManager<REQUEST_POOL_SIZE>>;
using OE = mmcso::OffloadEngine<mmcso::AtomicQueue<QUEUE_SIZE>, mmcso::ArrayRequestManager<REQUEST_POOL_SIZE>>;
// using OE = mmcso::OffloadEngine<mmcso::LockedQueue, mmcso::ArrayRequestManager<REQUEST_POOL_SIZE>>;

extern OE &oe;