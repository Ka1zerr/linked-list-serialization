#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct ListNode {
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;
};

// ─── Build list from inlet.in ────────────────────────────────────────────────

ListNode* buildList(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open " << filename << "\n";
        return nullptr;
    }

    std::vector<ListNode*> nodes;
    std::vector<int>       randIndices;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // Split on LAST semicolon so data may contain semicolons
        auto sep = line.rfind(';');
        if (sep == std::string::npos) {
            std::cerr << "Bad line: " << line << "\n";
            continue;
        }

        std::string data = line.substr(0, sep);
        int  randIdx     = std::stoi(line.substr(sep + 1));

        auto* node = new ListNode;
        node->data = std::move(data);
        nodes.push_back(node);
        randIndices.push_back(randIdx);
    }

    if (nodes.empty()) return nullptr;

    // Wire prev/next
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i]->prev = (i > 0)                  ? nodes[i - 1] : nullptr;
        nodes[i]->next = (i + 1 < nodes.size())   ? nodes[i + 1] : nullptr;
    }

    // Wire rand
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        int ri = randIndices[i];
        nodes[i]->rand = (ri >= 0 && ri < (int)nodes.size()) ? nodes[ri] : nullptr;
    }

    return nodes[0];
}

// ─── Serialization ───────────────────────────────────────────────────────────
// Format (all integers little-endian):
//   [uint32 count]
//   for each node:
//     [uint32 data_len][data bytes][int32 rand_index]

static void writeU32(std::ostream& out, uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), 4);
}
static void writeI32(std::ostream& out, int32_t v) {
    out.write(reinterpret_cast<const char*>(&v), 4);
}

void serialize(ListNode* head, const std::string& filename) {
    // Collect nodes and build index map
    std::vector<ListNode*>              nodes;
    std::unordered_map<ListNode*, int>  indexMap;

    for (auto* cur = head; cur; cur = cur->next) {
        indexMap[cur] = static_cast<int>(nodes.size());
        nodes.push_back(cur);
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open " << filename << " for writing\n";
        return;
    }

    writeU32(out, static_cast<uint32_t>(nodes.size()));

    for (auto* node : nodes) {
        uint32_t len = static_cast<uint32_t>(node->data.size());
        writeU32(out, len);
        out.write(node->data.data(), len);

        int32_t ri = node->rand ? indexMap.at(node->rand) : -1;
        writeI32(out, ri);
    }
}

// ─── Deserialization ─────────────────────────────────────────────────────────

static uint32_t readU32(std::istream& in) {
    uint32_t v = 0;
    in.read(reinterpret_cast<char*>(&v), 4);
    return v;
}
static int32_t readI32(std::istream& in) {
    int32_t v = 0;
    in.read(reinterpret_cast<char*>(&v), 4);
    return v;
}

ListNode* deserialize(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open " << filename << "\n";
        return nullptr;
    }

    uint32_t count = readU32(in);
    if (count == 0) return nullptr;

    std::vector<ListNode*> nodes(count);
    std::vector<int32_t>   randIndices(count);

    for (uint32_t i = 0; i < count; ++i) {
        nodes[i] = new ListNode;
        uint32_t len = readU32(in);
        nodes[i]->data.resize(len);
        in.read(nodes[i]->data.data(), len);
        randIndices[i] = readI32(in);
    }

    // Wire prev/next/rand
    for (uint32_t i = 0; i < count; ++i) {
        nodes[i]->prev = (i > 0)           ? nodes[i - 1] : nullptr;
        nodes[i]->next = (i + 1 < count)   ? nodes[i + 1] : nullptr;
        int32_t ri     = randIndices[i];
        nodes[i]->rand = (ri >= 0 && ri < (int32_t)count) ? nodes[ri] : nullptr;
    }

    return nodes[0];
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void freeList(ListNode* head) {
    while (head) {
        auto* next = head->next;
        delete head;
        head = next;
    }
}

void printList(ListNode* head) {
    int idx = 0;
    for (auto* cur = head; cur; cur = cur->next, ++idx) {
        std::cout << "[" << idx << "] data=\"" << cur->data << "\"";
        if (cur->rand) {
            // find rand index
            int ri = 0;
            for (auto* t = head; t && t != cur->rand; t = t->next) ++ri;
            std::cout << " rand=" << ri;
        } else {
            std::cout << " rand=null";
        }
        std::cout << "\n";
    }
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::string inFile  = "inlet.in";
    std::string outFile = "outlet.out";

    if (argc >= 2) inFile  = argv[1];
    if (argc >= 3) outFile = argv[2];

    std::cout << "Reading list from: " << inFile << "\n";
    ListNode* head = buildList(inFile);
    if (!head) {
        std::cerr << "Empty or invalid list.\n";
        return 1;
    }

    std::cout << "List built:\n";
    printList(head);

    std::cout << "\nSerializing to: " << outFile << "\n";
    serialize(head, outFile);
    freeList(head);

    std::cout << "Deserializing back...\n";
    ListNode* head2 = deserialize(outFile);

    std::cout << "List after deserialization:\n";
    printList(head2);
    freeList(head2);

    std::cout << "\nDone.\n";
    return 0;
}
