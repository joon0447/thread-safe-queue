#include <iostream>
#include "queue.h"
#include <mutex>

std::mutex q_mutex;

Queue* init(void) {
	Queue* q = new Queue();
	q->head = nullptr;
	q->tail = nullptr;
	return q;
}


void release(Queue* queue) {
	std::lock_guard<std::mutex> lock(q_mutex);
	Node* cur = queue->head;
	while (cur) {
		Node* next = cur->next;
		delete cur;
		cur = next;
	}
	
	delete queue;
}


Node* nalloc(Item item) {
	// Node 생성, item으로 초기화
	Node* node = new Node();
	node->item = item;
	node->next = nullptr;
	return node;
}


void nfree(Node* node) {
	delete node;
}


Node* nclone(Node* node) {
	if (!node) return nullptr;
	return nalloc(node->item);
}


Reply enqueue(Queue* queue, Item item) {
	std::lock_guard<std::mutex> lock(q_mutex);

	Node* new_node = nalloc(item);
	Reply reply;
	reply.success = true;

	if (!queue->head) {
		// 빈 큐
		queue->head = queue->tail = new_node;
	}
	else {
		Node* cur = queue->head;
		Node* prev = nullptr;

		while (cur && cur->item.key <= item.key) {
			prev = cur;
			cur = cur->next;
		}

		if (!prev) {
			// 맨 앞에 삽입
			new_node->next = queue->head;
			queue->head = new_node;
		}
		else {
			prev->next = new_node;
			new_node->next = cur;
			if (!cur) {
				queue->tail = new_node;
			}
		}
	}

	return reply;
}

Reply dequeue(Queue* queue) {
	std::lock_guard<std::mutex> lock(q_mutex);
	Reply reply;
	if (!queue->head) {
		reply.success = false;
		return reply;
	}

	Node* temp = queue->head;
	reply.item = temp->item;
	reply.success = true;

	queue->head = queue->head->next;
	if (!queue->head) queue->tail = nullptr;

	nfree(temp);
	return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
	std::lock_guard<std::mutex> lock(q_mutex);

	Queue* new_queue = init();
	Node* cur = queue->head;
	Node* last = nullptr;

	while (cur) {
		if (cur->item.key >= start && cur->item.key <= end) {
			Node* copied = nclone(cur);
			if (!new_queue->head) {
				new_queue->head = new_queue->tail = copied;
			}
			else {
				new_queue->tail->next = copied;
				new_queue->tail = copied;
			}
		}
		if (cur->item.key > end) break;
		cur = cur->next;
	}

	return new_queue;
}
