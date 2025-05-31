#include <iostream>
#include <thread>
#include <atomic>
#include "queue.h"

using namespace std;

// �ʰ��� ���� �׽�Ʈ
// ����: �Ʒ� ����(Operation, Request)�� ������ ��
// ť�� Item�� void*�̹Ƿ� �󸶵��� �޶��� �� ����

#define REQUEST_PER_CLINET	10000

atomic<int> sum_key = 0;
atomic<int> sum_value = 0;
//atomic<double> response_time_tot = 0.0;

typedef enum {
	GET,
	SET,
	GETRANGE
} Operation;

typedef struct {
	Operation op;
	Item item;
} Request;

void client_func(Queue* queue, Request requests[], int n_request) {
	Reply reply = { false, 0 };

	// start_time = .....

	for (int i = 0; i < n_request; i++) {
		if (requests[i].op == GET) {
			reply = dequeue(queue);
		}
		else { // SET
			reply = enqueue(queue, requests[i].item);
		}

		if (reply.success) {
			// �ܼ��� ���Ϲ��� Ű ���� ����(�ƹ� �ǹ� ����)
			sum_key += reply.item.key;
			sum_value += (int)reply.item.value;// void*���� �ٽ� int�� ��ȯ

			// ���Ϲ��� key, value �� ����
			// ...����...
		}
		else {
			// noop
		}
	}

	// ��¥�� �ʿ��� �� �����ð��� �����ϴ� �ڵ�
	//
	// elapsed_time = finish_time - start_time;
	// finish_time = ....
	// average_response_time = elapsed_time / REQUEST_PER_CLIENT;
	// printf(...average_response_time of client1 = .....);
	// response_time_tot += finish_time - start_time;
}

int main(void) {
	srand((unsigned int)time(NULL));

	// ��ũ�ε� ����(GETRANGE�� �н�)
	Request requests[REQUEST_PER_CLINET];
	for (int i = 0; i < REQUEST_PER_CLINET / 2; i++) {
		requests[i].op = SET;
		requests[i].item.key = i;
		requests[i].item.value = (void*)(rand() % 1000000);
	}
	for (int i = REQUEST_PER_CLINET / 2; i < REQUEST_PER_CLINET; i++) {
		requests[i].op = GET;
	}

	Queue* queue = init();
	//if (queue == NULL) return 0;

	// �ϴ� �� �� ���ε�, �׷��� multi client��� �����ϱ�
	thread client = thread(client_func, queue, requests, REQUEST_PER_CLINET);
	client.join();

	release(queue);

	// �ǹ� ���� �۾�
	cout << "sum of returned keys = " << sum_key << endl;
	cout << "sum of returned values = " << sum_value << endl;

	// ��¥�� �ʿ��� �ڵ�
	// total_average_response_time = total_response_time / n_cleint;
	// printf("total average response time = ....
	return 0;
}