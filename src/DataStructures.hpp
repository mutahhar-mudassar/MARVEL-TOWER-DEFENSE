#pragma once

/**
 * ListNode - Node structure for doubly-linked list
 * 
 * This template structure represents a single node in a doubly-linked list.
 * Each node contains data and pointers to both the next and previous nodes,
 * allowing bidirectional traversal of the list.
 * 
 * Template Parameter:
 *   - T: Type of data stored in the node
 * 
 * Members:
 *   - data: The actual data value stored in this node
 *   - next: Pointer to the next node in the list (nullptr if last node)
 *   - prev: Pointer to the previous node in the list (nullptr if first node)
 * 
 * Usage:
 *   Used internally by LinkedList class. Not typically used directly by game code.
 */
template<typename T>
struct ListNode {
    T data;
    ListNode* next;
    ListNode* prev;
    ListNode(const T& value) : data(value), next(nullptr), prev(nullptr) {}
};

/**
 * LinkedList - Doubly-linked list data structure
 * 
 * This template class provides a doubly-linked list implementation for storing
 * collections of game entities (enemies, structures, projectiles). It supports
 * efficient insertion at the end and removal of arbitrary nodes.
 * 
 * Template Parameter:
 *   - T: Type of elements stored in the list (typically Enemy*, Structure*, or Projectile*)
 * 
 * Key Features:
 *   - Doubly-linked: Each node has pointers to both next and previous nodes
 *   - O(1) insertion: Adding elements at the end is constant time
 *   - O(1) removal: Removing a node by pointer is constant time
 *   - Memory management: Automatically deallocates nodes in destructor
 *   - Size tracking: Maintains count of elements in the list
 * 
 * Usage in Game:
 *   - LinkedList<Enemy*>: Stores all active enemies
 *   - LinkedList<Structure*>: Stores all placed structures
 *   - LinkedList<Projectile*>: Stores all active projectiles
 * 
 * Members:
 *   - head: Pointer to first node in the list (nullptr if empty)
 *   - tail: Pointer to last node in the list (nullptr if empty)
 *   - size: Current number of elements in the list
 * 
 * Operations:
 *   - pushBack(): Add element to end of list
 *   - remove(): Remove specific node from list
 *   - clear(): Remove all elements from list
 *   - getHead(): Get pointer to first node (for iteration)
 *   - getSize(): Get number of elements
 *   - isEmpty(): Check if list is empty
 * 
 * Iteration Example:
 *   ListNode<Enemy*>* node = enemies.getHead();
 *   while (node) {
 *       Enemy* enemy = node->data;
 *       // Process enemy...
 *       node = node->next;
 *   }
 */
template<typename T>
class LinkedList {
private:
    ListNode<T>* head;
    ListNode<T>* tail;
    int size;
public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}
    ~LinkedList() { clear(); }
    
    void pushBack(const T& value) {
        ListNode<T>* newNode = new ListNode<T>(value);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }
    
    void remove(ListNode<T>* node) {
        if (node == nullptr) return;
        if (node->prev) node->prev->next = node->next;
        else head = node->next;

        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
        delete node;
        size--;
    }
    
    void clear() {
        while (head) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        size = 0;
    }
    
    ListNode<T>* getHead() { return head; }
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

/**
 * Queue - First-In-First-Out (FIFO) queue data structure
 * 
 * This template class provides a queue implementation using a linked list.
 * Elements are added to the rear and removed from the front, maintaining
 * FIFO order. Used for enemy spawning queue.
 * 
 * Template Parameter:
 *   - T: Type of elements stored in the queue (typically Enemy*)
 * 
 * Key Features:
 *   - FIFO ordering: First element added is first removed
 *   - O(1) enqueue: Adding elements is constant time
 *   - O(1) dequeue: Removing elements is constant time
 *   - Memory management: Automatically deallocates nodes in destructor
 *   - Size tracking: Maintains count of elements in the queue
 * 
 * Usage in Game:
 *   - Queue<Enemy*>: Stores enemies waiting to spawn in current wave
 *   - Enemies are enqueued at wave start, dequeued during wave active state
 *   - Ensures enemies spawn in correct order with proper timing
 * 
 * Internal Structure:
 *   - QueueNode: Internal node structure for linked list implementation
 *   - front: Pointer to first node (next to be dequeued)
 *   - rear: Pointer to last node (where new elements are added)
 *   - size: Current number of elements in the queue
 * 
 * Operations:
 *   - enqueue(): Add element to rear of queue
 *   - dequeue(): Remove and return element from front of queue
 *   - getSize(): Get number of elements
 *   - isEmpty(): Check if queue is empty
 * 
 * Example:
 *   Queue<Enemy*> spawnQueue;
 *   spawnQueue.enqueue(enemy1);
 *   spawnQueue.enqueue(enemy2);
 *   Enemy* next;
 *   if (spawnQueue.dequeue(next)) {
 *       // Process next enemy...
 *   }
 */
