#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>  // ✅ 추가
#include "queue.h"

using namespace std;

#define REQUEST_PER_CLIENT 10000

atomic<int> sum_key = 0;
atomic<int> sum_value = 0;

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

	for (int i = 0; i < n_request; i++) {
		if (requests[i].op == GET) {
			reply = dequeue(queue);
		}
		else { // SET
			reply = enqueue(queue, requests[i].item);
		}

		if (reply.success) {
			sum_key += reply.item.key;
			sum_value += *((int*)reply.item.value);  // ✅ 캐스팅 수정
		}
	}
}

int main(void) {
	srand((unsigned int)time(NULL));

	// 워크로드 생성
	Request requests[REQUEST_PER_CLIENT];
	for (int i = 0; i < REQUEST_PER_CLIENT / 2; i++) {
		requests[i].op = SET;
		requests[i].item.key = i;

		int* val = new int(rand() % 1000000);
		requests[i].item.value = malloc(sizeof(int));
		memcpy(requests[i].item.value, val, sizeof(int));
		requests[i].item.value_size = sizeof(int);
		delete val;
	}
	for (int i = REQUEST_PER_CLIENT / 2; i < REQUEST_PER_CLIENT; i++) {
		requests[i].op = GET;
	}

	Queue* queue = init();

	// 시간 측정 시작
	auto start_time = chrono::high_resolution_clock::now();

	// 클라이언트 스레드 1개 (여러 개로 쉽게 확장 가능)
	thread client(client_func, queue, requests, REQUEST_PER_CLIENT);
	client.join();

	auto end_time = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = end_time - start_time;

	release(queue);

	cout << "sum of returned keys = " << sum_key << endl;
	cout << "sum of returned values = " << sum_value << endl;

	// ✅ 처리율 출력
	cout << "Total elapsed time: " << elapsed.count() << " sec" << endl;
	cout << "Throughput: " << (REQUEST_PER_CLIENT / elapsed.count()) << " ops/sec" << endl;

	return 0;
}