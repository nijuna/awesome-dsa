/**
 * Reference Implementation: Doubly Linked List with O(1) Splice
 * Demonstrates node-based memory layout, iterator stability, sentinel nodes,
 * and O(1) list splicing (the key superpower of linked lists over arrays).
 *
 * Language: C++17
 */

#include <iostream>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <utility>

template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
        explicit Node(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
    };

private:
    Node* head_sentinel_;
    Node* tail_sentinel_;
    std::size_t size_;

    void init_sentinels() {
        head_sentinel_ = reinterpret_cast<Node*>(new char[sizeof(Node)]);
        tail_sentinel_ = reinterpret_cast<Node*>(new char[sizeof(Node)]);
        head_sentinel_->prev = nullptr;
        head_sentinel_->next = tail_sentinel_;
        tail_sentinel_->prev = head_sentinel_;
        tail_sentinel_->next = nullptr;
        size_ = 0;
    }

    void destroy_nodes() {
        Node* curr = head_sentinel_->next;
        while (curr != tail_sentinel_) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        delete[] reinterpret_cast<char*>(head_sentinel_);
        delete[] reinterpret_cast<char*>(tail_sentinel_);
        head_sentinel_ = nullptr;
        tail_sentinel_ = nullptr;
        size_ = 0;
    }

public:
    DoublyLinkedList() {
        init_sentinels();
    }

    ~DoublyLinkedList() {
        destroy_nodes();
    }

    // Non-copyable for simplicity, but movable
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    DoublyLinkedList(DoublyLinkedList&& other) noexcept {
        init_sentinels();
        swap(*this, other);
    }

    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            destroy_nodes();
            init_sentinels();
            swap(*this, other);
        }
        return *this;
    }

    friend void swap(DoublyLinkedList& a, DoublyLinkedList& b) noexcept {
        using std::swap;
        swap(a.head_sentinel_, b.head_sentinel_);
        swap(a.tail_sentinel_, b.tail_sentinel_);
        swap(a.size_, b.size_);
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    Node* begin_node() const noexcept { return head_sentinel_->next; }
    Node* end_node() const noexcept { return tail_sentinel_; }

    Node* push_front(const T& val) {
        return insert_before(head_sentinel_->next, val);
    }

    Node* push_back(const T& val) {
        return insert_before(tail_sentinel_, val);
    }

    Node* insert_before(Node* pos, const T& val) {
        Node* new_node = new Node(val);
        new_node->prev = pos->prev;
        new_node->next = pos;
        pos->prev->next = new_node;
        pos->prev = new_node;
        ++size_;
        return new_node;
    }

    void pop_front() {
        if (empty()) throw std::out_of_range("pop_front on empty list");
        erase(head_sentinel_->next);
    }

    void pop_back() {
        if (empty()) throw std::out_of_range("pop_back on empty list");
        erase(tail_sentinel_->prev);
    }

    Node* erase(Node* node) {
        if (node == head_sentinel_ || node == tail_sentinel_) return nullptr;
        Node* next_node = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        --size_;
        return next_node;
    }

    /**
     * O(1) Splice: Transfers 'node' from 'other' into 'this' before 'pos'.
     * Node pointers remain fully valid, zero heap allocations or copies.
     */
    void splice(Node* pos, DoublyLinkedList& other, Node* node) {
        if (node == other.head_sentinel_ || node == other.tail_sentinel_) return;

        // Detach from 'other'
        node->prev->next = node->next;
        node->next->prev = node->prev;
        --other.size_;

        // Attach into 'this' before 'pos'
        node->prev = pos->prev;
        node->next = pos;
        pos->prev->next = node;
        pos->prev = node;
        ++size_;
    }
};

void run_tests() {
    DoublyLinkedList<int> list;
    assert(list.empty());
    assert(list.size() == 0);

    auto* n1 = list.push_back(10);
    auto* n2 = list.push_back(20);
    auto* n3 = list.push_back(30);
    assert(list.size() == 3);

    // Verify links
    assert(list.begin_node() == n1);
    assert(n1->next == n2);
    assert(n2->next == n3);
    assert(n3->next == list.end_node());
    assert(n3->prev == n2);
    assert(n2->prev == n1);

    // Push front
    auto* n0 = list.push_front(5);
    assert(list.size() == 4);
    assert(list.begin_node() == n0);
    assert(n0->next == n1);

    // Erase node n2 (value 20)
    list.erase(n2);
    assert(list.size() == 3);
    assert(n1->next == n3);
    assert(n3->prev == n1);

    // Test O(1) Splice between lists
    DoublyLinkedList<int> other;
    auto* o1 = other.push_back(100);
    auto* o2 = other.push_back(200);
    assert(other.size() == 2);

    // Splice o2 before n3 in list
    list.splice(n3, other, o2);
    assert(other.size() == 1);
    assert(other.begin_node() == o1);
    assert(list.size() == 4);
    assert(n1->next == o2);
    assert(o2->prev == n1);
    assert(o2->next == n3);
    assert(n3->prev == o2);

    // Pop tests
    list.pop_front(); // removes n0 (5)
    assert(list.size() == 3);
    list.pop_back();  // removes n3 (30)
    assert(list.size() == 2);

    std::cout << "[PASS] All DoublyLinkedList C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
