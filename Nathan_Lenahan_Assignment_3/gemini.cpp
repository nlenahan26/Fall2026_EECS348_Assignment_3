#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Map sender category string to integer priority level
int getSenderPriority(const std::string& sender) {
    if (sender == "Boss") return 5;
    if (sender == "Subordinate") return 4;
    if (sender == "Peer") return 3;
    if (sender == "ImportantPerson") return 2;
    if (sender == "OtherPerson") return 1;
    return 0;
}

// Helper to convert MM-DD-YYYY string to YYYYMMDD string for chronological comparison
std::string parseDateToComparable(const std::string& dateStr) {
    if (dateStr.length() < 10) return "";
    std::string month = dateStr.substr(0, 2);
    std::string day = dateStr.substr(3, 2);
    std::string year = dateStr.substr(6, 4);
    return year + month + day;
}

// Email Class Object
class Email {
private:
    std::string sender;
    std::string subject;
    std::string date;

public:
    Email() : sender(""), subject(""), date("") {}
    Email(std::string s, std::string subj, std::string d) 
        : sender(s), subject(subj), date(d) {}

    std::string getSender() const { return sender; }
    std::string getSubject() const { return subject; }
    std::string getDate() const { return date; }

    // Priority Comparison:
    // 1. Higher sender priority comes first.
    // 2. If same priority, newer email (lexicographically greater YYYYMMDD) comes first.
    bool operator<(const Email& other) const {
        int p1 = getSenderPriority(this->sender);
        int p2 = getSenderPriority(other.sender);

        if (p1 != p2) {
            return p1 < p2;
        }

        std::string d1 = parseDateToComparable(this->date);
        std::string d2 = parseDateToComparable(other.date);

        return d1 < d2;
    }

    bool operator>(const Email& other) const {
        return other < *this;
    }
};

// Custom MaxHeap Implementation (List/Array-based)
class MaxHeap {
private:
    std::vector<Email> heap;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent] < heap[index]) {
                std::swap(heap[parent], heap[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapifyDown(int index) {
        int size = heap.size();
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < size && heap[largest] < heap[left]) {
                largest = left;
            }
            if (right < size && heap[largest] < heap[right]) {
                largest = right;
            }

            if (largest != index) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

public:
    MaxHeap() {}

    void insert(const Email& email) {
        heap.push_back(email);
        heapifyUp(heap.size() - 1);
    }

    bool isEmpty() const {
        return heap.empty();
    }

    Email peek() const {
        if (!isEmpty()) {
            return heap[0];
        }
        return Email();
    }

    Email extractMax() {
        if (isEmpty()) {
            return Email();
        }
        Email root = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) {
            heapifyDown(0);
        }
        return root;
    }

    int count() const {
        return heap.size();
    }
};

// PriorityQueue Object wrapper over MaxHeap
class EmailPriorityQueue {
private:
    MaxHeap maxHeap;

public:
    void addEmail(const Email& email) {
        maxHeap.insert(email);
    }

    void displayNext() const {
        if (maxHeap.isEmpty()) {
            std::cout << "No emails in queue." << std::endl;
            return;
        }
        Email top = maxHeap.peek();
        std::cout << "Next email:\n";
        std::cout << "\tSender: " << top.getSender() << "\n";
        std::cout << "\tSubject: " << top.getSubject() << "\n";
        std::cout << "\tDate: " << top.getDate() << "\n\n";
    }

    void readNext() {
        if (!maxHeap.isEmpty()) {
            maxHeap.extractMax();
        }
    }

    void displayCount() const {
        int cnt = maxHeap.count();
        std::cout << "There are " << cnt << " emails to read.\n\n";
    }
};

// Main Command Processor
int main() {
    EmailPriorityQueue emailSystem;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        if (line == "COUNT") {
            emailSystem.displayCount();
        } else if (line == "NEXT") {
            emailSystem.displayNext();
        } else if (line == "READ") {
            emailSystem.readNext();
        } else if (line.rfind("EMAIL ", 0) == 0) {
            std::string payload = line.substr(6); // Remove "EMAIL "
            std::stringstream ss(payload);
            std::string sender, subject, date;

            if (std::getline(ss, sender, ',') &&
                std::getline(ss, subject, ',') &&
                std::getline(ss, date, ',')) {
                
                emailSystem.addEmail(Email(sender, subject, date));
            }
        }
    }

    return 0;
}