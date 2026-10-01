#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// ---------------------------------------------------------------
// Email: one message in the CEO's inbox
// ---------------------------------------------------------------
class Email {
public:
    string sender;
    string subject;
    string date;      // MM-DD-YYYY
    int senderRank;   // higher = read sooner
    int dateKey;      // YYYYMMDD, larger = newer
    int sequence;     // arrival order, used only to break exact ties

    Email() : sender(""), subject(""), date(""),
              senderRank(0), dateKey(0), sequence(0) {}

    Email(const string& s, const string& subj, const string& d, int seq)
        : sender(s), subject(subj), date(d), sequence(seq) {
        senderRank = rankFor(s);
        dateKey = keyFor(d);
    }

    // Returns true if this email should be read before 'other'
    bool hasHigherPriorityThan(const Email& other) const {
        if (senderRank != other.senderRank) {
            return senderRank > other.senderRank;
        }
        if (dateKey != other.dateKey) {
            return dateKey > other.dateKey;   // newest first
        }
        return sequence < other.sequence;     // earlier arrival first
    }

private:
    static int rankFor(const string& s) {
        if (s == "Boss")            return 5;
        if (s == "Subordinate")     return 4;
        if (s == "Peer")            return 3;
        if (s == "ImportantPerson") return 2;
        return 1;                               // OtherPerson
    }

    static int keyFor(const string& d) {
        // MM-DD-YYYY -> YYYYMMDD
        int month = stoi(d.substr(0, 2));
        int day   = stoi(d.substr(3, 2));
        int year  = stoi(d.substr(6, 4));
        return year * 10000 + month * 100 + day;
    }
};

// ---------------------------------------------------------------
// MaxHeap: list (array) based, grows as needed
// ---------------------------------------------------------------
class MaxHeap {
private:
    Email* items;
    int count;
    int capacity;

    int parentOf(int i) const { return (i - 1) / 2; }
    int leftOf(int i)   const { return 2 * i + 1; }
    int rightOf(int i)  const { return 2 * i + 2; }

    void swapItems(int a, int b) {
        Email temp = items[a];
        items[a] = items[b];
        items[b] = temp;
    }

    void grow() {
        int newCapacity = capacity * 2;
        Email* bigger = new Email[newCapacity];
        for (int i = 0; i < count; i++) {
            bigger[i] = items[i];
        }
        delete[] items;
        items = bigger;
        capacity = newCapacity;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = parentOf(i);
            if (items[i].hasHigherPriorityThan(items[parent])) {
                swapItems(i, parent);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(int i) {
        while (true) {
            int left = leftOf(i);
            int right = rightOf(i);
            int largest = i;

            if (left < count && items[left].hasHigherPriorityThan(items[largest])) {
                largest = left;
            }
            if (right < count && items[right].hasHigherPriorityThan(items[largest])) {
                largest = right;
            }

            if (largest == i) {
                break;
            }
            swapItems(i, largest);
            i = largest;
        }
    }

public:
    MaxHeap() : count(0), capacity(16) {
        items = new Email[capacity];
    }

    ~MaxHeap() {
        delete[] items;
    }

    // Heap owns raw memory; disallow copying
    MaxHeap(const MaxHeap&) = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    void insert(const Email& e) {
        if (count == capacity) {
            grow();
        }
        items[count] = e;
        siftUp(count);
        count++;
    }

    const Email& peekMax() const {
        return items[0];
    }

    void removeMax() {
        if (count == 0) {
            return;
        }
        count--;
        items[0] = items[count];
        siftDown(0);
    }
};

// ---------------------------------------------------------------
// EmailQueue: the CEO's inbox (priority queue backed by MaxHeap)
// ---------------------------------------------------------------
class EmailQueue {
private:
    MaxHeap heap;
    int nextSequence;

public:
    EmailQueue() : nextSequence(0) {}

    void addEmail(const string& sender, const string& subject, const string& date) {
        Email e(sender, subject, date, nextSequence);
        nextSequence++;
        heap.insert(e);
    }

    void showNext() const {
        if (heap.isEmpty()) {
            cout << "There are no emails to read." << endl << endl;
            return;
        }
        const Email& e = heap.peekMax();
        cout << "Next email:" << endl;
        cout << "\tSender: " << e.sender << endl;
        cout << "\tSubject: " << e.subject << endl;
        cout << "\tDate: " << e.date << endl << endl;
    }

    void markRead() {
        heap.removeMax();   // does nothing if empty
    }

    void showCount() const {
        cout << "There are " << heap.size() << " emails to read." << endl << endl;
    }
};

// ---------------------------------------------------------------
// CommandProcessor: reads the test file and runs each command
// ---------------------------------------------------------------
class CommandProcessor {
private:
    EmailQueue inbox;

    static string trim(const string& s) {
        size_t start = 0;
        size_t end = s.size();
        while (start < end && (s[start] == ' ' || s[start] == '\t' ||
                               s[start] == '\r' || s[start] == '\n')) {
            start++;
        }
        while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' ||
                               s[end - 1] == '\r' || s[end - 1] == '\n')) {
            end--;
        }
        return s.substr(start, end - start);
    }

    void handleEmail(const string& args) {
        size_t firstComma = args.find(',');
        size_t lastComma = args.rfind(',');
        if (firstComma == string::npos || firstComma == lastComma) {
            return;   // malformed line
        }
        string sender  = trim(args.substr(0, firstComma));
        string subject = trim(args.substr(firstComma + 1, lastComma - firstComma - 1));
        string date    = trim(args.substr(lastComma + 1));
        inbox.addEmail(sender, subject, date);
    }

public:
    void processFile(const string& filename) {
        ifstream in(filename);
        if (!in) {
            cout << "Could not open file: " << filename << endl;
            return;
        }

        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            if (line.compare(0, 6, "EMAIL ") == 0) {
                handleEmail(line.substr(6));
            } else if (line == "NEXT") {
                inbox.showNext();
            } else if (line == "READ") {
                inbox.markRead();
            } else if (line == "COUNT") {
                inbox.showCount();
            }
        }
    }
};

int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "Enter test file name: ";
        getline(cin, filename);
    }

    CommandProcessor processor;
    processor.processFile(filename);
    return 0;
}
