#ifndef _DESKTOP_H_
#define _DESKTOP_H_

#include <vector>

class Workspace;

class Desktop {
public:
    Desktop();
    ~Desktop();

    void create();
    bool remove();
    bool jumpto(size_t index);

private:
    Desktop(const Desktop&);
    Desktop& operator=(const Desktop&);

    void __create();
    void __remove();
    void __jumpto();

    static unsigned int __stdcall createThread(void *arg);
    static unsigned int __stdcall removeThread(void *arg);
    static unsigned int __stdcall jumptoThread(void *arg);

private:
    size_t index_; // Index of the current workspace (zero based)
    size_t jumptoIndex_; // Used to save the index jumped to. Ugly :(

    std::vector<Workspace *> workspaces_;
};

#endif // _DESKTOP_H_
