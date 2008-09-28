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

    void __create();
    void __remove();
    void __jumpto();

private:
    Desktop(const Desktop&);
    Desktop& operator=(const Desktop&);

private:
    size_t index_; // Index of the current workspace (zero based)
    size_t jumptoIndex_; // Used to save the index jumped to. Ugly :(

    std::vector<Workspace *> workspaces_;
};

#endif // _DESKTOP_H_
