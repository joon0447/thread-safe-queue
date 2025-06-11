#include <iostream>
#include "queue.h"
#include <mutex>
#include <cstring>
#include <cstdlib>

std::mutex q_mutex;

Item item_deep_copy(const Item& src) {
	Item dst;
	dst.key = src.key;
	dst.value_size = src.value_size;
	dst.value = malloc(src.value_size);
	memcpy(dst.value, src.value, src.value_size);
	return dst;
}


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
		free(cur->item.value);
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
	Reply reply;
	reply.success = true;

	// 중복 key 덮어쓰기
	Node* cur = queue->head;
	while (cur) {
		if (cur->item.key == item.key) {
			free(cur->item.value);
			cur->item = item_deep_copy(item);
			reply.item = cur->item;
			return reply;
		}
		cur = cur->next;
	}

	// 새로운 노드 삽입
	Item copied_item = item_deep_copy(item);
	Node* new_node = new Node();
	new_node->item = copied_item;
	new_node->next = nullptr;

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
	reply.item = copied_item;
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
	queue->head = queue->head->next;
	if (!queue->head) queue->tail = nullptr;

	reply.item = temp->item;
	reply.success = true;

	delete temp;
	return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
	std::lock_guard<std::mutex> lock(q_mutex);
	Queue* new_queue = init();
	Node* cur = queue->head;

	while (cur) {
		if (cur->item.key >= start && cur->item.key <= end) {
			Node* new_node = new Node();
			new_node->item = item_deep_copy(cur->item);
			new_node->next = nullptr;

			if (!new_queue->head) {
				new_queue->head = new_queue->tail = new_node;
			}
			else {
				new_queue->tail->next = new_node;
				new_queue->tail = new_node;
			}
		}
		if (cur->item.key > end) break;
		cur = cur->next;
	}

	return new_queue;
}
