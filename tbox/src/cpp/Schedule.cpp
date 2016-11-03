#include "../inc/Schedule.h"
#include "../inc/Application.h"
//Schedule& Schedule::getInstance()
//{
//	return Application::getInstance().getSchedule();
//}

Schedule::Schedule()
{
	nodes.next = &nodes;
	nodes.prev = &nodes;
	nodes.fireTime = 0x7fffffffffffffff;
	nodes.task = NULL;
}

void Schedule::triger(Timestamp current)
{
	Node* node = nodes.next;
	//LOG_V("Schedule::triger(%lld)", current.getValue());
	while (node != &nodes)
	{
		if (node->fireTime >= current) {
			if (PostEvent(AppEvent::InsertTask, 0, 0, node->task)) {
				Node* flag = node->next;
				node->prev->next = node->next;
				node->next->prev = node->prev;
				bc_del node;
				node = flag;
			}
			else {
				//keep it in the queue because app queue is full,and to finish it next period
				LOG_W("Schedule::triger() not complished because queue of app event is full");
				break;
			}
		}
		else {
			break;
		}
	}
}

void Schedule::insert(Timestamp fireTime, Task* task)
{
	Node* n = bc_new Node();
	n->fireTime = fireTime;
	n->task = task;

	Node* node = nodes.next;
	while (node != &nodes)
	{
		if (node->fireTime < fireTime)node = node->next;
		else break;;
	}
	n->next = node;
	n->prev = node->prev;
	node->prev->next = n;
	node->prev = n;
}

void Schedule::remove(u32 appId)
{
	Node* node = nodes.next;
	while (node != &nodes) {
		if (node->task->getApplicationId() == appId) {
			node->prev->next = node->next;
			node->next->prev = node->prev;
			Node* next = node->next;
			bc_del node->task;
			bc_del node;
			node = next;
		}
		else {
			node = node->next;
		}
	}
}

bool Schedule::remove(Task* task)
{
	Node* node = nodes.next;
	while (node != &nodes) {
		if (node->task == task) {
			node->prev->next = node->next;
			node->next->prev = node->prev;
			bc_del task;
			bc_del node;
			return true;
		}
		node = node->next;
	}
	return false;
}

void Schedule::update(Timestamp fireTime, u32 appid)
{
	Node* flag = nodes.next;
	Node* node = flag;
	while (node != &nodes) {
		if (node->task->getApplicationId() == appid) {
			node->fireTime = fireTime;
			flag = node->next;
			Node* n;
			if (fireTime < node->prev->fireTime) {
				do
				{
					n = node->prev;
					if (n == &nodes) {
						node->prev->next = node->next;
						node->next->prev = node->prev;

						node->next = n->next;
						node->prev = n;
						n->next->prev = node;
						n->next = n;
						n = n->prev;
						break;
					} 
				} while (fireTime < n->fireTime);
				node->prev->next = node->next;
				node->next->prev = node->prev;

				node->next = n->next;
				node->prev = n;
				n->next->prev = node;
				n->next = n;
				n = n->prev;
			}
			else if (fireTime > node->next->fireTime) {
				do
				{
					n = node->next;
					if (n == &nodes) {
						node->prev->next = node->next;
						node->next->prev = node->prev;

						node->next = n->next;
						node->prev = n;
						n->next->prev = node;
						n->next = n;
						n = n->prev;
						break;
					}
				} while (fireTime > n->fireTime);
				node->prev->next = node->next;
				node->next->prev = node->prev;

				node->next = n->next;
				node->prev = n;
				n->next->prev = node;
				n->next = n;
				n = n->prev;
			}
			else {

			}
			node = flag;
		}
		else {
			node = node->next;
		}
	}
}
