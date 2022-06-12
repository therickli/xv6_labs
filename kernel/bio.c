// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

#define NBUCKET 13
struct {
  struct spinlock lock;
  struct buf head;
} bufhash[NBUCKET];

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }

  for (int i=0;i<NBUCKET;i++) {
    initlock(&bufhash[i].lock, "bcacheblock");
    bufhash[i].head.prev = &bufhash[i].head;
    bufhash[i].head.next = &bufhash[i].head;
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bufhash[0].head.next;
    b->prev = &bufhash[0].head;
    initsleeplock(&b->lock, "buffer");
    bufhash[0].head.next->prev = b;
    bufhash[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // acquire(&bcache.lock);

  // Is the block already cached?
  // for(b = bcache.head.next; b != &bcache.head; b = b->next){
  //   if(b->dev == dev && b->blockno == blockno){
  //     b->refcnt++;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
  int index = blockno % NBUCKET, indexlru;
  //printf("1!!!\n");
  acquire(&bufhash[index].lock);
  for (b = bufhash[index].head.next; b != &bufhash[index].head; b=b->next) {
    if(b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
  //printf("release1!!!\n");
      release(&bufhash[index].lock);
  //printf("2!!!\n");
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bufhash[index].lock);

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
  //printf("3!!!\n");
  acquire(&bcache.lock);

  uint time=0xFFFFFFFF;
  struct buf *lru = 0;
  // for(b = bcache.buf+1; b < bcache.buf+NBUF; b++){
  //   if(b->refcnt == 0 && b->timestamp < time) {
  //     indexlru = lru->blockno % NBUCKET;
  //     if (indexlru != index) {
  //     time = b->timestamp;
  //     lru = b;
  //   }
  // }
  for (int i = 0; i < NBUCKET; i++) {
    acquire(&bufhash[i].lock);
    int update=0;
    for (b = bufhash[i].head.next; b != &bufhash[i].head; b = b->next) {
      if (b->refcnt == 0 && b->timestamp < time) {
        if (lru!=0) {
          indexlru = lru->blockno % NBUCKET;
  //printf("release2!!! i:%d\n", indexlru);
          if (i!=indexlru) {
            release(&bufhash[indexlru].lock);
          }
        }
        update = 1;
        time = b->timestamp;
        lru = b;
      }
    }
    if(update==0){
  //printf("release3!!! i:%d\n", i);
      release(&bufhash[i].lock);
    }
  }

  if (lru!=0) {
    indexlru = lru->blockno % NBUCKET;
    if (indexlru != index) {
  //printf("4!!!\n");
      lru->prev->next = lru->next;
      lru->next->prev = lru->prev;
  //printf("release4!!! i:%d\n", indexlru);
      release(&bufhash[indexlru].lock);
      acquire(&bufhash[index].lock);
      lru->next = bufhash[index].head.next;
      lru->prev = &bufhash[index].head;
      bufhash[index].head.next->prev = lru;
      bufhash[index].head.next = lru;
      lru->dev = dev;
      lru->blockno = blockno;
      lru->valid = 0;
      lru->refcnt = 1;
  //printf("release5!!!\n");
      release(&bufhash[index].lock);
    } else {
  //printf("release6!!! i:%d\n", indexlru);
      release(&bufhash[indexlru].lock);
      acquire(&bufhash[index].lock);
      lru->dev = dev;
      lru->blockno = blockno;
      lru->valid = 0;
      lru->refcnt = 1;
  //printf("release7!!!\n");
      release(&bufhash[index].lock);
    }
  //printf("release8!!!\n");
    release(&bcache.lock);
  //printf("5!!!\n");
    acquiresleep(&lru->lock);
    return lru;
  }

  //printf("lru=%p\n", lru);
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  b->timestamp = ticks;
  //printf("release9!!!\n");
  releasesleep(&b->lock);
  b->refcnt--;
  // acquire(&bcache.lock);
  // b->refcnt--;
  // if (b->refcnt == 0) {
  //   // no one is waiting for it.
  //   b->next->prev = b->prev;
  //   b->prev->next = b->next;
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }
  
  // release(&bcache.lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


