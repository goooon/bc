#ifndef GUARD_List_h__
#define GUARD_List_h__

template<class Any>
struct Node {
	Node<Any> * prior;
	Any data;
	Node<Any> * next;
};
template<class Any>
class LinkedList {
	Node<Any> * head;//头结点
	Node<Any> * tail;//尾结点
	int length;//链表长度
	Node<Any> * getNode(int index) {
		//获得index位置的结点数据
		if (index<0 || index>length) {
			return NULL;
		}
		else {
			Node<Any> *temp = head->next;
			for (int i = 0;i < index;i++, temp = temp->next);
			return temp;
		}
	};
public:
	LinkedList()
	{
		head = bc_new Node<Any>;
		tail = bc_new Node<Any>;
		head->data = NULL;
		head->next = tail;
		head->prior = NULL;
		tail->data = NULL;
		tail->prior = head;
		tail->next = NULL;
		length = 0;
	}
	~LinkedList()
	{
		for (int i = 0;i < length + 2;i++) {
			Node<Any> * temp = head;
			head = head->next;
			/*Any * data = temp->data;
			if (data != NULL) {
				bc_del data;
			}*/
			bc_del temp;
		}
	}
	//list
	bool add(Any& data)//添加数据
	{
		return addLast(data);
	}
	
	bool addFirst(Any& data)//向链表头部添加一条数据
	{
		Node<Any> * node = bc_new Node<Any>;
		if (node == NULL)return false;
		node->data = data;
		node->prior = head;;
		node->next = head->next;
		node->next->prior = node;
		head->next = node;
		length++;
		return true;
	}
	bool addLast(Any& data)//向链表尾部添加一条数据
	{
		Node<Any> * node = bc_new Node<Any>;
		if (node == NULL)return false;
		node->data = data;
		node->next = tail;
		node->prior = tail->prior;
		node->prior->next = node;
		tail->prior = node;
		length++;
		return true;
	}
	bool removeFirst(Any& a)//移除链表第一个元素
	{
		if (length == 0) {
			return false;
		}
		else {
			Node<Any> * temp = head->next;
			a = temp->data;
			head->next = temp->next;
			head->next->prior = head;
			bc_del temp;
			length--;
			return true;
		}
	}
	bool removeLast(Any& a)//移除链表最后一个元素
	{
		if (length == 0) {
			return false;
		}
		else {
			Node<Any> * temp = tail->prior;
			a = temp->data;
			tail->prior = temp->prior;
			tail->prior->next = tail;
			bc_del temp;
			length--;
			return true;
		}
	}
	int size()//获得链表的长度
	{
		return length;
	}
	//stack
	bool pop(Any& a)//出栈一条数据
	{
		return removeLast(a);
	}
	bool push(Any& data)//入栈一条数据
	{
		return addLast(data);
	}
	//queue
	bool in(Any& data)//进入队列
	{
		return addLast(data);
	}
	bool out(Any& data)//出队列
	{
		return removeFirst(data);
	}
};
#endif // GUARD_List_h__
