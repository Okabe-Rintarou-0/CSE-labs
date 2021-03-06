#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <assert.h>

#include "rpc.h"
#include "mr_protocol.h"

using namespace std;

struct KeyVal {
    KeyVal(const string &key, const string &val) : key(key), val(val) {}

    string key;
    string val;
};

ostream &operator<<(ostream &os, const KeyVal &keyVal) {
    cout << "(" << keyVal.key << ", " << keyVal.val << ")" << endl;
    return os;
}

inline bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int strHash(const string &str) {
    unsigned int hashVal = 0;
    for (char ch:str) {
        hashVal = hashVal * 131 + (int) ch;
    }
    return hashVal % REDUCER_COUNT;
}

//
// The map function is called once for each file of input. The first
// argument is the name of the input file, and the second is the
// file's complete contents. You should ignore the input file name,
// and look only at the contents argument. The return value is a slice
// of key/value pairs.
//
vector <KeyVal> Map(const string &filename, const string &content) {
    // Copy your code from mr_sequential.cc here.
    unordered_map<std::string, int> wordFreq;
    string word;
    vector <KeyVal> keyVals;
    for (char ch:content) {
        if (isLetter(ch))
            word += ch;
        else if (word.size() > 0) {
            ++wordFreq[word];
            word.clear();
        }
    }
    for (auto entry:wordFreq) {
        keyVals.push_back(KeyVal(entry.first, to_string(entry.second)));
    }
    return keyVals;
}

//
// The reduce function is called once for each key generated by the
// map tasks, with a list of all the values created for that key by
// any map task.
//
string Reduce(const string &key, const vector <string> &values) {
    // Copy your code from mr_sequential.cc here.
    unsigned long long sum = 0;
    for (const string &value:values) {
        sum += atoll(value.c_str());
    }
    return to_string(sum);
}


typedef vector<KeyVal> (*MAPF)(const string &key, const string &value);

typedef string (*REDUCEF)(const string &key, const vector <string> &values);

class Worker {
public:
    Worker(const string &dst, const string &dir, MAPF mf, REDUCEF rf);

    void doWork();

private:
    void doMap(int index, const string &filename);

    void doReduce(int index, int nfiles);

    void doSubmit(mr_tasktype taskType, int index);

    void askTask(mr_protocol::AskTaskResponse &res);

    mutex mtx;
    int id;

    bool working = false;

    rpcc *cl;
    std::string basedir;
    MAPF mapf;
    REDUCEF reducef;
};


Worker::Worker(const string &dst, const string &dir, MAPF mf, REDUCEF rf) {
    this->basedir = dir;
    this->mapf = mf;
    this->reducef = rf;

    sockaddr_in dstsock;
    make_sockaddr(dst.c_str(), &dstsock);
    this->cl = new rpcc(dstsock);
    if (this->cl->bind() < 0) {
        printf("mr worker: call bind error\n");
    }
}

void Worker::askTask(mr_protocol::AskTaskResponse &res) {
    cl->call(mr_protocol::asktask, id, res);
}

void Worker::doMap(int index, const string &filename) {
    // Lab2: Your code goes here.
    working = true;
    string intermediatePrefix;
    //this->basedir
    intermediatePrefix = basedir + "mr-" + to_string(index) + "-";
    string content;
    ifstream file(filename);
    ostringstream tmp;
    tmp << file.rdbuf();
    content = tmp.str();

//    cout << "read from file: " << filename
//         << ", and its content is: " + content.substr(0, 5)
//         << "... (" << content.size() << " in total)" << endl;

    vector <KeyVal> keyVals = Map(filename, content);

//    cout << "Finish map. Now write into intermediates..." << endl;

    vector <string> contents(REDUCER_COUNT);
    for (const KeyVal &keyVal:keyVals) {
        int reducerId = strHash(keyVal.key);
        contents[reducerId] += keyVal.key + ' ' + keyVal.val + '\n';
    }

    for (int i = 0; i < REDUCER_COUNT; ++i) {
        const string &content = contents[i];
        if (!content.empty()) {
            string intermediateFilepath = intermediatePrefix + to_string(i);
            ofstream file(intermediateFilepath, ios::out);
            file << content;
            file.close();
        }
    }

    file.close();
}

void Worker::doReduce(int index, int nfiles) {
    // Lab2: Your code goes here.
//    cout << "worker: start reducing on " << index << endl;
    string filepath;
    unordered_map<string, unsigned long long> wordFreqs;
    for (int i = 0; i < nfiles; ++i) {
        filepath = basedir + "mr-" + to_string(i) + '-' + to_string(index);
        ifstream file(filepath, ios::in);
        if (!file.is_open()) {
//            cout << "reduce worker: file " << filepath << "doesn't exist" << endl;
            continue;
        }
        string key, value;
//        cout << "reduce worker: read from file " << filepath << endl;
        while (file >> key >> value) {
            wordFreqs[key] += atoll(value.c_str());
//            cout << "worker " << index << " put key: " << key << ", " << value << endl;
        }
        file.close();
    }

    string content;
    for (const pair<string, unsigned long long> &keyVal: wordFreqs) {
//        cout << "worker " << index << ": Key: " << keyVal.first << " and value: " << keyVal.second << endl;
        content += keyVal.first + ' ' + to_string(keyVal.second) + '\n';
    }

    ofstream mrOut(basedir + "mr-out", ios::out | ios::app);
    mrOut << content << endl;
    mrOut.close();
    working = true;
}

void Worker::doSubmit(mr_tasktype taskType, int index) {
    bool success;
    mr_protocol::status ret = this->cl->call(mr_protocol::submittask, (int) taskType, index, success);
    if (ret != mr_protocol::OK || !success) {
        fprintf(stderr, "submit task failed\n");
        exit(-1);
    }
//    cout << "worker: submit succeeded" << endl;
    working = false;
}

void Worker::doWork() {
    for (;;) {
        mr_protocol::AskTaskResponse res;
        if (!working)
            askTask(res);
        switch (res.tasktype) {
            case MAP:
                cout << "worker: receive map task " << res.index << endl;
                doMap(res.index, res.filename);
                doSubmit(MAP, res.index);
                break;
            case REDUCE:
                cout << "worker: receive reduce task" << res.index << endl;
                doReduce(res.index, res.nfiles);
                doSubmit(REDUCE, res.index);
                break;
            case NONE:
                cout << "worker: receive no task" << endl;
                sleep(1);
                break;
        }
        // Lab2: Your code goes here.
        // Hints: send asktask RPC call to coordinator
        // if mr_tasktype::MAP, then doMap and doSubmit
        // if mr_tasktype::REDUCE, then doReduce and doSubmit
        // if mr_tasktype::NONE, meaning currently no work is needed, then sleep
        //

    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <coordinator_listen_port> <intermediate_file_dir> \n", argv[0]);
        exit(1);
    }

    MAPF mf = Map;
    REDUCEF rf = Reduce;

    Worker w(argv[1], argv[2], mf, rf);
    w.doWork();

    return 0;
}