template<typename T>
class Queue {
private:
    struct QueueNode {
        T data;
        QueueNode* next;
        QueueNode(const T& value) : data(value), next(nullptr) {}
    };
    QueueNode* front;
    QueueNode* rear;
    int size;
public:
    Queue() : front(nullptr), rear(nullptr), size(0) {}
    ~Queue() {
        while (front) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    void enqueue(const T& value) {
        QueueNode* newNode = new QueueNode(value);
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    bool dequeue(T& value) {
        if (front == nullptr) return false;
        value = front->data;
        QueueNode* temp = front;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete temp;
        size--;
        return true;
    }
    
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

/**
 * CombatLog - Circular buffer for storing combat messages
 * 
 * This class provides a fixed-size circular buffer for storing combat log messages.
 * Messages are displayed in the game UI to show important events like enemy kills,
 * boss defeats, tower damage, etc. When the buffer is full, new messages overwrite
 * the oldest ones.
 * 
 * Key Features:
 *   - Circular buffer: Fixed-size array that wraps around when full
 *   - FIFO behavior: Oldest messages are overwritten when buffer is full
 *   - Fixed message length: Each message is limited to 63 characters + null terminator
 *   - Efficient access: O(1) insertion and retrieval
 *   - Maximum capacity: Stores up to 10 messages
 * 
 * Usage in Game:
 *   - Displays combat events in the side panel during gameplay
 *   - Shows messages like "Wave 5 started!", "Ultron DEFEATED! +$100", etc.
 *   - Automatically manages message history (oldest messages disappear when full)
 * 
 * Members:
 *   - MAX_LOGS: Maximum number of messages that can be stored (10)
 *   - logs: 2D character array storing message strings
 *   - head: Index of the next position to write (circular buffer head)
 *   - count: Current number of messages stored (0 to MAX_LOGS)
 * 
 * Operations:
 *   - add(): Add a new message to the log (overwrites oldest if full)
 *   - clear(): Remove all messages from the log
 *   - getCount(): Get number of messages currently stored
 *   - getMessage(): Retrieve message at specific index (0 = oldest, count-1 = newest)
 * 
 * Example:
 *   CombatLog log;
 *   log.add("Wave 1 started!");
 *   log.add("Enemy destroyed!");
 *   for (int i = 0; i < log.getCount(); i++) {
 *       const char* msg = log.getMessage(i);
 *       // Display message...
 *   }
 */
class CombatLog {
private:
    static const int MAX_LOGS = 10;
    char logs[MAX_LOGS][64];
    int head;
    int count;
public:
    CombatLog() : head(0), count(0) {
        for (int i = 0; i < MAX_LOGS; i++) logs[i][0] = '\0';
    }
    
    void add(const char* message) {
        int i = 0;
        while (message[i] != '\0' && i < 63) {
            logs[head][i] = message[i];
            i++;
        }
        logs[head][i] = '\0';
        head = (head + 1) % MAX_LOGS;
        if (count < MAX_LOGS) count++;
    }
    
    void clear() {
        head = 0;
        count = 0;
    }
    
    int getCount() const { return count; }
    
    const char* getMessage(int index) const {
        if (index < 0 || index >= count) return "";
        int start = (head - count + MAX_LOGS) % MAX_LOGS;
        int idx = (start + index) % MAX_LOGS;
        return logs[idx]; 
    }
};