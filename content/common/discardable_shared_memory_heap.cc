// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/discardable_shared_memory_heap.h"

#include "base/memory/discardable_shared_memory.h"

namespace content {
namespace {

bool IsPowerOfTwo(size_t x) {
  return (x & (x - 1)) == 0;
}

bool IsInFreeList(DiscardableSharedMemoryHeap::Span* span) {
  return span->previous() || span->next();
}

}  // namespace

DiscardableSharedMemoryHeap::Span::Span(
    linked_ptr<base::DiscardableSharedMemory> shared_memory,
    size_t start,
    size_t length)
    : shared_memory_(shared_memory), start_(start), length_(length) {
}

DiscardableSharedMemoryHeap::Span::~Span() {
}

DiscardableSharedMemoryHeap::DiscardableSharedMemoryHeap(size_t block_size)
    : block_size_(block_size) {
  DCHECK_NE(block_size_, 0u);
  DCHECK(IsPowerOfTwo(block_size_));
}

DiscardableSharedMemoryHeap::~DiscardableSharedMemoryHeap() {
  while (!free_spans_.empty())
    RemoveFromFreeList(free_spans_.tail()->value());
}

scoped_ptr<DiscardableSharedMemoryHeap::Span> DiscardableSharedMemoryHeap::Grow(
    scoped_ptr<base::DiscardableSharedMemory> memory,
    size_t size) {
  linked_ptr<base::DiscardableSharedMemory> shared_memory(memory.release());

  // Memory must be aligned to block size.
  DCHECK_EQ(
      reinterpret_cast<size_t>(shared_memory->memory()) & (block_size_ - 1),
      0u);
  DCHECK_EQ(size & (block_size_ - 1), 0u);

  scoped_ptr<Span> span(
      new Span(shared_memory,
               reinterpret_cast<size_t>(shared_memory->memory()) / block_size_,
               size / block_size_));
  RegisterSpan(span.get());
  return span.Pass();
}

void DiscardableSharedMemoryHeap::MergeIntoFreeList(scoped_ptr<Span> span) {
  // Merge with previous span if possible.
  SpanMap::iterator prev_it = spans_.find(span->start_ - 1);
  if (prev_it != spans_.end() && IsInFreeList(prev_it->second)) {
    scoped_ptr<Span> prev = RemoveFromFreeList(prev_it->second);
    DCHECK_EQ(prev->start_ + prev->length_, span->start_);
    span->start_ -= prev->length_;
    span->length_ += prev->length_;
    spans_[span->start_] = span.get();
  }

  // Merge with next span if possible.
  SpanMap::iterator next_it = spans_.find(span->start_ + span->length_);
  if (next_it != spans_.end() && IsInFreeList(next_it->second)) {
    scoped_ptr<Span> next = RemoveFromFreeList(next_it->second);
    DCHECK_EQ(next->start_, span->start_ + span->length_);
    span->length_ += next->length_;
    spans_[span->start_ + span->length_ - 1] = span.get();
  }

  free_spans_.Append(span.release());
}

scoped_ptr<DiscardableSharedMemoryHeap::Span>
DiscardableSharedMemoryHeap::Split(Span* span, size_t blocks) {
  DCHECK(blocks);
  DCHECK_LT(blocks, span->length_);

  scoped_ptr<Span> leftover(new Span(
      span->shared_memory_, span->start_ + blocks, span->length_ - blocks));
  RegisterSpan(leftover.get());
  spans_[span->start_ + blocks - 1] = span;
  span->length_ = blocks;
  return leftover.Pass();
}

scoped_ptr<DiscardableSharedMemoryHeap::Span>
DiscardableSharedMemoryHeap::SearchFreeList(size_t blocks) {
  DCHECK(blocks);

  // Search through list to find best span.
  Span* best = nullptr;

  // This implements address-ordered best-fit.
  for (Span* span = free_spans_.head()->value(); span != free_spans_.end();
       span = span->next()->value()) {
    // Skip span if it's not large enough.
    if (span->length_ < blocks)
      continue;

    if (best) {
      // Skip span if |best| is a better fit.
      if (span->length_ > best->length_)
        continue;

      // Skip span if |best| starts at a lower address.
      if ((span->length_ == best->length_) && (span->start_ > best->start_))
        continue;
    }

    best = span;
  }

  return best ? Carve(best, blocks) : nullptr;
}

void DiscardableSharedMemoryHeap::DeleteSpan(scoped_ptr<Span> span) {
  DCHECK(!IsInFreeList(span.get()));
  spans_.erase(span->start_);
  if (span->length_ > 1)
    spans_.erase(span->start_ + span->length_ - 1);
}

scoped_ptr<DiscardableSharedMemoryHeap::Span>
DiscardableSharedMemoryHeap::RemoveFromFreeList(Span* span) {
  span->RemoveFromList();
  return make_scoped_ptr(span);
}

scoped_ptr<DiscardableSharedMemoryHeap::Span>
DiscardableSharedMemoryHeap::Carve(Span* span, size_t blocks) {
  scoped_ptr<Span> serving = RemoveFromFreeList(span);

  const int extra = serving->length_ - blocks;
  if (extra) {
    scoped_ptr<Span> leftover(
        new Span(serving->shared_memory_, serving->start_ + blocks, extra));
    RegisterSpan(leftover.get());

    // No need to coalesce as the previous span of |leftover| was just split
    // and the next span of |leftover| was not previously coalesced with
    // |span|.
    free_spans_.Append(leftover.release());

    serving->length_ = blocks;
    spans_[serving->start_ + blocks - 1] = serving.get();
  }

  return serving.Pass();
}

void DiscardableSharedMemoryHeap::RegisterSpan(Span* span) {
  spans_[span->start_] = span;
  if (span->length_ > 1)
    spans_[span->start_ + span->length_ - 1] = span;
}

}  // namespace content
